#include "GameDetector.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>

namespace Gloss {

struct GameProfile {
    const wchar_t* key;
    const wchar_t* steamFolder;
    const wchar_t* defaultPath;
    GameEngine engine;
};

static const GameProfile SUPPORTED[] = {
    { L"nier_automata",          L"NieRAutomata",                    L"\\Steam\\steamapps\\common\\NieRAutomata",                    GameEngine::Custom },
    { L"nier_replicant",         L"NieRReplicant",                   L"\\Steam\\steamapps\\common\\NieR Replicant ver.1.2247",       GameEngine::Custom },
    { L"resident_evil_4",        L"Resident Evil 4",                 L"\\Steam\\steamapps\\common\\RESIDENT EVIL 4 BIOHAZARD",       GameEngine::REEngine },
    { L"resident_evil_village",  L"Resident Evil Village",           L"\\Steam\\steamapps\\common\\RESIDENT EVIL VILLAGE",           GameEngine::REEngine },
    { L"monster_hunter_rise",    L"MonsterHunterRise",               L"\\Steam\\steamapps\\common\\MonsterHunterRise",               GameEngine::REEngine },
    { L"monster_hunter_world",   L"MonsterHunterWorld",              L"\\Steam\\steamapps\\common\\Monster Hunter World",            GameEngine::Custom },
    { L"dragons_dogma_2",        L"DragonsDogma2",                   L"\\Steam\\steamapps\\common\\DRAGONS DOGMA 2",                 GameEngine::REEngine },
    { L"hogwarts_legacy",        L"Hogwarts Legacy",                 L"\\Steam\\steamapps\\common\\Hogwarts Legacy",                 GameEngine::UnrealEngine },
    { L"devil_may_cry_5",        L"DevilMayCry5",                    L"\\Steam\\steamapps\\common\\DEVIL MAY CRY 5",                 GameEngine::REEngine },
    { L"sekiro",                 L"Sekiro",                          L"\\Steam\\steamapps\\common\\Sekiro",                         GameEngine::Custom },
    { L"elden_ring",             L"Elden Ring",                      L"\\Steam\\steamapps\\common\\ELDEN RING",                     GameEngine::Custom },
    { L"final_fantasy_xvi",      L"FFXVI",                           L"\\Steam\\steamapps\\common\\FINAL FANTASY XVI",               GameEngine::Custom },
};

static bool DirExists(const std::wstring& path) {
    DWORD attr = GetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static std::wstring SteamInstallRoot() {
    PWSTR raw = nullptr;
    std::wstring result;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, nullptr, &raw))) {
        result = raw;
        CoTaskMemFree(raw);
    }
    return result;
}

std::wstring GameDetector::DetectInstall(const std::wstring& gameKey) {
    std::wstring steamRoot = SteamInstallRoot();
    if (steamRoot.empty()) return L"";

    for (const auto& profile : SUPPORTED) {
        if (gameKey == profile.key) {
            std::wstring candidate = steamRoot + profile.defaultPath;
            if (DirExists(candidate)) return candidate;

            // Probe common alternate library drives (D:, E:, F:)
            const wchar_t* drives[] = { L"D:", L"E:", L"F:" };
            for (const auto& d : drives) {
                std::wstring alt = std::wstring(d) + profile.defaultPath;
                if (DirExists(alt)) return alt;
            }
        }
    }
    return L"";
}

std::vector<std::wstring> GameDetector::ListSupportedGames() {
    std::vector<std::wstring> keys;
    keys.reserve(sizeof(SUPPORTED) / sizeof(SUPPORTED[0]));
    for (const auto& p : SUPPORTED) {
        keys.push_back(p.key);
    }
    return keys;
}

GameEngine GameDetector::DetectEngine(const std::wstring& gameKey) {
    for (const auto& profile : SUPPORTED) {
        if (gameKey == profile.key) {
            return profile.engine;
        }
    }
    return GameEngine::Unknown;
}

}  // namespace Gloss
