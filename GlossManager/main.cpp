#include "GameDetector.h"
#include "ModInstaller.h"
#include <windows.h>
#include <iostream>
#include <vector>

using namespace Gloss;

static void PrintProgress(int current, int total) {
    int pct = (total > 0) ? (current * 100 / total) : 0;
    std::wcout << L"\r  install: " << pct << L"%  " << std::flush;
}

int wmain() {
    std::wcout << L"== Gloss Mod Manager ==\n";

    // List supported games and pick the first detected install.
    auto games = GameDetector::ListSupportedGames();
    std::wstring pickedGame;
    std::wstring gameRoot;
    for (const auto& g : games) {
        std::wstring path = GameDetector::DetectInstall(g);
        if (!path.empty()) {
            pickedGame = g;
            gameRoot = path;
            std::wcout << L"  detected: " << g << L" at " << path << L"\n";
            break;
        }
    }
    if (gameRoot.empty()) {
        std::wcerr << L"  no supported game detected.\n";
        return 1;
    }

    ModInstaller installer;
    std::vector<ModEntry> mods = {
        { L"m1", L"HDTextures",  L"HDTextures.pak",     L"", ModState::Disabled, 0, pickedGame },
        { L"m2", L"UIOverhaul",  L"UIOverhaul.zip",     L"", ModState::Disabled, 1, pickedGame },
        { L"m3", L"GameplayMod", L"GameplayMod.7z",     L"", ModState::Disabled, 2, pickedGame },
    };

    std::wcout << L"  installing mods...\n";
    for (auto& m : mods) {
        if (installer.InstallMod(m, gameRoot, PrintProgress)) {
            m.state = ModState::Enabled;
            std::wcout << L"\n  ok: " << m.name << L"\n";
        } else {
            std::wcerr << L"\n  fail: " << m.name << L"\n";
        }
    }

    installer.DetectConflicts(mods);

    std::wcout << L"  load order:\n";
    for (const auto& m : mods) {
        std::wcout << L"    [" << m.loadOrder << L"] " << m.name
                   << L"  state=" << static_cast<int>(m.state) << L"\n";
    }

    std::wcout << L"  done.\n";
    return 0;
}
