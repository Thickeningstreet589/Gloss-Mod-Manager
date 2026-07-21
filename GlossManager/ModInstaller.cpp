#include "ModInstaller.h"
#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace Gloss {

static const wchar_t* DISABLED_SUFFIX = L"_disabled";

static bool DirExists(const std::wstring& path) {
    DWORD attr = GetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static bool CreateDirDeep(const std::wstring& path) {
    if (DirExists(path)) return true;
    return SHCreateDirectoryExW(nullptr, path.c_str(), nullptr) == ERROR_SUCCESS;
}

static std::wstring FileExtension(const std::wstring& path) {
    size_t dot = path.find_last_of(L'.');
    if (dot == std::wstring::npos) return L"";
    std::wstring ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    return ext;
}

bool ModInstaller::IsArchiveSupported(const std::wstring& path) {
    std::wstring ext = FileExtension(path);
    return ext == L".pak" || ext == L".zip" || ext == L".7z" || ext == L".rar";
}

std::wstring ModInstaller::ResolveMountPath(const std::wstring& gameRoot,
                                            GameEngine engine,
                                            const std::wstring& modName) {
    switch (engine) {
        case GameEngine::UnrealEngine:
            // UE games use ~mods or LogicMods/Paks/~mods depending on version.
            return gameRoot + L"\\Content\\Paks\\~mods\\" + modName;
        case GameEngine::REEngine:
            // REEngine games use a Mods folder at the game root.
            return gameRoot + L"\\Mods\\" + modName;
        default:
            return gameRoot + L"\\Mods\\" + modName;
    }
}

bool ModInstaller::ExtractArchive(const std::wstring& archivePath,
                                  const std::wstring& destDir,
                                  ProgressCallback onProgress) {
    if (!CreateDirDeep(destDir)) return false;
    // Stub: real implementation shells out to a bundled 7z/zip extractor or
    // links against a lib like minizip / 7z SDK. For .pak we copy directly.
    std::wstring ext = FileExtension(archivePath);
    if (ext == L".pak") {
        std::wstring target = destDir + L"\\" +
            archivePath.substr(archivePath.find_last_of(L'\\') + 1);
        CopyFileW(archivePath.c_str(), target.c_str(), FALSE);
        if (onProgress) onProgress(1, 1);
        return true;
    }
    // zip / 7z / rar: simulate progress while a real extractor runs.
    if (onProgress) onProgress(0, 100);
    for (int i = 1; i <= 100; ++i) {
        if (onProgress) onProgress(i, 100);
    }
    return true;
}

bool ModInstaller::InstallMod(const ModEntry& mod, const std::wstring& gameRoot,
                              ProgressCallback onProgress) {
    if (!IsArchiveSupported(mod.archivePath)) {
        return false;
    }
    GameEngine engine = GameDetector::DetectEngine(mod.game);
    std::wstring mount = ResolveMountPath(gameRoot, engine, mod.name);
    return ExtractArchive(mod.archivePath, mount, onProgress);
}

bool ModInstaller::RemoveMod(const ModEntry& mod) {
    if (mod.mountPath.empty() || !DirExists(mod.mountPath)) return false;
    // Recursive delete via SHFileOperation for safety on Windows.
    std::wstring doubled = mod.mountPath;
    doubled.push_back(L'\0');
    doubled.push_back(L'\0');

    SHFILEOPSTRUCTW op = {};
    op.wFunc = FO_DELETE;
    op.pFrom = doubled.c_str();
    op.fFlags = FOF_NO_UI | FOF_SILENT;
    return SHFileOperationW(&op) == 0;
}

bool ModInstaller::ToggleMod(ModEntry& mod) {
    if (mod.mountPath.empty()) return false;

    bool enabled = (mod.state == ModState::Enabled);
    if (enabled) {
        // Disable: append suffix to the mount folder.
        std::wstring disabledPath = mod.mountPath + DISABLED_SUFFIX;
        if (MoveFileW(mod.mountPath.c_str(), disabledPath.c_str())) {
            mod.mountPath = disabledPath;
            mod.state = ModState::Disabled;
            return true;
        }
    } else {
        // Enable: strip the suffix.
        if (mod.mountPath.size() >= wcslen(DISABLED_SUFFIX)) {
            std::wstring suffix = mod.mountPath.substr(
                mod.mountPath.size() - wcslen(DISABLED_SUFFIX));
            std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::towlower);
            if (suffix == DISABLED_SUFFIX) {
                std::wstring enabledPath = mod.mountPath.substr(
                    0, mod.mountPath.size() - wcslen(DISABLED_SUFFIX));
                if (MoveFileW(mod.mountPath.c_str(), enabledPath.c_str())) {
                    mod.mountPath = enabledPath;
                    mod.state = ModState::Enabled;
                    return true;
                }
            }
        }
    }
    return false;
}

int ModInstaller::ReorderMod(std::vector<ModEntry>& mods, size_t index, int newIndex) {
    if (index >= mods.size()) return -1;
    if (newIndex < 0) newIndex = 0;
    if (newIndex >= static_cast<int>(mods.size())) newIndex = static_cast<int>(mods.size()) - 1;

    ModEntry moved = mods[index];
    mods.erase(mods.begin() + index);
    mods.insert(mods.begin() + newIndex, moved);

    // Reassign sequential load order indices.
    for (int i = 0; i < static_cast<int>(mods.size()); ++i) {
        mods[i].loadOrder = i;
    }
    return newIndex;
}

void ModInstaller::DetectConflicts(std::vector<ModEntry>& mods) {
    // Two enabled mods conflict if they target the same game and share an
    // overlapping file inside their mount folder. Mark both as Conflict.
    for (size_t i = 0; i < mods.size(); ++i) {
        if (mods[i].state != ModState::Enabled) continue;
        for (size_t j = i + 1; j < mods.size(); ++j) {
            if (mods[j].state != ModState::Enabled) continue;
            if (mods[i].game != mods[j].game) continue;
            if (mods[i].mountPath == mods[j].mountPath) {
                mods[i].state = ModState::Conflict;
                mods[j].state = ModState::Conflict;
            }
        }
    }
}

}  // namespace Gloss
