#include <execinfo.h>
#include <ucontext.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sf3500_diag.h"

#define DBG_RING_SIZE (64 * 1024)
static char dbg_ring[DBG_RING_SIZE];
static unsigned dbg_head;        /* total bytes ever written */
static FILE *dbg_stream;
static int dbg_stream_tried;

static char dbg_ctx[1024];

static void dbg_ring_put(const char *p, size_t n)
{
   while (n--)
      dbg_ring[dbg_head++ % DBG_RING_SIZE] = *p++;
}

void Dbg_Log(const char *fmt, ...)
{
   char line[512];
   int n;
   va_list ap;
   va_start(ap, fmt);
   n = vsnprintf(line, sizeof(line) - 1, fmt, ap);
   va_end(ap);
   if (n < 0)
      return;
   if (n > (int)sizeof(line) - 2)
      n = sizeof(line) - 2;
   if (n == 0 || line[n - 1] != '\n')
      line[n++] = '\n';
   dbg_ring_put(line, n);
   if (!dbg_stream_tried)
   {
      const char *path = getenv("Q2_LOG");
      dbg_stream_tried = 1;
      if (path || access("/mnt/sdcard/q2_debug.txt", F_OK) == 0)
         dbg_stream = fopen(path ? path : "/mnt/sdcard/q2.log", "w");
   }
   if (dbg_stream)
   {
      fwrite(line, 1, n, dbg_stream);
      fflush(dbg_stream);
   }
}

static void write_all(int fd, const char *p, size_t n)
{
   while (n)
   {
      ssize_t w = write(fd, p, n);
      if (w <= 0)
         return;
      p += w;
      n -= (size_t)w;
   }
}

void Dbg_DumpCrashLog(const char *why)
{
   static int dumping;
   int fd;
   unsigned start, len;
   void *frames[32];
   if (dumping)
      return;
   dumping = 1;
   fd = open(getenv("Q2_CRASH_LOG") ? getenv("Q2_CRASH_LOG") : "/mnt/sdcard/q2_crash.log",
             O_WRONLY | O_CREAT | O_TRUNC, 0644);
   if (fd >= 0)
   {
      int n, mfd;
      char buf[1024];
      write_all(fd, "reason: ", 8);
      write_all(fd, why, strlen(why));
      write_all(fd, "\n", 1);
      write_all(fd, dbg_ctx, strlen(dbg_ctx));
      write_all(fd, "--- stack\n", 11);
      n = backtrace(frames, 32);
      backtrace_symbols_fd(frames, n, fd);
      write_all(fd, "--- memory map\n", 16);
      mfd = open("/proc/self/maps", O_RDONLY);
      if (mfd >= 0)
      {
         ssize_t r;
         while ((r = read(mfd, buf, sizeof(buf))) > 0)
            write_all(fd, buf, (size_t)r);
         close(mfd);
      }
      write_all(fd, "--- log\n", 8);
      len = dbg_head < DBG_RING_SIZE ? dbg_head : DBG_RING_SIZE;
      start = dbg_head < DBG_RING_SIZE ? 0 : dbg_head % DBG_RING_SIZE;
      write_all(fd, dbg_ring + start, len - start);
      if (start)
         write_all(fd, dbg_ring, start);
      close(fd);
   }
   dumping = 0;
}

static void dbg_signal(int sig, siginfo_t *si, void *ucv)
{
   char why[32];
   ucontext_t *uc = (ucontext_t *)ucv;
   if (uc && si)
   {
      unsigned long *r = (unsigned long *)uc->uc_mcontext.gregs;
      snprintf(dbg_ctx, sizeof(dbg_ctx),
               "pc=%08lx ra=%08lx sp=%08lx gp=%08lx fault_addr=%p code=%d\n"
               "v0=%08lx v1=%08lx a0=%08lx a1=%08lx a2=%08lx a3=%08lx\n"
               "t0=%08lx t1=%08lx t2=%08lx t3=%08lx t8=%08lx t9=%08lx\n"
               "s0=%08lx s1=%08lx s2=%08lx s3=%08lx s4=%08lx s5=%08lx s6=%08lx s7=%08lx\n",
               (unsigned long)uc->uc_mcontext.pc, r[31], r[29], r[28], si->si_addr, si->si_code,
               r[2], r[3], r[4], r[5], r[6], r[7],
               r[8], r[9], r[10], r[11], r[24], r[25],
               r[16], r[17], r[18], r[19], r[20], r[21], r[22], r[23]);
   }
   snprintf(why, sizeof(why), "fatal signal %d", sig);
   Dbg_DumpCrashLog(why);
   signal(sig, SIG_DFL);
   raise(sig);
}

void Dbg_InstallCrashHandlers(void)
{
   struct sigaction sa;
   memset(&sa, 0, sizeof(sa));
   sa.sa_sigaction = dbg_signal;
   sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGSEGV, &sa, NULL);
   sigaction(SIGBUS, &sa, NULL);
   sigaction(SIGILL, &sa, NULL);
   sigaction(SIGFPE, &sa, NULL);
   sigaction(SIGABRT, &sa, NULL);
}
