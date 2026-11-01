/* SF3500 diagnostics: an in-memory log ring that is written to /mnt/sdcard/q2_crash.log when
 * something goes wrong (Sys_Error, an ERR_DROP error, a fatal signal, with a stack trace and
 * the memory map so addresses can be resolved), and streamed to /mnt/sdcard/q2.log only if
 * /mnt/sdcard/q2_debug.txt exists (or the Q2_LOG environment variable names a file). */
#ifndef SF3500_DIAG_H
#define SF3500_DIAG_H

void Dbg_Log(const char *fmt, ...);
void Dbg_DumpCrashLog(const char *why);
void Dbg_InstallCrashHandlers(void);

#endif
