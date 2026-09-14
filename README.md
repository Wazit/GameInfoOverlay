# GTA:SA Game Info Overlay

Displays FPS, RAM usage, in-game clock, and session play time on a configurable on-screen overlay.

Fully native **AML SDK** — no CLEO, no CALLSCM, no OpcodeCaller, no DEFOPCODE.

## ✨ Features
- FPS counter with color coding (green / yellow / red)
- RAM usage display (MB + percentage)
- In-game clock (HH:MM)
- Session play time
- Configurable position, scale, and toggle via INI
- Works with GTA:SA Android ModLoader (AML) 1.4+

## 📦 AMLMar structure
```
GameInfoOverlay_AMLMar.zip
   ├── configs/
   │   └── net.wazitech.gameinfooverlay.ini
   └── mods/
       ├── libGameInfoOverlay.so       (armeabi-v7a / GTA:SA 2.00)
       └── libGameInfoOverlay64.so     (arm64-v8a / GTA:SA 2.10)
```
> Zip is AMLMar-ready — drop it into your `mods/` folder, no manual extraction needed.

## 🔄 CLEO -> Native SDK mapping
| Original CLEO approach | Native SDK replacement |
|---|---|
| CFont drawing opcodes | `CFont::SetFontStyle / SetScale / SetColor / PrintString` |
| Timers | `CTimer::GetTimeMS()` |
| Clock | `CClock::ms_nGameClockHours / Minutes` |
| HUD drawing | `Events::drawHudEvent` |

No opcode was kept — everything translates to SDK calls because the SDK fully supports this mod.

## ✅ What to do
- Keep using `Events::drawHudEvent` for HUD drawing.
- Use `CFont` / `CTimer` / `CClock` SDK classes for text, time and clock.
- Keep `.so` files inside a `mods/` folder in the release zip.
- Update README + release asset every time you change behavior.

## ❌ What NOT to do
- Do NOT add CLEO / CALLSCM / DEFOPCODE / OpcodeCaller back — SDK covers it.
- Do NOT include `libs/` (build output) in source commits.
- Do NOT guess native addresses — the SDK already exports what we need.
- Do NOT remove `mod/` or `aml-psdk/` — `Android.mk` and `main.cpp` need them.

## 🔧 Build
```
./build.sh        # builds both ABIs (arm64-v8a + armeabi-v7a)
```
Outputs:
- `armeabi-v7a` -> `libGameInfoOverlay.so`
- `arm64-v8a`   -> `libGameInfoOverlay64.so`

## ✅ Verification checklist
1. Build completes with no errors on both ABIs.
2. No `CLEO`, `CALLSCM`, `DEFOPCODE`, `OpcodeCaller`, or `Op<0x...>` remains in `main.cpp`.
3. Test in GTA:SA 2.00 (arm32) and 2.10 (arm64).
4. FPS, RAM, clock, and play time all show and update correctly.
5. INI position/scale changes apply in-game.
6. Pack the two `.so` files into the AMLMar zip layout above and attach it to Releases.

## 📥 Releases
Download the ready AMLMar zip from the **Releases** tab (`GameInfoOverlay_AMLMar.zip`).
