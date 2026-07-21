#pragma once
#include <string>
#include <vector>
#include <functional>

namespace Gloss {

enum class ModState {
    Disabled,
    Enabled,
    Conflict,
    Error
};

enum class GameEngine {
    Unknown,
    UnrealEngine,
    REEngine,
    Custom
};

struct ModEntry {
    std::wstring id;
    std::wstring name;
    std::wstring archivePath;
    std::wstring mountPath;
    ModState state = ModState::Disabled;
    int loadOrder = 0;
    std::wstring game;
};

using ProgressCallback = std::function<void(int current, int total)>;

class GameDetector {
public:
    static std::wstring DetectInstall(const std::wstring& gameKey);
    static std::vector<std::wstring> ListSupportedGames();
    static GameEngine DetectEngine(const std::wstring& gameKey);
};

}  // namespace Gloss
