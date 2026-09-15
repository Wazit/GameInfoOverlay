# GameInfoOverlay — source code

## Files
- `main.cpp` — mod entry point and HUD drawing logic
- `Android.mk` — build config (modules: GameInfoOverlay / GameInfoOverlay64)
- `Application.mk` — ABI and platform settings
- `build.sh` — build + package script
- `ndkpath.txt` — NDK path hint for AML Compiler
- `test_build.ps1` — Windows build helper
- `aml-psdk/` — game SDK headers (CFont, CTimer, CClock, Events, etc.)
- `mod/` — AML mod headers (amlmod.h, logger, config)

## Build
```bash
./build.sh
```
Requires NDK r29+ and `ndk-build` on PATH. Outputs `mods/` + `configs/` zip.

## What NOT to commit
- `libs/` and `obj/` (build output)
- `*.so` files
