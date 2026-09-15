// ============================================================================
// GTA:SA Game Info Overlay
// Displays FPS, RAM usage, and in-game clock on a configurable overlay.
// Config is stored in INI under [Display] and [Position] sections.
// ============================================================================

#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>

#include <aml-psdk/game_sa/Events.h>
#include <aml-psdk/game_sa/base/Timer.h>
#include <aml-psdk/game_sa/engine/Font.h>
#include <aml-psdk/game_sa/engine/RsGlobal.h>
#include <aml-psdk/game_sa/engine/Sprite2d.h>
#include <aml-psdk/game_sa/other/Clock.h>
#include <aml-psdk/gta_base/RGBA.h>

#include <unistd.h>
#include <cstring>
#include <cstdio>

// ============================================================================
// Mod Declaration
// ============================================================================

MYMODCFG(net.rusjj.gtasa.gameinfooverlay, GTA:SA Game Info Overlay, 1.0, Codex)
NEEDGAME(com.rockstargames.gtasa)
BEGIN_DEPLIST()
    ADD_DEPENDENCY_VER(net.rusjj.aml, 1.0)
END_DEPLIST()

// ============================================================================
// Config entries — all persisted to the INI file
// ============================================================================

static ConfigEntry* pEntryShowFPS = NULL;
static ConfigEntry* pEntryShowRAM = NULL;
static ConfigEntry* pEntryShowTime    = NULL;
static ConfigEntry* pEntryShowPlayTime = NULL;
static ConfigEntry* pEntryPosX    = NULL;
static ConfigEntry* pEntryPosY    = NULL;
static ConfigEntry* pEntryScale   = NULL;

// ============================================================================
// FPS State
// ============================================================================

static float    fCurrentFPS   = 0.0f;
static int      nFrameCount   = 0;
static uint32_t nLastFPSTime  = 0;

// ============================================================================
// RAM State
// ============================================================================

static float    fRAMUsedMB    = 0.0f;
static float    fRAMTotalMB   = 0.0f;
static float    fRAMPercent   = 0.0f;
static uint32_t nLastRAMTime  = 0;

// ============================================================================
// Playtime State (session time played)
// ============================================================================

static uint64_t nPlayTimeMs   = 0;
static uint32_t nLastFrameTime = 0;

// ============================================================================
// Read RAM from /proc/self/statm
// ============================================================================

static void ReadRAM()
{
    FILE* f = fopen("/proc/self/statm", "r");
    if (!f) return;

    long totalPages = 0, residentPages = 0;
    if (fscanf(f, "%ld %ld", &totalPages, &residentPages) == 2)
    {
        long pageSize = sysconf(_SC_PAGESIZE);
        fRAMUsedMB  = (float)residentPages * (float)pageSize / 1048576.0f;
        fRAMTotalMB = (float)totalPages    * (float)pageSize / 1048576.0f;
        fRAMPercent = (totalPages > 0)
            ? ((float)residentPages / (float)totalPages * 100.0f)
            : 0.0f;
    }
    fclose(f);
}

// ============================================================================
// Draw the overlay
// ============================================================================

