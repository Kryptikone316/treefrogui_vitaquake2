# vitaquake2 for the Data Frog SF3500 (TreeFrogUI)

Changes on top of `tzubertowski/treefrogui_vitaquake2` for the SF3500 (MIPS32r2 74Kc, no GPU):

| Change | Why |
| --- | --- |
| `build_sf3500.sh`: `-O2`, **no** `-ffast-math`, **no** `-mdspr2` | With them the software renderer's alias span loop (`R_PolysetDrawSpans8_Opaque`) read a bad texture pointer and crashed on the device, within seconds to minutes of the opening demo (never in QEMU). Four builds that dropped one of the flags still crashed on a long enough run (7200 frames of the attract demo); without both, 7200 frames and a 40 minute play session were clean. The root cause is not known. |
| Game clock and sound follow real time (`-DSF3500`) | The core advanced the game and the sound by a fixed 1/60 s per run. At 25 to 45 runs per second that gave slow motion and starved audio. Both now use the real time since the last run (clamped to 100 ms). |
| 320x240 as the default resolution (`-DSF3500`) | The core defaulted to 960x544, far beyond what this CPU can draw. |
| Crash report, opt-in log (`libretro/sf3500_diag.c`) | Messages go into a 64 KB in-memory ring. On `Sys_Error`, an `ERR_DROP` error or a fatal signal it is written to `/mnt/sdcard/q2_crash.log` with the faulting address and the memory map; an empty `/mnt/sdcard/q2_debug.txt` streams everything to `q2.log`. A once-a-second `perf:` line shows runs per second and time in the core. |
| Test hooks | `Q2_CMD="map base1"`, `Q2_LOG`, `Q2_CRASH_LOG`, `Q2_CRASHTEST` for an off-device libretro test host. |
| `sf3500/quakeloader` | Gamepad loader the port runs first; lists Quake II, The Reckoning and Ground Zero and saves the choice in `game.txt`. `make preview` builds a PC version with scripted input for screenshots. |

The official add-ons are separate cores because their game code is linked in:
`GAME=xatrix ./build_sf3500.sh` and `GAME=rogue ./build_sf3500.sh`. (`zaero` builds too but is not shipped.)

Under QEMU the core needs a DSP-capable CPU model only if built with `-mdspr2`; the SF3500 build does not use it.

Licence: GPL-2.0, as upstream (`LICENSE`).
