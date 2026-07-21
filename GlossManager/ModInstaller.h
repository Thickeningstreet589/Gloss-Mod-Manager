#pragma once
#include "GameDetector.h"
#include <vector>

namespace Gloss {

class ModInstaller {
public:
    // Extract a mod archive (.pak, .zip, .7z, .rar) into the game's ~mods or
    // LogicMods folder based on the detected engine. Returns true on success.
    bool InstallMod(const ModEntry& mod, const std::wstring& gameRoot,
                    ProgressCallback onProgress = nullptr);

    // Remove a previously installed mod by deleting its mount folder.
    bool RemoveMod(const ModEntry& mod);

    // Toggle a mod between enabled/disabled by renaming its folder with
    // a `_disabled` suffix, avoiding full re-extraction on toggle.
    bool ToggleMod(ModEntry& mod);

    // Reorder a mod within the load order list. Returns the new index.
    int ReorderMod(std::vector<ModEntry>& mods, size_t index, int newIndex);

    // Detect conflicting .pak files across enabled mods and mark them.
    void DetectConflicts(std::vector<ModEntry>& mods);

private:
    std::wstring ResolveMountPath(const std::wstring& gameRoot, GameEngine engine,
                                  const std::wstring& modName);

    bool ExtractArchive(const std::wstring& archivePath, const std::wstring& destDir,
                        ProgressCallback onProgress);

    bool IsArchiveSupported(const std::wstring& path);
};

}  // namespace Gloss