static void DrawOverlay()
{
    bool  showFPS  = pEntryShowFPS ? pEntryShowFPS->GetBool() : true;
    bool  showRAM  = pEntryShowRAM ? pEntryShowRAM->GetBool() : true;
    bool  showTime = pEntryShowTime? pEntryShowTime->GetBool(): true;
    bool  showPlayTime = pEntryShowPlayTime ? pEntryShowPlayTime->GetBool() : true;
    if (!showFPS && !showRAM && !showTime && !showPlayTime) return;

    float screenW = (float)RsGlobal.maximumWidth;
    float screenH = (float)RsGlobal.maximumHeight;
    if (screenW < 1.0f || screenH < 1.0f) return;

    float scaleX = screenW / 640.0f;
    float scaleY = screenH / 448.0f;

    float posX   = pEntryPosX  ? pEntryPosX->GetFloat()  : 170.0f;
    float posY   = pEntryPosY  ? pEntryPosY->GetFloat()  : 10.0f;
    float scale  = pEntryScale ? pEntryScale->GetFloat() : 0.9f;
    if (scale < 0.1f)  scale = 0.1f;
    if (scale > 3.0f)  scale = 3.0f;

    float baseX  = posX * scaleX;
    float baseY  = posY * scaleY;
    float lineH  = 16.0f * scaleY * scale;

    // Count lines to size background
    int lines = 0;
    if (showFPS)      lines++;
    if (showRAM)      lines++;
    if (showTime)     lines++;
    if (showPlayTime) lines++;
    if (lines == 0) return;

    // Transparent background — no panel is drawn. Text stays readable via
    // the font drop-shadow/edge, so it works over any scene.

    // Font setup
    float fontX = 0.35f * scale * scaleX;
    float fontY = 0.65f * scale * scaleY;
    CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);
    CFont::SetScale(fontX, fontY);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetProportional(true);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(screenW);
    CFont::SetEdge(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));

    char buf[128];
    float y = baseY;

    // --- FPS ---
    if (showFPS)
    {
        if      (fCurrentFPS >= 50.0f) CFont::SetColor(CRGBA(100, 255, 100, 255));
        else if (fCurrentFPS >= 30.0f) CFont::SetColor(CRGBA(255, 255, 100, 255));
        else                           CFont::SetColor(CRGBA(255,  80,  80, 255));

        snprintf(buf, sizeof(buf), "FPS: %.1f", fCurrentFPS);
        CFont::PrintString(baseX, y, buf);
        y += lineH;
    }

    // --- RAM ---
    if (showRAM)
    {
        CFont::SetColor(CRGBA(100, 200, 255, 255));
        snprintf(buf, sizeof(buf), "RAM: %.1fMB (%.0f%%)", fRAMUsedMB, fRAMPercent);
        CFont::PrintString(baseX, y, buf);
        y += lineH;
    }

    // --- TIME ---
    if (showTime)
    {
        CFont::SetColor(CRGBA(255, 255, 200, 255));
        snprintf(buf, sizeof(buf), "TIME: %02d:%02d",
                 CClock::ms_nGameClockHours,
                 CClock::ms_nGameClockMinutes);
        CFont::PrintString(baseX, y, buf);
        y += lineH;
    }

    // --- PLAY TIME (session) ---
    if (showPlayTime)
    {
        CFont::SetColor(CRGBA(180, 255, 180, 255));
        uint64_t totalSeconds = nPlayTimeMs / 1000;
        uint64_t hours   = totalSeconds / 3600;
        uint64_t minutes = (totalSeconds % 3600) / 60;
        uint64_t seconds = totalSeconds % 60;
        if (hours > 0)
            snprintf(buf, sizeof(buf), "PLAY: %02llu:%02llu:%02llu",
                     (unsigned long long)hours, (unsigned long long)minutes, (unsigned long long)seconds);
        else
            snprintf(buf, sizeof(buf), "PLAY: %02llu:%02llu",
                     (unsigned long long)minutes, (unsigned long long)seconds);
        CFont::PrintString(baseX, y, buf);
        y += lineH;
    }

    CFont::RenderFontBuffer();
}

// ============================================================================
// Per-frame update
// ============================================================================

static void Update()
{
    uint32_t now = CTimer::GetTimeMS();

    // Playtime: count real gameplay time excluding big pauses/load jumps
    uint32_t delta = now - nLastFrameTime;
    if (delta < 5000) nPlayTimeMs += delta;
    nLastFrameTime = now;

    // FPS: accumulate frames, compute once per second
    nFrameCount++;
    if (now - nLastFPSTime >= 1000)
    {
        fCurrentFPS = (float)nFrameCount * 1000.0f / (float)(now - nLastFPSTime);
        nFrameCount = 0;
        nLastFPSTime = now;
    }

    // RAM: refresh every 1.5 seconds
    if (now - nLastRAMTime >= 1500)
    {
        ReadRAM();
        nLastRAMTime = now;
    }
}

// ============================================================================
// Entry Points
// ============================================================================

ON_MOD_LOAD()
{
    logger->SetTag("GameInfoOverlay");

    // Bind config entries — INI format:
    //
    //   [Display]
    //   ShowFPS = 1
    //   ShowRAM = 1
    //   ShowTime = 1
    //   ShowPlayTime = 1
    //
    //   [Position]
    //   PosX = 170.000000
    //   PosY = 10.000000
    //   Scale = 0.900000
    //
    pEntryShowFPS       = cfg->Bind("ShowFPS",  1,           "Display");
    pEntryShowRAM       = cfg->Bind("ShowRAM",  1,           "Display");
    pEntryShowTime      = cfg->Bind("ShowTime", 1,           "Display");
    pEntryShowPlayTime  = cfg->Bind("ShowPlayTime", 1,       "Display");
    pEntryPosX     = cfg->Bind("PosX",     170.0f,      "Position");
    pEntryPosY     = cfg->Bind("PosY",     10.0f,       "Position");
    pEntryScale    = cfg->Bind("Scale",     0.9f,        "Position");
    cfg->Save();

    // Seed timing
    nLastFrameTime = CTimer::GetTimeMS();
    nLastFPSTime = nLastFrameTime;
    nLastRAMTime = nLastFrameTime;
    ReadRAM(); // first read so it's not zero on screen

    // Hook into the game loop
    Events::gameProcessEvent += []()
    {
        Update();
    };

    Events::drawHudEvent += []()
    {
        DrawOverlay();
    };
}

extern "C" void OnAllModsLoaded()
{
    logger->Info("Game Info Overlay loaded! FPS:%d RAM:%d Time:%d PlayTime:%d Pos:(%.0f,%.0f) Scale:%.2f",
                 pEntryShowFPS->GetBool(), pEntryShowRAM->GetBool(), pEntryShowTime->GetBool(),
                 pEntryShowPlayTime->GetBool(),
                 pEntryPosX->GetFloat(), pEntryPosY->GetFloat(), pEntryScale->GetFloat());
}
