#include <SDL.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

#include "SteamHelper.h"

namespace fs = std::filesystem;

// Public domain 8x8 font from https://github.com/dhepper/font8x8
static const unsigned char kFont8x8Basic[128][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x7E,0x81,0xA5,0x81,0xBD,0x99,0x81,0x7E},
    {0x7E,0xFF,0xDB,0xFF,0xC3,0xE7,0xFF,0x7E},
    {0x6C,0xFE,0xFE,0xFE,0x7C,0x38,0x10,0x00},
    {0x10,0x38,0x7C,0xFE,0x7C,0x38,0x10,0x00},
    {0x38,0x7C,0x38,0xFE,0xFE,0xD6,0x10,0x38},
    {0x10,0x38,0x7C,0xFE,0xFE,0x7C,0x10,0x38},
    {0x00,0x00,0x18,0x3C,0x3C,0x18,0x00,0x00},
    {0xFF,0xFF,0xE7,0xC3,0xC3,0xE7,0xFF,0xFF},
    {0x00,0x3C,0x66,0x42,0x42,0x66,0x3C,0x00},
    {0xFF,0xC3,0x99,0xBD,0xBD,0x99,0xC3,0xFF},
    {0x0F,0x07,0x0F,0x7D,0xCC,0xCC,0xCC,0x78},
    {0x3C,0x66,0x66,0x66,0x3C,0x18,0x7E,0x18},
    {0x3F,0x33,0x3F,0x30,0x30,0x70,0xF0,0xE0},
    {0x7F,0x63,0x7F,0x63,0x63,0x67,0xE6,0xC0},
    {0x99,0x5A,0x3C,0xE7,0xE7,0x3C,0x5A,0x99},
    {0x80,0xE0,0xF8,0xFE,0xF8,0xE0,0x80,0x00},
    {0x02,0x0E,0x3E,0xFE,0x3E,0x0E,0x02,0x00},
    {0x18,0x3C,0x7E,0x18,0x18,0x7E,0x3C,0x18},
    {0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x00},
    {0x7F,0xDB,0xDB,0x7B,0x1B,0x1B,0x1B,0x00},
    {0x3E,0x61,0x3C,0x66,0x66,0x3C,0x86,0x7C},
    {0x00,0x00,0x00,0x00,0x7E,0x7E,0x7E,0x00},
    {0x18,0x3C,0x7E,0x18,0x7E,0x3C,0x18,0xFF},
    {0x18,0x3C,0x7E,0x18,0x18,0x18,0x18,0x00},
    {0x18,0x18,0x18,0x18,0x7E,0x3C,0x18,0x00},
    {0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00},
    {0x00,0x30,0x60,0xFE,0x60,0x30,0x00,0x00},
    {0x00,0x00,0xC0,0xC0,0xC0,0xFE,0x00,0x00},
    {0x00,0x24,0x66,0xFF,0x66,0x24,0x00,0x00},
    {0x00,0x18,0x3C,0x7E,0xFF,0xFF,0x00,0x00},
    {0x00,0xFF,0xFF,0x7E,0x3C,0x18,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    {0x6C,0x6C,0x48,0x00,0x00,0x00,0x00,0x00},
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00},
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00},
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00},
    {0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00},
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00},
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},
    {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0x00},
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    {0x7C,0xC6,0x0E,0x1C,0x70,0xC0,0xFE,0x00},
    {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00},
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00},
    {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00},
    {0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00},
    {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00},
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00},
    {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00},
    {0x0E,0x1C,0x38,0x70,0x38,0x1C,0x0E,0x00},
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
    {0x70,0x38,0x1C,0x0E,0x1C,0x38,0x70,0x00},
    {0x7C,0xC6,0x0E,0x1C,0x18,0x00,0x18,0x00},
    {0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0x00},
    {0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00},
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00},
    {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00},
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00},
    {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00},
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00},
    {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00},
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00},
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00},
    {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00},
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00},
    {0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00},
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00},
    {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00},
    {0x7C,0xC6,0xC6,0xC6,0xD6,0xCC,0x7A,0x00},
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00},
    {0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00},
    {0x7E,0x7E,0x5A,0x18,0x18,0x18,0x3C,0x00},
    {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    {0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00},
    {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
    {0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00},
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00},
    {0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0x00},
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00},
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00},
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0x00},
    {0x00,0x00,0x7C,0xC6,0xC0,0xC6,0x7C,0x00},
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00},
    {0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00},
    {0x3C,0x66,0x60,0xF8,0x60,0x60,0xF0,0x00},
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8},
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00},
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    {0x06,0x00,0x06,0x06,0x06,0x66,0x66,0x3C},
    {0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00},
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x00,0x00,0xCC,0xFE,0xFE,0xD6,0xC6,0x00},
    {0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x00},
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00},
    {0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0},
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E},
    {0x00,0x00,0xDC,0x76,0x66,0x60,0xF0,0x00},
    {0x00,0x00,0x7E,0xC0,0x7C,0x06,0xFC,0x00},
    {0x30,0x30,0xFC,0x30,0x30,0x36,0x1C,0x00},
    {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00},
    {0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x00},
    {0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x6C,0x00},
    {0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00},
    {0x00,0x00,0xC6,0xC6,0xC6,0x7E,0x06,0xFC},
    {0x00,0x00,0xFE,0x4C,0x18,0x32,0xFE,0x00},
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00},
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00},
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0x00}
};

struct Entry {
    std::string name;
    fs::path path;
    bool isDir;
    bool isParent;
};

enum class PaneSource {
    Local,
    Ftp
};

struct Pane {
    fs::path cwd;
    fs::path lastLocalCwd;
    std::string ftpPath;
    PaneSource source = PaneSource::Local;
    std::vector<Entry> entries;
    int selected = 0;
    int scroll = 0;
};

enum class Mode {
    Browse,
    ActionMenu,
    ConfirmDelete,
    Rename,
    CreateFolder,
    AddToSteam,
    AppMenu,
    Settings,
    EditSetting,
    ConfirmQuit
};

struct ActionContext {
    Entry entry;
    int paneIndex = 0;
};

struct StatusMessage {
    std::string text;
    std::chrono::steady_clock::time_point started;
};

static void setStatus(StatusMessage& status, const std::string& text);
static bool copyEntry(const Entry& entry, const fs::path& targetDir, std::string& error);
static bool moveEntry(const Entry& entry, const fs::path& targetDir, std::string& error);
static bool deleteEntry(const Entry& entry, std::string& error);
static bool renameEntry(const Entry& entry, const std::string& newName, std::string& error);
static bool isWindowsExe(const Entry& entry, const Pane& pane);
static bool isZipArchive(const Entry& entry, const Pane& pane);
static bool isRarArchive(const Entry& entry, const Pane& pane);
static std::string defaultSteamAppName(const Entry& entry);
static std::vector<std::string> buildActionOptions(const Entry& entry, const Pane& pane);
static void resetPanePosition(Pane& pane);

struct Settings {
    std::string ftpHost;
    int ftpPort = 21;
    std::string ftpUser;
    std::string ftpPass;
    std::string steamLaunchOptions;
    std::string steamCompatibilityToolVersion;
    float uiScale = 1.0f;
    bool showHidden = false;
    fs::path panePath[2];
};

enum class SettingField {
    FtpHost,
    FtpPort,
    FtpUser,
    FtpPass,
    SteamLaunchOptions,
    SteamCompatibilityTool
};

enum class OskAction {
    None,
    Backspace,
    Clear,
    Ok,
    Cancel,
    ToggleShift,
    ToggleSymbols
};

struct OskKey {
    std::string label;
    std::string value;
    OskAction action = OskAction::None;
};

struct OskState {
    int row = 0;
    int col = 0;
    bool uppercase = false;
    bool symbols = false;
};

struct TransferState {
    bool active = false;
    std::string title;
    std::string item;
    double progress = 0.0;
    int countCurrent = 0;
    int countTotal = 0;
    std::chrono::steady_clock::time_point lastDraw;
};

struct TransferContext {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    const Settings* settings = nullptr;
    bool* running = nullptr;
    TransferState* transfer = nullptr;
};

static std::string sanitizeLabel(const std::string& label) {
    std::string cleaned;
    cleaned.reserve(label.size());
    for (unsigned char c : label) {
        if (c >= 32 && c <= 126) {
            cleaned.push_back(static_cast<char>(c));
        } else {
            cleaned.push_back('?');
        }
    }
    return cleaned;
}

static void drawChar(SDL_Renderer* renderer, int x, int y, int scale, SDL_Color color, char ch) {
    unsigned char c = static_cast<unsigned char>(ch);
    if (c > 127) {
        c = '?';
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int row = 0; row < 8; ++row) {
        unsigned char bits = kFont8x8Basic[c][row];
        for (int col = 0; col < 8; ++col) {
            if (bits & (1 << (7 - col))) {
                SDL_Rect pixel {x + col * scale, y + row * scale, scale, scale};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

static void drawText(SDL_Renderer* renderer, int x, int y, int scale, SDL_Color color, const std::string& text) {
    int cursorX = x;
    const int advance = 8 * scale + scale;
    std::string safe = sanitizeLabel(text);
    for (char ch : safe) {
        drawChar(renderer, cursorX, y, scale, color, ch);
        cursorX += advance;
    }
}

static std::string ellipsize(const std::string& text, int maxChars) {
    if (maxChars <= 0) {
        return "";
    }
    if (static_cast<int>(text.size()) <= maxChars) {
        return text;
    }
    if (maxChars <= 3) {
        return text.substr(0, static_cast<size_t>(maxChars));
    }
    return text.substr(0, static_cast<size_t>(maxChars - 3)) + "...";
}

static std::string toLower(const std::string& text) {
    std::string out = text;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

static std::string quoteArg(const std::string& text) {
#ifdef _WIN32
    std::string escaped = "\"";
    for (char ch : text) {
        if (ch == '"') {
            escaped += "\\\"";
        } else {
            escaped.push_back(ch);
        }
    }
    escaped += "\"";
    return escaped;
#else
    std::string escaped = "\"";
    for (char ch : text) {
        if (ch == '\\' || ch == '"' || ch == '$' || ch == '`') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    escaped += "\"";
    return escaped;
#endif
}

static FILE* openPipe(const std::string& command, const char* mode) {
#ifdef _WIN32
    return _popen(command.c_str(), mode);
#else
    return popen(command.c_str(), mode);
#endif
}

static int closePipe(FILE* pipe) {
#ifdef _WIN32
    return _pclose(pipe);
#else
    return pclose(pipe);
#endif
}

static std::string trimWhitespace(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(start, end - start);
}

static std::string urlEncodeFilePath(const std::string& text) {
    std::ostringstream out;
    out << std::uppercase << std::hex;
    for (unsigned char c : text) {
        if (std::isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~' || c == ':') {
            out << static_cast<char>(c);
        } else {
            out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }
    return out.str();
}

static std::string fileUrlForPath(const fs::path& path) {
    std::string abs = fs::absolute(path).generic_string();
#ifdef _WIN32
    if (abs.size() >= 2 && abs[1] == ':') {
        return "file:///" + urlEncodeFilePath(abs);
    }
#endif
    return "file://" + urlEncodeFilePath(abs);
}

static bool openLocalFile(const fs::path& path, std::string& error) {
    std::string url = fileUrlForPath(path);
    if (SDL_OpenURL(url.c_str()) != 0) {
        error = SDL_GetError();
        return false;
    }
    return true;
}

static bool addExeToSteam(const fs::path& exePath,
                          const std::string& appName,
                          const std::string& launchOptions,
                          const std::string& compatibilityToolVersion,
                          std::string& error) {
    fs::path absExe = fs::absolute(exePath);
    fs::path startDir = absExe.parent_path();
    std::uint32_t appId = 0;
    if (!addShortcutToSteam(absExe, startDir, appName, launchOptions, appId, error)) {
        return false;
    }
    if (!compatibilityToolVersion.empty()) {
        std::string compatError;
        if (!setSteamCompatToolMapping(appId, compatibilityToolVersion, compatError)) {
            error = compatError;
        }
    }
    return true;
}

static int textWidth(int scale, const std::string& text) {
    const int advance = 8 * scale + scale;
    return static_cast<int>(text.size()) * advance;
}

static void insertFiltered(std::string& buffer, size_t& cursor, const std::string& input, bool digitsOnly) {
    if (cursor > buffer.size()) {
        cursor = buffer.size();
    }
    for (char ch : input) {
        if (digitsOnly && !std::isdigit(static_cast<unsigned char>(ch))) {
            continue;
        }
        buffer.insert(buffer.begin() + static_cast<std::string::difference_type>(cursor), ch);
        ++cursor;
    }
}

static void backspaceAtCursor(std::string& buffer, size_t& cursor) {
    if (cursor == 0 || buffer.empty()) {
        return;
    }
    if (cursor > buffer.size()) {
        cursor = buffer.size();
    }
    buffer.erase(buffer.begin() + static_cast<std::string::difference_type>(cursor - 1));
    --cursor;
}

static void drawInputText(SDL_Renderer* renderer, int x, int y, int scale, SDL_Color color,
                          const std::string& text, size_t cursor, int maxChars) {
    if (maxChars <= 0) {
        return;
    }
    std::string safe = sanitizeLabel(text);
    if (cursor > safe.size()) {
        cursor = safe.size();
    }
    int start = 0;
    if (static_cast<int>(safe.size()) > maxChars) {
        int cursorPos = static_cast<int>(cursor);
        if (cursorPos > start + maxChars) {
            start = cursorPos - maxChars;
        }
        int maxStart = static_cast<int>(safe.size()) - maxChars;
        start = std::clamp(start, 0, maxStart);
    }
    int count = std::min(maxChars, static_cast<int>(safe.size()) - start);
    std::string visible = safe.substr(static_cast<size_t>(start), static_cast<size_t>(count));
    drawText(renderer, x, y, scale, color, visible);

    const int advance = 8 * scale + scale;
    int cursorOffset = static_cast<int>(cursor) - start;
    cursorOffset = std::clamp(cursorOffset, 0, count);
    int cursorX = x + cursorOffset * advance;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect caret {cursorX, y, std::max(1, scale), 8 * scale};
    SDL_RenderFillRect(renderer, &caret);
}

static OskKey makeActionKey(const std::string& label, OskAction action) {
    return {label, "", action};
}

static std::vector<OskKey> makeCharRow(const std::string& chars) {
    std::vector<OskKey> row;
    row.reserve(chars.size());
    for (char ch : chars) {
        std::string value(1, ch);
        row.push_back({value, value, OskAction::None});
    }
    return row;
}

static std::vector<std::vector<OskKey>> buildOskLayout(bool uppercase, bool symbols, bool numeric) {
    std::vector<std::vector<OskKey>> rows;
    if (numeric) {
        rows.push_back(makeCharRow("123"));
        rows.push_back(makeCharRow("456"));
        rows.push_back(makeCharRow("789"));
        rows.push_back(makeCharRow("0"));
        rows.push_back({makeActionKey("BACK", OskAction::Backspace),
                        makeActionKey("CLEAR", OskAction::Clear),
                        makeActionKey("OK", OskAction::Ok)});
        rows.push_back({makeActionKey("CANCEL", OskAction::Cancel)});
        return rows;
    }

    if (symbols) {
        rows.push_back(makeCharRow("!@#$%^&*()"));
        rows.push_back(makeCharRow("-_=+[]{}\\|"));
        rows.push_back(makeCharRow(";:'\",./?"));
        rows.push_back(makeCharRow("<>`~"));
    } else {
        rows.push_back(makeCharRow("1234567890"));
        std::string row2 = "QWERTYUIOP";
        std::string row3 = "ASDFGHJKL";
        std::string row4 = "ZXCVBNM";
        if (!uppercase) {
            row2 = toLower(row2);
            row3 = toLower(row3);
            row4 = toLower(row4);
        }
        rows.push_back(makeCharRow(row2));
        rows.push_back(makeCharRow(row3));
        rows.push_back(makeCharRow(row4));
    }

    std::string shiftLabel = uppercase ? "LOWER" : "SHIFT";
    std::string symLabel = symbols ? "ABC" : "SYM";
    rows.push_back({makeActionKey(shiftLabel, OskAction::ToggleShift),
                    makeActionKey(symLabel, OskAction::ToggleSymbols),
                    {"SPACE", " ", OskAction::None},
                    makeActionKey("BACK", OskAction::Backspace),
                    makeActionKey("OK", OskAction::Ok)});
    rows.push_back({makeActionKey("CLEAR", OskAction::Clear),
                    makeActionKey("CANCEL", OskAction::Cancel)});
    return rows;
}

static void clampOskSelection(OskState& osk, const std::vector<std::vector<OskKey>>& layout) {
    if (layout.empty()) {
        osk.row = 0;
        osk.col = 0;
        return;
    }
    osk.row = std::clamp(osk.row, 0, static_cast<int>(layout.size()) - 1);
    int cols = static_cast<int>(layout[osk.row].size());
    if (cols <= 0) {
        osk.col = 0;
        return;
    }
    osk.col = std::clamp(osk.col, 0, cols - 1);
}

static void drawOsk(SDL_Renderer* renderer,
                    const SDL_Rect& area,
                    int fontScale,
                    float uiScale,
                    SDL_Color textColor,
                    const std::vector<std::vector<OskKey>>& layout,
                    const OskState& osk) {
    const int keyHeight = std::max(1, static_cast<int>(std::round(28.0f * uiScale)));
    const int keyGap = std::max(1, static_cast<int>(std::round(6.0f * uiScale)));
    const SDL_Color keyBg {25, 30, 35, 255};
    const SDL_Color keyBorder {60, 70, 80, 255};
    const SDL_Color keyActive {40, 120, 160, 255};

    int y = area.y;
    for (size_t rowIndex = 0; rowIndex < layout.size(); ++rowIndex) {
        const auto& row = layout[rowIndex];
        if (row.empty()) {
            y += keyHeight + keyGap;
            continue;
        }
        int cols = static_cast<int>(row.size());
        int keyWidth = (area.w - keyGap * (cols - 1)) / cols;
        int rowWidth = cols * keyWidth + keyGap * (cols - 1);
        int x = area.x + (area.w - rowWidth) / 2;
        for (int colIndex = 0; colIndex < cols; ++colIndex) {
            SDL_Rect keyRect {x, y, keyWidth, keyHeight};
            bool selected = (static_cast<int>(rowIndex) == osk.row && colIndex == osk.col);
            SDL_Color fill = selected ? keyActive : keyBg;
            SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
            SDL_RenderFillRect(renderer, &keyRect);
            SDL_SetRenderDrawColor(renderer, keyBorder.r, keyBorder.g, keyBorder.b, keyBorder.a);
            SDL_RenderDrawRect(renderer, &keyRect);

            const std::string& label = row[static_cast<size_t>(colIndex)].label;
            int labelWidth = textWidth(fontScale, label);
            int labelX = keyRect.x + (keyRect.w - labelWidth) / 2;
            int labelY = keyRect.y + (keyRect.h - 8 * fontScale) / 2;
            drawText(renderer, labelX, labelY, fontScale, textColor, label);
            x += keyWidth + keyGap;
        }
        y += keyHeight + keyGap;
    }
}

static void drawTransferScreen(SDL_Renderer* renderer, SDL_Window* window, const Settings& settings, const TransferState& transfer) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);

    float uiScale = settings.uiScale;
    const int fontScale = std::max(1, static_cast<int>(std::round(2.0f * uiScale)));
    const int smallScale = std::max(1, fontScale - 1);
    const int modalWidth = static_cast<int>(std::round(520.0f * uiScale));
    const int modalHeight = static_cast<int>(std::round(240.0f * uiScale));
    const int padding = static_cast<int>(std::round(20.0f * uiScale));

    SDL_SetRenderDrawColor(renderer, 15, 20, 24, 255);
    SDL_RenderClear(renderer);

    SDL_Rect modal {width / 2 - modalWidth / 2, height / 2 - modalHeight / 2, modalWidth, modalHeight};
    SDL_SetRenderDrawColor(renderer, 30, 35, 40, 255);
    SDL_RenderFillRect(renderer, &modal);
    SDL_SetRenderDrawColor(renderer, 80, 220, 140, 255);
    SDL_RenderDrawRect(renderer, &modal);

    SDL_Color textColor {230, 235, 240, 255};
    std::string title = transfer.title.empty() ? "Transferring" : transfer.title;
    drawText(renderer, modal.x + padding, modal.y + padding, fontScale, textColor, title);

    std::string itemLabel = transfer.item.empty() ? "(unknown)" : transfer.item;
    drawText(renderer, modal.x + padding, modal.y + padding + static_cast<int>(std::round(40.0f * uiScale)),
             smallScale, textColor, ellipsize(itemLabel, 48));

    int barWidth = modal.w - padding * 2;
    int barHeight = static_cast<int>(std::round(20.0f * uiScale));
    int barX = modal.x + padding;
    int barY = modal.y + modal.h / 2;
    SDL_Rect barRect {barX, barY, barWidth, barHeight};
    SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
    SDL_RenderFillRect(renderer, &barRect);
    SDL_SetRenderDrawColor(renderer, 80, 220, 140, 255);
    SDL_RenderDrawRect(renderer, &barRect);

    double progress = std::clamp(transfer.progress, 0.0, 1.0);
    int fillWidth = static_cast<int>(std::round(barWidth * progress));
    if (fillWidth > 0) {
        SDL_Rect fillRect {barX + 2, barY + 2, std::max(0, fillWidth - 4), barHeight - 4};
        SDL_SetRenderDrawColor(renderer, 40, 120, 160, 255);
        SDL_RenderFillRect(renderer, &fillRect);
    }

    int percent = static_cast<int>(std::round(progress * 100.0));
    std::string percentLabel = std::to_string(percent) + "%";
    drawText(renderer, modal.x + padding, modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
             smallScale, textColor, percentLabel);
    if (transfer.countTotal > 0) {
        std::string countLabel = std::to_string(transfer.countCurrent) + " of " + std::to_string(transfer.countTotal);
        int labelWidth = textWidth(smallScale, countLabel);
        int labelX = std::max(modal.x + padding, modal.x + modal.w - padding - labelWidth);
        drawText(renderer, labelX, modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                 smallScale, textColor, countLabel);
    }

    SDL_RenderPresent(renderer);
}

static void pumpTransferUI(TransferContext& ctx) {
    if (!ctx.window || !ctx.renderer || !ctx.settings || !ctx.transfer) {
        return;
    }
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT && ctx.running) {
            *ctx.running = false;
        }
    }
    if (ctx.running && !*ctx.running) {
        return;
    }
    drawTransferScreen(ctx.renderer, ctx.window, *ctx.settings, *ctx.transfer);
}

static void startTransferItem(TransferContext* ctx, const std::string& title, const std::string& item, bool resetCount = true) {
    if (!ctx || !ctx->transfer) {
        return;
    }
    ctx->transfer->active = true;
    ctx->transfer->title = title;
    ctx->transfer->item = item;
    ctx->transfer->progress = 0.0;
    if (resetCount) {
        ctx->transfer->countCurrent = 0;
        ctx->transfer->countTotal = 0;
    }
    ctx->transfer->lastDraw = std::chrono::steady_clock::now() - std::chrono::milliseconds(100);
    pumpTransferUI(*ctx);
}

static void updateTransferProgress(TransferContext* ctx, double progress) {
    if (!ctx || !ctx->transfer) {
        return;
    }
    ctx->transfer->progress = progress;
    auto now = std::chrono::steady_clock::now();
    if (now - ctx->transfer->lastDraw > std::chrono::milliseconds(33)) {
        ctx->transfer->lastDraw = now;
        pumpTransferUI(*ctx);
    }
}

static void updateTransferCount(TransferContext* ctx, int current, int total) {
    if (!ctx || !ctx->transfer) {
        return;
    }
    ctx->transfer->countCurrent = std::max(0, current);
    ctx->transfer->countTotal = std::max(0, total);
    auto now = std::chrono::steady_clock::now();
    if (now - ctx->transfer->lastDraw > std::chrono::milliseconds(33)) {
        ctx->transfer->lastDraw = now;
        pumpTransferUI(*ctx);
    }
}

static void finishTransfer(TransferContext* ctx) {
    if (!ctx || !ctx->transfer) {
        return;
    }
    ctx->transfer->active = false;
}

static std::string formatScale(float scale) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << scale;
    return out.str();
}

static std::string maskPassword(const std::string& pass) {
    return std::string(pass.size(), '*');
}

static fs::path getHomePath() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
    if (home && home[0] != '\0') {
        return fs::path(home);
    }
    const char* drive = std::getenv("HOMEDRIVE");
    const char* path = std::getenv("HOMEPATH");
    if (drive && path && drive[0] != '\0' && path[0] != '\0') {
        return fs::path(std::string(drive) + path);
    }
#else
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return fs::path(home);
    }
#endif
    return fs::current_path();
}

static std::string escapeXml(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char ch : text) {
        switch (ch) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        case '\'':
            out += "&apos;";
            break;
        default:
            out.push_back(ch);
            break;
        }
    }
    return out;
}

static std::string unescapeXml(const std::string& text) {
    std::string out = text;
    auto replaceAll = [&](const std::string& from, const std::string& to) {
        size_t start = 0;
        while ((start = out.find(from, start)) != std::string::npos) {
            out.replace(start, from.size(), to);
            start += to.size();
        }
    };
    replaceAll("&lt;", "<");
    replaceAll("&gt;", ">");
    replaceAll("&quot;", "\"");
    replaceAll("&apos;", "'");
    replaceAll("&amp;", "&");
    return out;
}

static std::string readTag(const std::string& xml, const std::string& tag) {
    std::string open = "<" + tag + ">";
    std::string close = "</" + tag + ">";
    size_t start = xml.find(open);
    if (start == std::string::npos) {
        return "";
    }
    start += open.size();
    size_t end = xml.find(close, start);
    if (end == std::string::npos) {
        return "";
    }
    return xml.substr(start, end - start);
}

static float clampUiScale(float scale) {
    return std::clamp(scale, 0.5f, 2.5f);
}

static bool loadConfig(Settings& settings, const std::string& path, const fs::path& homePath) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string xml = buffer.str();
    if (xml.empty()) {
        return false;
    }

    std::string uiScaleValue = readTag(xml, "uiScale");
    if (!uiScaleValue.empty()) {
        try {
            settings.uiScale = clampUiScale(std::stof(uiScaleValue));
        } catch (const std::exception&) {
        }
    }

    settings.ftpHost = unescapeXml(readTag(xml, "ftpHost"));
    std::string ftpPortValue = readTag(xml, "ftpPort");
    if (!ftpPortValue.empty()) {
        try {
            settings.ftpPort = std::stoi(ftpPortValue);
        } catch (const std::exception&) {
        }
    }
    if (settings.ftpPort <= 0) {
        settings.ftpPort = 21;
    }
    settings.ftpUser = unescapeXml(readTag(xml, "ftpUser"));
    settings.ftpPass = unescapeXml(readTag(xml, "ftpPass"));
    settings.steamLaunchOptions = unescapeXml(readTag(xml, "steamLaunchOptions"));
    settings.steamCompatibilityToolVersion = unescapeXml(readTag(xml, "steamCompatibilityToolVersion"));
    std::string showHiddenValue = readTag(xml, "showHidden");
    if (!showHiddenValue.empty()) {
        std::string lowered;
        lowered.reserve(showHiddenValue.size());
        for (unsigned char c : showHiddenValue) {
            lowered.push_back(static_cast<char>(std::tolower(c)));
        }
        settings.showHidden = (lowered == "true" || lowered == "1" || lowered == "yes");
    }

    std::string pane0 = unescapeXml(readTag(xml, "pane0"));
    std::string pane1 = unescapeXml(readTag(xml, "pane1"));

    if (!pane0.empty()) {
        settings.panePath[0] = fs::path(pane0);
    }
    if (!pane1.empty()) {
        settings.panePath[1] = fs::path(pane1);
    }

    for (int i = 0; i < 2; ++i) {
        if (settings.panePath[i].empty() || !fs::is_directory(settings.panePath[i])) {
            settings.panePath[i] = homePath;
        }
    }

    return true;
}

static bool saveConfig(const Settings& settings, const Pane panes[2], const std::string& path) {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file << "<config>\n";
    file << "  <uiScale>" << settings.uiScale << "</uiScale>\n";
    file << "  <ftpHost>" << escapeXml(settings.ftpHost) << "</ftpHost>\n";
    file << "  <ftpPort>" << settings.ftpPort << "</ftpPort>\n";
    file << "  <ftpUser>" << escapeXml(settings.ftpUser) << "</ftpUser>\n";
    file << "  <ftpPass>" << escapeXml(settings.ftpPass) << "</ftpPass>\n";
    file << "  <steamLaunchOptions>" << escapeXml(settings.steamLaunchOptions) << "</steamLaunchOptions>\n";
    file << "  <steamCompatibilityToolVersion>" << escapeXml(settings.steamCompatibilityToolVersion)
         << "</steamCompatibilityToolVersion>\n";
    file << "  <showHidden>" << (settings.showHidden ? "true" : "false") << "</showHidden>\n";
    file << "  <pane0>" << escapeXml(panes[0].lastLocalCwd.string()) << "</pane0>\n";
    file << "  <pane1>" << escapeXml(panes[1].lastLocalCwd.string()) << "</pane1>\n";
    file << "</config>\n";
    return true;
}

static void sortEntries(std::vector<Entry>& entries) {
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.isDir != b.isDir) {
            return a.isDir > b.isDir;
        }
        return toLower(a.name) < toLower(b.name);
    });
}

static std::string normalizeFtpPath(const std::string& path) {
    if (path.empty()) {
        return "/";
    }
    std::string normalized = path;
    if (normalized[0] != '/') {
        normalized = "/" + normalized;
    }
    while (normalized.size() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    return normalized;
}

static std::string ftpJoinPath(const std::string& base, const std::string& name) {
    std::string cleanBase = normalizeFtpPath(base);
    if (cleanBase == "/") {
        return cleanBase + name;
    }
    return cleanBase + "/" + name;
}

static std::string ftpParentPath(const std::string& path) {
    std::string clean = normalizeFtpPath(path);
    if (clean == "/") {
        return "/";
    }
    size_t pos = clean.find_last_of('/');
    if (pos == std::string::npos || pos == 0) {
        return "/";
    }
    return clean.substr(0, pos);
}

#ifdef USE_CURL
struct CurlBuffer {
    std::string data;
};

static size_t curlWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    CurlBuffer* buffer = static_cast<CurlBuffer*>(userdata);
    buffer->data.append(static_cast<char*>(ptr), total);
    return total;
}

static bool parseFtpListLine(const std::string& line, Entry& entry);

static std::string urlEncodePath(const std::string& path) {
    std::ostringstream out;
    for (unsigned char c : path) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            out << static_cast<char>(c);
        } else {
            out << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(c) << std::nouppercase << std::dec;
        }
    }
    return out.str();
}

static std::string buildFtpUrl(const Settings& settings, const std::string& path, bool isDir) {
    std::string url = "ftp://" + settings.ftpHost;
    if (settings.ftpPort > 0 && settings.ftpPort != 21) {
        url += ":" + std::to_string(settings.ftpPort);
    }
    std::string remotePath = normalizeFtpPath(path);
    if (isDir && remotePath.back() != '/') {
        remotePath.push_back('/');
    }
    url += urlEncodePath(remotePath);
    return url;
}

static bool configureFtpHandle(CURL* curl, const Settings& settings, std::string& error) {
    if (!curl) {
        error = "Failed to initialize CURL";
        return false;
    }
    if (settings.ftpHost.empty()) {
        error = "FTP host not set";
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_USERNAME, settings.ftpUser.empty() ? "anonymous" : settings.ftpUser.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, settings.ftpPass.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    return true;
}

static bool fetchFtpList(const Settings& settings, const std::string& path, std::string& output, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    std::string url = buildFtpUrl(settings, path, true);
    CurlBuffer buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "MLSD");
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        curl = curl_easy_init();
        if (!configureFtpHandle(curl, settings, error)) {
            if (curl) {
                curl_easy_cleanup(curl);
            }
            return false;
        }
        buffer.data.clear();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            error = curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return false;
        }
    }
    curl_easy_cleanup(curl);
    output = buffer.data;
    return true;
}

static size_t curlWriteFileCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    FILE* file = static_cast<FILE*>(userdata);
    return std::fwrite(ptr, size, nmemb, file);
}

static size_t curlReadFileCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    FILE* file = static_cast<FILE*>(userdata);
    return std::fread(ptr, size, nmemb, file);
}

struct CurlProgressData {
    TransferContext* ctx = nullptr;
};

static int curlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    CurlProgressData* data = static_cast<CurlProgressData*>(clientp);
    if (!data || !data->ctx) {
        return 0;
    }
    double total = 0.0;
    double now = 0.0;
    if (dltotal > 0) {
        total = static_cast<double>(dltotal);
        now = static_cast<double>(dlnow);
    } else if (ultotal > 0) {
        total = static_cast<double>(ultotal);
        now = static_cast<double>(ulnow);
    }
    double progress = total > 0.0 ? (now / total) : 0.0;
    updateTransferProgress(data->ctx, progress);
    if (data->ctx->running && !*data->ctx->running) {
        return 1;
    }
    return 0;
}

static bool ftpDownloadFile(const Settings& settings, const std::string& remotePath, const fs::path& localPath,
                            TransferContext* ctx, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    FILE* file = std::fopen(localPath.string().c_str(), "wb");
    if (!file) {
        error = "Failed to open local file";
        curl_easy_cleanup(curl);
        return false;
    }

    std::string url = buildFtpUrl(settings, remotePath, false);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    CurlProgressData progressData {ctx};
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);

    CURLcode res = curl_easy_perform(curl);
    std::fclose(file);
    if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }
    if (ctx) {
        updateTransferProgress(ctx, 1.0);
    }
    curl_easy_cleanup(curl);
    return true;
}

static bool ftpUploadFile(const Settings& settings, const fs::path& localPath, const std::string& remotePath,
                          TransferContext* ctx, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    FILE* file = std::fopen(localPath.string().c_str(), "rb");
    if (!file) {
        error = "Failed to open local file";
        curl_easy_cleanup(curl);
        return false;
    }

    curl_off_t fileSize = 0;
    try {
        fileSize = static_cast<curl_off_t>(fs::file_size(localPath));
    } catch (const fs::filesystem_error&) {
    }

    std::string url = buildFtpUrl(settings, remotePath, false);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, curlReadFileCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    CurlProgressData progressData {ctx};
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);

    CURLcode res = curl_easy_perform(curl);
    std::fclose(file);
    if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }
    if (ctx) {
        updateTransferProgress(ctx, 1.0);
    }
    curl_easy_cleanup(curl);
    return true;
}

static bool ftpDeletePath(const Settings& settings, const std::string& remotePath, bool isDir, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    std::string url = buildFtpUrl(settings, "/", true);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    std::string command = std::string(isDir ? "RMD " : "DELE ") + normalizeFtpPath(remotePath);
    struct curl_slist* quote = nullptr;
    quote = curl_slist_append(quote, command.c_str());
    curl_easy_setopt(curl, CURLOPT_QUOTE, quote);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(quote);
    if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_cleanup(curl);
    return true;
}

static bool ftpRenamePath(const Settings& settings, const std::string& fromPath, const std::string& toPath, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    std::string url = buildFtpUrl(settings, "/", true);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    std::string rnfr = "RNFR " + normalizeFtpPath(fromPath);
    std::string rnto = "RNTO " + normalizeFtpPath(toPath);
    struct curl_slist* quote = nullptr;
    quote = curl_slist_append(quote, rnfr.c_str());
    quote = curl_slist_append(quote, rnto.c_str());
    curl_easy_setopt(curl, CURLOPT_QUOTE, quote);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(quote);
    if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_cleanup(curl);
    return true;
}

static bool listFtpEntries(const Settings& settings, const std::string& path, std::vector<Entry>& entries,
                           std::string& error, bool includeHidden) {
    std::string listing;
    if (!fetchFtpList(settings, path, listing, error)) {
        return false;
    }
    std::istringstream lines(listing);
    std::string line;
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Entry entry;
        if (parseFtpListLine(line, entry)) {
            if (!includeHidden && !entry.name.empty() && entry.name[0] == '.') {
                continue;
            }
            entries.push_back(entry);
        }
    }
    sortEntries(entries);
    return true;
}

static bool ftpCreateDir(const Settings& settings, const std::string& remotePath, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!configureFtpHandle(curl, settings, error)) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        return false;
    }

    std::string url = buildFtpUrl(settings, "/", true);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    std::string command = "MKD " + normalizeFtpPath(remotePath);
    struct curl_slist* quote = nullptr;
    quote = curl_slist_append(quote, command.c_str());
    curl_easy_setopt(curl, CURLOPT_QUOTE, quote);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(quote);
    if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_cleanup(curl);
    return true;
}

static bool ftpEnsureDir(const Settings& settings, const std::string& remotePath, std::string& error) {
    std::string clean = normalizeFtpPath(remotePath);
    if (clean == "/") {
        return true;
    }
    size_t pos = 1;
    while (true) {
        pos = clean.find('/', pos);
        std::string segment = (pos == std::string::npos) ? clean : clean.substr(0, pos);
        std::string cmdError;
        ftpCreateDir(settings, segment, cmdError);
        if (pos == std::string::npos) {
            break;
        }
        ++pos;
    }
    return true;
}

static bool ftpEntryExists(const Settings& settings, const std::string& dirPath, const std::string& name, bool& isDir, std::string& error) {
    error.clear();
    std::vector<Entry> entries;
    if (!listFtpEntries(settings, dirPath, entries, error, true)) {
        return false;
    }
    for (const auto& entry : entries) {
        if (entry.name == name) {
            isDir = entry.isDir;
            return true;
        }
    }
    return false;
}

static bool ftpDownloadDirectory(const Settings& settings, const std::string& remotePath, const fs::path& localPath,
                                 TransferContext* ctx, const std::string& title, std::string& error) {
    if (fs::exists(localPath)) {
        error = "Target already exists";
        return false;
    }
    {
        std::error_code ec;
        if (!fs::create_directories(localPath, ec)) {
            error = "Failed to create local directory";
            return false;
        }
    }

    std::vector<Entry> entries;
    if (!listFtpEntries(settings, remotePath, entries, error, true)) {
        return false;
    }
    for (const auto& entry : entries) {
        std::string childRemote = ftpJoinPath(remotePath, entry.name);
        fs::path childLocal = localPath / entry.name;
        if (entry.isDir) {
            if (!ftpDownloadDirectory(settings, childRemote, childLocal, ctx, title, error)) {
                return false;
            }
        } else {
            if (fs::exists(childLocal)) {
                error = "Target already exists";
                return false;
            }
            if (ctx) {
                startTransferItem(ctx, title, entry.name);
            }
            if (!ftpDownloadFile(settings, childRemote, childLocal, ctx, error)) {
                return false;
            }
        }
    }
    return true;
}

static bool ftpUploadDirectory(const Settings& settings, const fs::path& localPath, const std::string& remotePath,
                               TransferContext* ctx, const std::string& title, std::string& error) {
    if (!fs::is_directory(localPath)) {
        error = "Source is not a directory";
        return false;
    }
    if (!ftpEnsureDir(settings, remotePath, error)) {
        return false;
    }

    for (const auto& item : fs::directory_iterator(localPath)) {
        const fs::path& childPath = item.path();
        std::string childRemote = ftpJoinPath(remotePath, childPath.filename().string());
        if (item.is_directory()) {
            if (!ftpUploadDirectory(settings, childPath, childRemote, ctx, title, error)) {
                return false;
            }
        } else if (item.is_regular_file()) {
            if (ctx) {
                startTransferItem(ctx, title, childPath.filename().string());
            }
            if (!ftpUploadFile(settings, childPath, childRemote, ctx, error)) {
                return false;
            }
        }
    }
    return true;
}

static bool ftpDeleteRecursive(const Settings& settings, const std::string& remotePath, std::string& error) {
    std::vector<Entry> entries;
    if (!listFtpEntries(settings, remotePath, entries, error, true)) {
        return false;
    }
    for (const auto& entry : entries) {
        std::string childRemote = ftpJoinPath(remotePath, entry.name);
        if (entry.isDir) {
            if (!ftpDeleteRecursive(settings, childRemote, error)) {
                return false;
            }
        } else {
            if (!ftpDeletePath(settings, childRemote, false, error)) {
                return false;
            }
        }
    }
    return ftpDeletePath(settings, remotePath, true, error);
}

#endif

static bool copyLocalFileWithProgress(const fs::path& src, const fs::path& dst, TransferContext* ctx,
                                      const std::string& title, const std::string& label, bool resetCount,
                                      std::string& error) {
    if (fs::exists(dst)) {
        error = "Target already exists";
        return false;
    }
    std::ifstream input(src, std::ios::binary);
    if (!input.is_open()) {
        error = "Failed to open source file";
        return false;
    }
    std::ofstream output(dst, std::ios::binary);
    if (!output.is_open()) {
        error = "Failed to open target file";
        return false;
    }

    std::uintmax_t totalSize = 0;
    try {
        totalSize = fs::file_size(src);
    } catch (const fs::filesystem_error&) {
    }

    if (ctx) {
        startTransferItem(ctx, title, label, resetCount);
    }

    const size_t bufferSize = 64 * 1024;
    std::vector<char> buffer(bufferSize);
    std::uintmax_t totalRead = 0;
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        std::streamsize bytes = input.gcount();
        if (bytes <= 0) {
            break;
        }
        output.write(buffer.data(), bytes);
        if (!output) {
            error = "Failed to write target file";
            return false;
        }
        totalRead += static_cast<std::uintmax_t>(bytes);
        if (ctx && totalSize > 0) {
            updateTransferProgress(ctx, static_cast<double>(totalRead) / static_cast<double>(totalSize));
        }
        if (ctx && ctx->running && !*ctx->running) {
            error = "Transfer cancelled";
            return false;
        }
    }
    if (ctx) {
        updateTransferProgress(ctx, 1.0);
    }
    return true;
}

static bool copyLocalDirectoryWithProgress(const fs::path& srcDir, const fs::path& dstDir, TransferContext* ctx,
                                           const std::string& title, std::string& error) {
    if (fs::exists(dstDir)) {
        error = "Target already exists";
        return false;
    }
    std::error_code ec;
    if (!fs::create_directories(dstDir, ec)) {
        error = "Failed to create target directory";
        return false;
    }
    int totalFiles = 0;
    for (const auto& entry : fs::recursive_directory_iterator(srcDir, ec)) {
        if (ec) {
            break;
        }
        if (entry.is_regular_file(ec)) {
            ++totalFiles;
        }
    }
    int copiedFiles = 0;
    if (ctx && totalFiles > 0) {
        updateTransferCount(ctx, 0, totalFiles);
    }
    ec.clear();
    for (const auto& entry : fs::recursive_directory_iterator(srcDir, ec)) {
        if (ec) {
            break;
        }
        const fs::path& path = entry.path();
        fs::path relative = fs::relative(path, srcDir, ec);
        if (ec) {
            relative = path.filename();
        }
        fs::path target = dstDir / relative;
        if (entry.is_directory()) {
            std::error_code dirEc;
            fs::create_directories(target, dirEc);
        } else if (entry.is_regular_file()) {
            if (!copyLocalFileWithProgress(path, target, ctx, title, relative.string(), false, error)) {
                return false;
            }
            ++copiedFiles;
            if (ctx && totalFiles > 0) {
                updateTransferCount(ctx, copiedFiles, totalFiles);
            }
        }
    }
    return true;
}

static bool copyLocalPathWithProgress(const fs::path& src, const fs::path& dst, TransferContext* ctx,
                                      const std::string& title, std::string& error) {
    if (fs::is_directory(src)) {
        return copyLocalDirectoryWithProgress(src, dst, ctx, title, error);
    }
    return copyLocalFileWithProgress(src, dst, ctx, title, src.filename().string(), true, error);
}

static int countZipEntries(const fs::path& zipPath, std::string& error) {
#ifdef _WIN32
    std::string command = "tar -tf " + quoteArg(zipPath.string()) + " 2>&1";
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
        error = "Failed to run tar";
        return -1;
    }
    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = trimWhitespace(buffer);
        if (!line.empty() && line.rfind("tar:", 0) != 0 && line.rfind("bsdtar:", 0) != 0) {
            ++count;
        }
    }
    int status = closePipe(pipe);
    if (status != 0) {
        error = "tar failed";
        return -1;
    }
    return count;
#else
    std::string command = "unzip -Z -1 " + quoteArg(zipPath.string()) + " 2>&1";
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
        error = "Failed to run unzip";
        return -1;
    }
    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = trimWhitespace(buffer);
        if (!line.empty()) {
            ++count;
        }
    }
    int status = closePipe(pipe);
    if (status != 0) {
        error = "unzip failed";
        return -1;
    }
    return count;
#endif
}

static bool parseUnzipOutputLine(const std::string& line, std::string& item) {
    std::string trimmed = trimWhitespace(line);
    if (trimmed.empty()) {
        return false;
    }
    size_t colon = trimmed.find(':');
    if (colon == std::string::npos) {
        return false;
    }
    std::string prefix = toLower(trimWhitespace(trimmed.substr(0, colon)));
    if (prefix == "inflating" || prefix == "extracting" || prefix == "creating") {
        item = trimWhitespace(trimmed.substr(colon + 1));
        return !item.empty();
    }
    return false;
}

static bool extractZipWithProgress(const fs::path& zipPath, const fs::path& destDir, TransferContext* ctx,
                                   std::string& error) {
    if (!fs::exists(destDir)) {
        error = "Target directory not found";
        return false;
    }

    int totalEntries = 0;
    std::string listError;
    int count = countZipEntries(zipPath, listError);
    if (count > 0) {
        totalEntries = count;
    }

    if (ctx) {
        startTransferItem(ctx, "Extracting", zipPath.filename().string());
        if (totalEntries > 0) {
            updateTransferCount(ctx, 0, totalEntries);
        }
    }

#ifdef _WIN32
    std::string command = "tar -xf " + quoteArg(zipPath.string()) + " -C " + quoteArg(destDir.string()) + " -v 2>&1";
#else
    std::string command = "unzip -o " + quoteArg(zipPath.string()) + " -d " + quoteArg(destDir.string()) + " 2>&1";
#endif
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
#ifdef _WIN32
        error = "Failed to run tar";
#else
        error = "Failed to run unzip";
#endif
        return false;
    }

    int extracted = 0;
    std::string lastLine;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = buffer;
        std::string trimmed = trimWhitespace(line);
        if (!trimmed.empty()) {
            lastLine = trimmed;
        }
        std::string item;
#ifdef _WIN32
        std::string trimmedItem = trimWhitespace(line);
        if (trimmedItem.rfind("x ", 0) == 0) {
            trimmedItem = trimWhitespace(trimmedItem.substr(2));
        }
        if (!trimmedItem.empty() && trimmedItem.rfind("tar:", 0) != 0 && trimmedItem.rfind("bsdtar:", 0) != 0) {
            item = trimmedItem;
#else
        if (parseUnzipOutputLine(line, item)) {
#endif
            ++extracted;
            if (ctx && ctx->transfer) {
                ctx->transfer->item = item;
                if (totalEntries > 0) {
                    updateTransferProgress(ctx, static_cast<double>(extracted) / static_cast<double>(totalEntries));
                    updateTransferCount(ctx, extracted, totalEntries);
                } else {
                    updateTransferProgress(ctx, 0.0);
                }
            }
        }
    }

    int status = closePipe(pipe);
    if (status != 0) {
#ifdef _WIN32
        error = lastLine.empty() ? "tar failed" : "tar failed: " + lastLine;
#else
        error = lastLine.empty() ? "unzip failed" : "unzip failed: " + lastLine;
#endif
        return false;
    }
    if (ctx) {
        updateTransferProgress(ctx, 1.0);
    }
    return true;
}

static int countRarEntries(const fs::path& rarPath, std::string& error) {
#ifdef _WIN32
    std::string command = "tar -tf " + quoteArg(rarPath.string()) + " 2>&1";
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
        error = "Failed to run tar";
        return -1;
    }
    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = trimWhitespace(buffer);
        if (!line.empty() && line.rfind("tar:", 0) != 0 && line.rfind("bsdtar:", 0) != 0) {
            ++count;
        }
    }
    int status = closePipe(pipe);
    if (status != 0) {
        error = "tar failed";
        return -1;
    }
    return count;
#else
    std::string command = "unrar lb " + quoteArg(rarPath.string()) + " 2>&1";
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
        error = "Failed to run unrar";
        return -1;
    }
    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = trimWhitespace(buffer);
        if (!line.empty()) {
            ++count;
        }
    }
    int status = closePipe(pipe);
    if (status != 0) {
        error = "unrar failed";
        return -1;
    }
    return count;
#endif
}

static bool parseUnrarOutputLine(const std::string& line, std::string& item) {
    std::string trimmed = trimWhitespace(line);
    if (trimmed.empty()) {
        return false;
    }
    std::string lowered = toLower(trimmed);
    if (lowered.rfind("extracting ", 0) == 0) {
        item = trimWhitespace(trimmed.substr(std::string("extracting ").size()));
        return !item.empty();
    }
    if (lowered.rfind("creating ", 0) == 0) {
        item = trimWhitespace(trimmed.substr(std::string("creating ").size()));
        return !item.empty();
    }
    return false;
}

static bool extractRarWithProgress(const fs::path& rarPath, const fs::path& destDir, TransferContext* ctx,
                                   std::string& error) {
    if (!fs::exists(destDir)) {
        error = "Target directory not found";
        return false;
    }

    int totalEntries = 0;
    std::string listError;
    int count = countRarEntries(rarPath, listError);
    if (count > 0) {
        totalEntries = count;
    }

    if (ctx) {
        startTransferItem(ctx, "Extracting", rarPath.filename().string());
        if (totalEntries > 0) {
            updateTransferCount(ctx, 0, totalEntries);
        }
    }

#ifdef _WIN32
    std::string command = "tar -xf " + quoteArg(rarPath.string()) + " -C " + quoteArg(destDir.string()) + " -v 2>&1";
#else
    std::string command = "unrar x -o+ -y " + quoteArg(rarPath.string()) + " " + quoteArg(destDir.string()) + " 2>&1";
#endif
    FILE* pipe = openPipe(command, "r");
    if (!pipe) {
#ifdef _WIN32
        error = "Failed to run tar";
#else
        error = "Failed to run unrar";
#endif
        return false;
    }

    int extracted = 0;
    std::string lastLine;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = buffer;
        std::string trimmed = trimWhitespace(line);
        if (!trimmed.empty()) {
            lastLine = trimmed;
        }
        std::string item;
#ifdef _WIN32
        std::string trimmedItem = trimWhitespace(line);
        if (trimmedItem.rfind("x ", 0) == 0) {
            trimmedItem = trimWhitespace(trimmedItem.substr(2));
        }
        if (!trimmedItem.empty() && trimmedItem.rfind("tar:", 0) != 0 && trimmedItem.rfind("bsdtar:", 0) != 0) {
            item = trimmedItem;
#else
        if (parseUnrarOutputLine(line, item)) {
#endif
            ++extracted;
            if (ctx && ctx->transfer) {
                ctx->transfer->item = item;
                if (totalEntries > 0) {
                    updateTransferProgress(ctx, static_cast<double>(extracted) / static_cast<double>(totalEntries));
                    updateTransferCount(ctx, extracted, totalEntries);
                } else {
                    updateTransferProgress(ctx, 0.0);
                }
            }
        }
    }

    int status = closePipe(pipe);
    if (status != 0) {
#ifdef _WIN32
        error = lastLine.empty() ? "tar failed" : "tar failed: " + lastLine;
#else
        error = lastLine.empty() ? "unrar failed" : "unrar failed: " + lastLine;
#endif
        return false;
    }
    if (ctx) {
        updateTransferProgress(ctx, 1.0);
    }
    return true;
}

static bool parseFtpListLine(const std::string& line, Entry& entry) {
    if (line.empty()) {
        return false;
    }
    if (line.rfind("total ", 0) == 0) {
        return false;
    }
    if (line.find("type=") != std::string::npos) {
        bool isDir = (line.find("type=dir") != std::string::npos || line.find("type=cdir") != std::string::npos ||
                      line.find("type=pdir") != std::string::npos);
        size_t nameStart = line.find(' ');
        if (nameStart == std::string::npos) {
            return false;
        }
        while (nameStart < line.size() && line[nameStart] == ' ') {
            ++nameStart;
        }
        if (nameStart >= line.size()) {
            return false;
        }
        std::string name = line.substr(nameStart);
        if (name == "." || name == "..") {
            return false;
        }
        entry.name = name;
        entry.isDir = isDir;
        entry.isParent = false;
        entry.path.clear();
        return !entry.name.empty();
    }
    bool isDir = (line[0] == 'd');
    int tokens = 0;
    bool inToken = false;
    size_t nameStart = std::string::npos;
    for (size_t i = 0; i < line.size(); ++i) {
        if (!std::isspace(static_cast<unsigned char>(line[i]))) {
            if (!inToken) {
                inToken = true;
                ++tokens;
                if (tokens == 9) {
                    nameStart = i;
                    break;
                }
            }
        } else {
            inToken = false;
        }
    }
    if (nameStart == std::string::npos) {
        entry.name = line;
        entry.isDir = false;
        entry.isParent = false;
        entry.path.clear();
        return !entry.name.empty();
    }
    std::string name = line.substr(nameStart);
    size_t arrow = name.find(" -> ");
    if (arrow != std::string::npos) {
        name = name.substr(0, arrow);
    }
    if (!name.empty() && name.back() == '/') {
        name.pop_back();
        isDir = true;
    }
    if (name == "." || name == "..") {
        return false;
    }
    entry.name = name;
    entry.isDir = isDir;
    entry.isParent = false;
    entry.path.clear();
    return !entry.name.empty();
}

static void loadLocalEntries(Pane& pane, const Settings& settings) {
    pane.entries.clear();
    pane.lastLocalCwd = pane.cwd;
    bool hasParent = pane.cwd.has_parent_path();
    if (hasParent) {
        pane.entries.push_back({"..", pane.cwd.parent_path(), true, true});
    }

    std::vector<Entry> collected;
    try {
        for (const auto& item : fs::directory_iterator(pane.cwd)) {
            Entry entry;
            entry.path = item.path();
            entry.name = item.path().filename().string();
            if (!settings.showHidden && !entry.name.empty() && entry.name[0] == '.') {
                continue;
            }
            entry.isDir = item.is_directory();
            entry.isParent = false;
            collected.push_back(entry);
        }
    } catch (const fs::filesystem_error&) {
    }

    sortEntries(collected);
    pane.entries.insert(pane.entries.end(), collected.begin(), collected.end());
}

static void loadFtpEntries(Pane& pane, const Settings& settings, StatusMessage* status) {
    pane.entries.clear();
    pane.ftpPath = normalizeFtpPath(pane.ftpPath);
    if (pane.ftpPath != "/") {
        pane.entries.push_back({"..", {}, true, true});
    }

#ifdef USE_CURL
    if (settings.ftpHost.empty()) {
        if (status) {
            setStatus(*status, "FTP host not set");
        }
        return;
    }
    std::string error;
    std::vector<Entry> collected;
    if (!listFtpEntries(settings, pane.ftpPath, collected, error, settings.showHidden)) {
        if (status) {
            setStatus(*status, "FTP error: " + error);
        }
        return;
    }

    pane.entries.insert(pane.entries.end(), collected.begin(), collected.end());
#else
    if (status) {
        setStatus(*status, "FTP support not built");
    }
#endif
}

static void loadEntries(Pane& pane, const Settings& settings, StatusMessage* status = nullptr) {
    if (pane.source == PaneSource::Ftp) {
        loadFtpEntries(pane, settings, status);
    } else {
        loadLocalEntries(pane, settings);
    }

    if (pane.selected >= static_cast<int>(pane.entries.size())) {
        pane.selected = static_cast<int>(pane.entries.size()) - 1;
    }
    if (pane.selected < 0) {
        pane.selected = 0;
    }
    if (pane.scroll > pane.selected) {
        pane.scroll = pane.selected;
    }
}

static void resetPanePosition(Pane& pane) {
    pane.selected = 0;
    pane.scroll = 0;
}

static void connectToFtp(Pane& pane, const Settings& settings, StatusMessage& status) {
    if (settings.ftpHost.empty()) {
        setStatus(status, "FTP host not set");
        return;
    }
    pane.source = PaneSource::Ftp;
    pane.ftpPath = "/";
    resetPanePosition(pane);
    loadEntries(pane, settings, &status);
}

static bool copyBetweenPanes(const Entry& entry, const Pane& src, const Pane& dst, const Settings& settings,
                             TransferContext* ctx, const std::string& title, std::string& error) {
    if (src.source == PaneSource::Local && dst.source == PaneSource::Local) {
        fs::path target = dst.cwd / entry.name;
        return copyLocalPathWithProgress(entry.path, target, ctx, title, error);
    }
#ifdef USE_CURL
    if (src.source == PaneSource::Ftp && dst.source == PaneSource::Local) {
        fs::path target = dst.cwd / entry.name;
        if (fs::exists(target)) {
            error = "Target already exists";
            return false;
        }
        std::string remotePath = ftpJoinPath(src.ftpPath, entry.name);
        if (ctx) {
            startTransferItem(ctx, title, entry.name);
        }
        if (entry.isDir) {
            return ftpDownloadDirectory(settings, remotePath, target, ctx, title, error);
        }
        return ftpDownloadFile(settings, remotePath, target, ctx, error);
    }
    if (src.source == PaneSource::Local && dst.source == PaneSource::Ftp) {
        bool isDir = false;
        if (ftpEntryExists(settings, dst.ftpPath, entry.name, isDir, error)) {
            error = "Target already exists";
            return false;
        }
        if (!error.empty()) {
            return false;
        }
        std::string remotePath = ftpJoinPath(dst.ftpPath, entry.name);
        if (ctx) {
            startTransferItem(ctx, title, entry.name);
        }
        if (entry.isDir) {
            return ftpUploadDirectory(settings, entry.path, remotePath, ctx, title, error);
        }
        return ftpUploadFile(settings, entry.path, remotePath, ctx, error);
    }
#endif
    error = "FTP copy not supported";
    return false;
}

static bool deleteFromPane(const Pane& pane, const Entry& entry, const Settings& settings, std::string& error) {
    if (pane.source == PaneSource::Local) {
        return deleteEntry(entry, error);
    }
#ifdef USE_CURL
    std::string remotePath = ftpJoinPath(pane.ftpPath, entry.name);
    if (entry.isDir) {
        return ftpDeleteRecursive(settings, remotePath, error);
    }
    return ftpDeletePath(settings, remotePath, false, error);
#else
    error = "FTP delete not supported";
    return false;
#endif
}

static bool renameInPane(const Pane& pane, const Entry& entry, const Settings& settings, const std::string& newName, std::string& error) {
    if (pane.source == PaneSource::Local) {
        return renameEntry(entry, newName, error);
    }
#ifdef USE_CURL
    std::string fromPath = ftpJoinPath(pane.ftpPath, entry.name);
    std::string toPath = ftpJoinPath(pane.ftpPath, newName);
    return ftpRenamePath(settings, fromPath, toPath, error);
#else
    error = "FTP rename not supported";
    return false;
#endif
}

static bool createDirInPane(const Pane& pane, const Settings& settings, const std::string& name, std::string& error) {
    if (pane.source == PaneSource::Local) {
        if (name.empty()) {
            error = "Folder name required";
            return false;
        }
        fs::path target = pane.cwd / name;
        if (fs::exists(target)) {
            error = "Target already exists";
            return false;
        }
        std::error_code ec;
        if (!fs::create_directory(target, ec) || ec) {
            error = ec ? ec.message() : "Failed to create folder";
            return false;
        }
        return true;
    }
#ifdef USE_CURL
    if (name.empty()) {
        error = "Folder name required";
        return false;
    }
    bool isDir = false;
    if (ftpEntryExists(settings, pane.ftpPath, name, isDir, error)) {
        error = "Target already exists";
        return false;
    }
    if (!error.empty()) {
        return false;
    }
    std::string remotePath = ftpJoinPath(pane.ftpPath, name);
    return ftpCreateDir(settings, remotePath, error);
#else
    error = "FTP create not supported";
    return false;
#endif
}

static bool moveBetweenPanes(const Entry& entry, const Pane& src, const Pane& dst, const Settings& settings,
                             TransferContext* ctx, const std::string& title, std::string& error) {
    if (src.source == PaneSource::Local && dst.source == PaneSource::Local) {
        fs::path target = dst.cwd / entry.name;
        if (!copyLocalPathWithProgress(entry.path, target, ctx, title, error)) {
            return false;
        }
        std::error_code ec;
        if (entry.isDir) {
            fs::remove_all(entry.path, ec);
        } else {
            fs::remove(entry.path, ec);
        }
        if (ec) {
            error = "Move delete failed";
            return false;
        }
        return true;
    }
    if (!copyBetweenPanes(entry, src, dst, settings, ctx, title, error)) {
        return false;
    }
    std::string deleteError;
    if (!deleteFromPane(src, entry, settings, deleteError)) {
        error = "Move delete failed: " + deleteError;
        return false;
    }
    return true;
}

static void handleActionSelection(int menuIndex,
                                  ActionContext& action,
                                  Pane panes[2],
                                  Settings& settings,
                                  StatusMessage& status,
                                  Mode& mode,
                                  int& confirmIndex,
                                  std::string& renameBuffer,
                                  size_t& renameCursor,
                                  std::string& createFolderName,
                                  size_t& createFolderCursor,
                                  std::string& addToSteamName,
                                  size_t& addToSteamCursor,
                                  OskState& osk,
                                  TransferContext* transferCtx) {
    auto options = buildActionOptions(action.entry, panes[action.paneIndex]);
    if (options.empty()) {
        mode = Mode::Browse;
        return;
    }
    if (menuIndex < 0 || menuIndex >= static_cast<int>(options.size())) {
        menuIndex = 0;
    }
    const std::string& option = options[static_cast<size_t>(menuIndex)];

    if (option == "Delete") {
        mode = Mode::ConfirmDelete;
        confirmIndex = 1;
        return;
    }
    if (option == "Rename") {
        renameBuffer = action.entry.name;
        renameCursor = renameBuffer.size();
        osk = {};
        mode = Mode::Rename;
        SDL_SetHint(SDL_HINT_ENABLE_SCREEN_KEYBOARD, "0");
        SDL_StartTextInput();
        return;
    }
    if (option == "Create New Folder") {
        createFolderName = "New Folder";
        createFolderCursor = createFolderName.size();
        osk = {};
        mode = Mode::CreateFolder;
        SDL_SetHint(SDL_HINT_ENABLE_SCREEN_KEYBOARD, "0");
        SDL_StartTextInput();
        return;
    }
    if (option == "Add to Steam") {
        addToSteamName = defaultSteamAppName(action.entry);
        addToSteamCursor = addToSteamName.size();
        osk = {};
        mode = Mode::AddToSteam;
        SDL_SetHint(SDL_HINT_ENABLE_SCREEN_KEYBOARD, "0");
        SDL_StartTextInput();
        return;
    }
    if (option == "Extract") {
        std::string error;
        bool ok = false;
        if (isZipArchive(action.entry, panes[action.paneIndex])) {
            ok = extractZipWithProgress(action.entry.path, panes[action.paneIndex].cwd, transferCtx, error);
        } else if (isRarArchive(action.entry, panes[action.paneIndex])) {
            ok = extractRarWithProgress(action.entry.path, panes[action.paneIndex].cwd, transferCtx, error);
        } else {
            error = "Unsupported archive";
        }
        setStatus(status, ok ? "Extracted" : ("Extract failed: " + error));
        if (transferCtx) {
            finishTransfer(transferCtx);
        }
        loadEntries(panes[action.paneIndex], settings, &status);
        mode = Mode::Browse;
        return;
    }

    Pane& src = panes[action.paneIndex];
    Pane& dst = panes[1 - action.paneIndex];
    std::string error;
    bool ok = false;
    if (option == "Copy") {
        ok = copyBetweenPanes(action.entry, src, dst, settings, transferCtx, "Copying", error);
        setStatus(status, ok ? "Copied" : ("Copy failed: " + error));
    } else if (option == "Move") {
        ok = moveBetweenPanes(action.entry, src, dst, settings, transferCtx, "Moving", error);
        setStatus(status, ok ? "Moved" : ("Move failed: " + error));
    }
    if (transferCtx) {
        finishTransfer(transferCtx);
    }
    loadEntries(src, settings, &status);
    loadEntries(dst, settings, &status);
    mode = Mode::Browse;
}

static void ensureVisible(Pane& pane, int visibleRows) {
    if (pane.selected < pane.scroll) {
        pane.scroll = pane.selected;
    } else if (pane.selected >= pane.scroll + visibleRows) {
        pane.scroll = pane.selected - visibleRows + 1;
    }
    if (pane.scroll < 0) {
        pane.scroll = 0;
    }
}

static void setStatus(StatusMessage& status, const std::string& text) {
    status.text = text;
    status.started = std::chrono::steady_clock::now();
}

static bool supportsAddToSteam() {
    return true;
}

static bool isWindowsExe(const Entry& entry, const Pane& pane) {
    if (pane.source != PaneSource::Local) {
        return false;
    }
    if (entry.isDir || entry.isParent) {
        return false;
    }
    std::string ext = toLower(entry.path.extension().string());
    return ext == ".exe";
}

static bool isAppImage(const Entry& entry, const Pane& pane) {
    if (pane.source != PaneSource::Local) {
        return false;
    }
    if (entry.isDir || entry.isParent) {
        return false;
    }
    std::string ext = toLower(entry.path.extension().string());
    return ext == ".appimage";
}

static bool isShellScript(const Entry& entry, const Pane& pane) {
    if (pane.source != PaneSource::Local) {
        return false;
    }
    if (entry.isDir || entry.isParent) {
        return false;
    }
    std::string ext = toLower(entry.path.extension().string());
    return ext == ".sh";
}

static bool isZipArchive(const Entry& entry, const Pane& pane) {
    if (pane.source != PaneSource::Local) {
        return false;
    }
    if (entry.isDir || entry.isParent) {
        return false;
    }
    std::string ext = toLower(entry.path.extension().string());
    return ext == ".zip";
}

static bool isRarArchive(const Entry& entry, const Pane& pane) {
    if (pane.source != PaneSource::Local) {
        return false;
    }
    if (entry.isDir || entry.isParent) {
        return false;
    }
    std::string ext = toLower(entry.path.extension().string());
    return ext == ".rar";
}

static std::string defaultSteamAppName(const Entry& entry) {
    std::string name = entry.path.stem().string();
    if (name.empty()) {
        return entry.name;
    }
    return name;
}

static std::vector<std::string> buildActionOptions(const Entry& entry, const Pane& pane) {
    std::vector<std::string> options = {"Copy", "Move", "Delete", "Rename", "Create New Folder"};
    if (isZipArchive(entry, pane) || isRarArchive(entry, pane)) {
        options.insert(options.begin() + 2, "Extract");
    }
    if (supportsAddToSteam() && (isWindowsExe(entry, pane) || isAppImage(entry, pane) || isShellScript(entry, pane))) {
        options.push_back("Add to Steam");
    }
    return options;
}

static bool statusActive(const StatusMessage& status) {
    using namespace std::chrono;
    return !status.text.empty() && (steady_clock::now() - status.started) < seconds(4);
}

static bool copyEntry(const Entry& entry, const fs::path& targetDir, std::string& error) {
    fs::path target = targetDir / entry.name;
    if (fs::exists(target)) {
        error = "Target already exists";
        return false;
    }
    try {
        if (entry.isDir) {
            fs::copy(entry.path, target, fs::copy_options::recursive);
        } else {
            fs::copy_file(entry.path, target);
        }
    } catch (const fs::filesystem_error& ex) {
        error = ex.what();
        return false;
    }
    return true;
}

static bool moveEntry(const Entry& entry, const fs::path& targetDir, std::string& error) {
    fs::path target = targetDir / entry.name;
    if (fs::exists(target)) {
        error = "Target already exists";
        return false;
    }
    try {
        fs::rename(entry.path, target);
        return true;
    } catch (const fs::filesystem_error&) {
        if (!copyEntry(entry, targetDir, error)) {
            return false;
        }
        try {
            if (entry.isDir) {
                fs::remove_all(entry.path);
            } else {
                fs::remove(entry.path);
            }
        } catch (const fs::filesystem_error& ex) {
            error = ex.what();
            return false;
        }
    }
    return true;
}

static bool deleteEntry(const Entry& entry, std::string& error) {
    try {
        if (entry.isDir) {
            fs::remove_all(entry.path);
        } else {
            fs::remove(entry.path);
        }
    } catch (const fs::filesystem_error& ex) {
        error = ex.what();
        return false;
    }
    return true;
}

static bool renameEntry(const Entry& entry, const std::string& newName, std::string& error) {
    fs::path target = entry.path.parent_path() / newName;
    if (fs::exists(target)) {
        error = "Target already exists";
        return false;
    }
    try {
        fs::rename(entry.path, target);
    } catch (const fs::filesystem_error& ex) {
        error = ex.what();
        return false;
    }
    return true;
}

static void enterSelected(Pane& pane, const Settings& settings, StatusMessage* status) {
    if (pane.entries.empty()) {
        return;
    }
    Entry entry = pane.entries[static_cast<size_t>(pane.selected)];
    if (pane.source == PaneSource::Ftp) {
        if (entry.isParent) {
            pane.ftpPath = ftpParentPath(pane.ftpPath);
            resetPanePosition(pane);
            loadEntries(pane, settings, status);
            return;
        }
        if (entry.isDir) {
            pane.ftpPath = ftpJoinPath(pane.ftpPath, entry.name);
            resetPanePosition(pane);
            loadEntries(pane, settings, status);
        } else if (status) {
            setStatus(*status, "Open not supported on FTP");
        }
        return;
    }

    if (entry.isParent) {
        pane.cwd = entry.path;
        resetPanePosition(pane);
        loadEntries(pane, settings, status);
        return;
    }
    if (entry.isDir) {
        pane.cwd = entry.path;
        resetPanePosition(pane);
        loadEntries(pane, settings, status);
        return;
    }
    if (status) {
        std::string error;
        if (openLocalFile(entry.path, error)) {
            setStatus(*status, "Opened");
        } else {
            setStatus(*status, "Open failed: " + error);
        }
    }
}

static void goUp(Pane& pane, const Settings& settings, StatusMessage* status) {
    if (pane.source == PaneSource::Ftp) {
        pane.ftpPath = ftpParentPath(pane.ftpPath);
        resetPanePosition(pane);
        loadEntries(pane, settings, status);
        return;
    }
    if (pane.cwd.has_parent_path()) {
        pane.cwd = pane.cwd.parent_path();
        resetPanePosition(pane);
        loadEntries(pane, settings, status);
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

#ifdef __linux__
    // Keep the compositor active so the Steam OSK/overlay can appear above fullscreen.
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

#ifdef USE_CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif

    Settings settings;
    fs::path homePath = getHomePath();
    settings.panePath[0] = homePath;
    settings.panePath[1] = homePath;

    std::string configPath;
    char* prefPath = SDL_GetPrefPath("GamepadCommander", "GamepadCommander");
    if (prefPath) {
        configPath = std::string(prefPath) + "config.xml";
        SDL_free(prefPath);
    }
    if (!configPath.empty()) {
        loadConfig(settings, configPath, homePath);
    }
    settings.uiScale = clampUiScale(settings.uiScale);

    SDL_Window* window = SDL_CreateWindow(
        "GamepadCommander",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    std::unordered_map<SDL_JoystickID, SDL_GameController*> controllers;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (!SDL_IsGameController(i)) {
            continue;
        }
        SDL_GameController* controller = SDL_GameControllerOpen(i);
        if (!controller) {
            continue;
        }
        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
        SDL_JoystickID instanceId = joystick ? SDL_JoystickInstanceID(joystick) : -1;
        if (instanceId == -1 || controllers.find(instanceId) != controllers.end()) {
            SDL_GameControllerClose(controller);
            continue;
        }
        controllers.emplace(instanceId, controller);
    }

    Pane panes[2];
    for (int i = 0; i < 2; ++i) {
        panes[i].source = PaneSource::Local;
        panes[i].cwd = settings.panePath[i];
        panes[i].lastLocalCwd = panes[i].cwd;
        panes[i].ftpPath = "/";
        loadEntries(panes[i], settings);
    }

    int activePane = 0;
    Mode mode = Mode::Browse;
    int menuIndex = 0;
    int confirmIndex = 1;
    int quitConfirmIndex = 1;
    int appMenuIndex = 0;
    int settingsIndex = 0;
    ActionContext action;
    StatusMessage status;

    const std::array<std::string, 3> appMenuOptions = {"Settings", "Connect to FTP", "Quit"};
    const std::array<std::string, 9> settingsOptions = {"FTP Host",
                                                        "FTP Port",
                                                        "FTP User",
                                                        "FTP Password",
                                                        "Steam Launch Options",
                                                        "Steam Compatibility Tool",
                                                        "UI Scale",
                                                        "Show Hidden",
                                                        "Back"};

    std::string renameBuffer;
    std::string createFolderName;
    std::string addToSteamName;
    size_t renameCursor = 0;
    size_t createFolderCursor = 0;
    size_t addToSteamCursor = 0;
    OskState osk;
    std::string editBuffer;
    size_t editCursor = 0;
    SettingField editField = SettingField::FtpHost;
    TransferState transfer;
    std::unordered_set<SDL_JoystickID> leftTriggerHeld;
    std::unordered_set<SDL_JoystickID> rightTriggerHeld;

    bool running = true;
    TransferContext transferCtx {window, renderer, &settings, &running, &transfer};
    while (running) {
        auto commitRename = [&]() {
            std::string error;
            if (!renameBuffer.empty() && renameBuffer != action.entry.name) {
                if (renameInPane(panes[action.paneIndex], action.entry, settings, renameBuffer, error)) {
                    setStatus(status, "Renamed");
                    loadEntries(panes[action.paneIndex], settings, &status);
                } else {
                    setStatus(status, "Rename failed: " + error);
                }
            }
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto cancelRename = [&]() {
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto commitCreateFolder = [&]() {
            std::string error;
            if (createFolderName.empty()) {
                setStatus(status, "Folder name required");
            } else if (createDirInPane(panes[action.paneIndex], settings, createFolderName, error)) {
                setStatus(status, "Folder created");
                loadEntries(panes[action.paneIndex], settings, &status);
            } else {
                setStatus(status, "Create folder failed: " + error);
            }
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto cancelCreateFolder = [&]() {
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto commitAddToSteam = [&]() {
            std::string error;
            if (addExeToSteam(action.entry.path, addToSteamName, settings.steamLaunchOptions,
                              settings.steamCompatibilityToolVersion, error)) {
                if (!error.empty()) {
                    setStatus(status, "Added to Steam, but: " + error);
                } else {
                    setStatus(status, "Added to Steam: " + addToSteamName);
                }
            } else {
                setStatus(status, "Add to Steam failed: " + error);
            }
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto cancelAddToSteam = [&]() {
            mode = Mode::Browse;
            SDL_StopTextInput();
        };
        auto commitEdit = [&]() {
            if (editField == SettingField::FtpHost) {
                settings.ftpHost = editBuffer;
            } else if (editField == SettingField::FtpPort) {
                if (!editBuffer.empty()) {
                    try {
                        int value = std::stoi(editBuffer);
                        settings.ftpPort = std::clamp(value, 1, 65535);
                    } catch (const std::exception&) {
                    }
                }
            } else if (editField == SettingField::FtpUser) {
                settings.ftpUser = editBuffer;
            } else if (editField == SettingField::FtpPass) {
                settings.ftpPass = editBuffer;
            } else if (editField == SettingField::SteamLaunchOptions) {
                settings.steamLaunchOptions = editBuffer;
            } else if (editField == SettingField::SteamCompatibilityTool) {
                settings.steamCompatibilityToolVersion = editBuffer;
            }
            mode = Mode::Settings;
            SDL_StopTextInput();
        };
        auto cancelEdit = [&]() {
            mode = Mode::Settings;
            SDL_StopTextInput();
        };

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_CONTROLLERDEVICEADDED) {
                if (SDL_IsGameController(event.cdevice.which)) {
                    SDL_GameController* added = SDL_GameControllerOpen(event.cdevice.which);
                    if (added) {
                        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(added);
                        SDL_JoystickID instanceId = joystick ? SDL_JoystickInstanceID(joystick) : -1;
                        if (instanceId == -1 || controllers.find(instanceId) != controllers.end()) {
                            SDL_GameControllerClose(added);
                        } else {
                            controllers.emplace(instanceId, added);
                        }
                    }
                }
            }
            if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                SDL_JoystickID removedId = event.cdevice.which;
                auto it = controllers.find(removedId);
                if (it != controllers.end()) {
                    SDL_GameControllerClose(it->second);
                    controllers.erase(it);
                }
                leftTriggerHeld.erase(removedId);
                rightTriggerHeld.erase(removedId);
            }
            if (event.type == SDL_CONTROLLERAXISMOTION) {
                const int pressThreshold = 16000;
                const int releaseThreshold = 12000;
                SDL_JoystickID instanceId = event.caxis.which;
                if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    if (event.caxis.value > pressThreshold) {
                        if (leftTriggerHeld.insert(instanceId).second) {
                            if (mode == Mode::Browse && !panes[activePane].entries.empty()) {
                                panes[activePane].selected = std::max(0, panes[activePane].selected - 10);
                            }
                        }
                    } else if (event.caxis.value < releaseThreshold) {
                        leftTriggerHeld.erase(instanceId);
                    }
                } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    if (event.caxis.value > pressThreshold) {
                        if (rightTriggerHeld.insert(instanceId).second) {
                            if (mode == Mode::Browse && !panes[activePane].entries.empty()) {
                                panes[activePane].selected = std::min(static_cast<int>(panes[activePane].entries.size()) - 1,
                                                                      panes[activePane].selected + 10);
                            }
                        }
                    } else if (event.caxis.value < releaseThreshold) {
                        rightTriggerHeld.erase(instanceId);
                    }
                }
            }

            if (event.type == SDL_TEXTINPUT &&
                (mode == Mode::Rename || mode == Mode::CreateFolder || mode == Mode::EditSetting || mode == Mode::AddToSteam)) {
                std::string input = event.text.text;
                if (mode == Mode::Rename) {
                    insertFiltered(renameBuffer, renameCursor, input, false);
                } else if (mode == Mode::CreateFolder) {
                    insertFiltered(createFolderName, createFolderCursor, input, false);
                } else if (mode == Mode::AddToSteam) {
                    insertFiltered(addToSteamName, addToSteamCursor, input, false);
                } else {
                    bool digitsOnly = (editField == SettingField::FtpPort);
                    insertFiltered(editBuffer, editCursor, input, digitsOnly);
                }
            }

            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    if (mode == Mode::Browse) {
                        appMenuIndex = 0;
                        mode = Mode::AppMenu;
                    } else if (mode == Mode::EditSetting) {
                        cancelEdit();
                    } else if (mode == Mode::AddToSteam) {
                        cancelAddToSteam();
                    } else if (mode == Mode::CreateFolder) {
                        cancelCreateFolder();
                    } else if (mode == Mode::Rename) {
                        cancelRename();
                    } else if (mode == Mode::Settings) {
                        mode = Mode::AppMenu;
                    } else {
                        mode = Mode::Browse;
                    }
                }

                if (mode == Mode::Browse) {
                    bool hasEntries = !panes[activePane].entries.empty();
                    if (key == SDLK_UP && hasEntries) {
                        panes[activePane].selected = std::max(0, panes[activePane].selected - 1);
                    } else if (key == SDLK_DOWN && hasEntries) {
                        panes[activePane].selected = std::min(static_cast<int>(panes[activePane].entries.size()) - 1, panes[activePane].selected + 1);
                    } else if (key == SDLK_PAGEUP && hasEntries) {
                        panes[activePane].selected = std::max(0, panes[activePane].selected - 10);
                    } else if (key == SDLK_PAGEDOWN && hasEntries) {
                        panes[activePane].selected = std::min(static_cast<int>(panes[activePane].entries.size()) - 1,
                                                              panes[activePane].selected + 10);
                    } else if (key == SDLK_RETURN) {
                        enterSelected(panes[activePane], settings, &status);
                    } else if (key == SDLK_BACKSPACE) {
                        goUp(panes[activePane], settings, &status);
                    } else if (key == SDLK_TAB) {
                        activePane = 1 - activePane;
                    } else if (key == SDLK_x && hasEntries) {
                        Entry entry = panes[activePane].entries[static_cast<size_t>(panes[activePane].selected)];
                        if (!entry.isParent) {
                            action = {entry, activePane};
                            menuIndex = 0;
                            mode = Mode::ActionMenu;
                        }
                    }
                } else if (mode == Mode::ActionMenu) {
                    auto actionOptions = buildActionOptions(action.entry, panes[action.paneIndex]);
                    if (actionOptions.empty()) {
                        mode = Mode::Browse;
                        break;
                    }
                    if (menuIndex < 0 || menuIndex >= static_cast<int>(actionOptions.size())) {
                        menuIndex = 0;
                    }
                    if (key == SDLK_UP) {
                        menuIndex = (menuIndex + static_cast<int>(actionOptions.size()) - 1) % static_cast<int>(actionOptions.size());
                    } else if (key == SDLK_DOWN) {
                        menuIndex = (menuIndex + 1) % static_cast<int>(actionOptions.size());
                    } else if (key == SDLK_RETURN) {
                        handleActionSelection(menuIndex, action, panes, settings, status, mode,
                                             confirmIndex, renameBuffer, renameCursor,
                                             createFolderName, createFolderCursor,
                                             addToSteamName, addToSteamCursor,
                                             osk, &transferCtx);
                    }
                } else if (mode == Mode::ConfirmDelete) {
                    if (key == SDLK_LEFT || key == SDLK_RIGHT) {
                        confirmIndex = 1 - confirmIndex;
                    } else if (key == SDLK_RETURN) {
                        if (confirmIndex == 0) {
                            std::string error;
                            if (deleteFromPane(panes[action.paneIndex], action.entry, settings, error)) {
                                setStatus(status, "Deleted");
                            } else {
                                setStatus(status, "Delete failed: " + error);
                            }
                            loadEntries(panes[action.paneIndex], settings, &status);
                        }
                        mode = Mode::Browse;
                    }
                } else if (mode == Mode::Rename) {
                    if (key == SDLK_LEFT) {
                        if (renameCursor > 0) {
                            --renameCursor;
                        }
                    } else if (key == SDLK_RIGHT) {
                        if (renameCursor < renameBuffer.size()) {
                            ++renameCursor;
                        }
                    } else if (key == SDLK_BACKSPACE) {
                        backspaceAtCursor(renameBuffer, renameCursor);
                    } else if (key == SDLK_RETURN) {
                        commitRename();
                    }
                } else if (mode == Mode::CreateFolder) {
                    if (key == SDLK_LEFT) {
                        if (createFolderCursor > 0) {
                            --createFolderCursor;
                        }
                    } else if (key == SDLK_RIGHT) {
                        if (createFolderCursor < createFolderName.size()) {
                            ++createFolderCursor;
                        }
                    } else if (key == SDLK_BACKSPACE) {
                        backspaceAtCursor(createFolderName, createFolderCursor);
                    } else if (key == SDLK_RETURN) {
                        commitCreateFolder();
                    }
                } else if (mode == Mode::AddToSteam) {
                    if (key == SDLK_LEFT) {
                        if (addToSteamCursor > 0) {
                            --addToSteamCursor;
                        }
                    } else if (key == SDLK_RIGHT) {
                        if (addToSteamCursor < addToSteamName.size()) {
                            ++addToSteamCursor;
                        }
                    } else if (key == SDLK_BACKSPACE) {
                        backspaceAtCursor(addToSteamName, addToSteamCursor);
                    } else if (key == SDLK_RETURN) {
                        commitAddToSteam();
                    }
                } else if (mode == Mode::AppMenu) {
                    if (key == SDLK_UP) {
                        appMenuIndex = (appMenuIndex + static_cast<int>(appMenuOptions.size()) - 1) % static_cast<int>(appMenuOptions.size());
                    } else if (key == SDLK_DOWN) {
                        appMenuIndex = (appMenuIndex + 1) % static_cast<int>(appMenuOptions.size());
                    } else if (key == SDLK_RETURN) {
                        if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Settings") {
                            settingsIndex = 0;
                            mode = Mode::Settings;
                        } else if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Connect to FTP") {
                            connectToFtp(panes[activePane], settings, status);
                            mode = Mode::Browse;
                        } else if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Quit") {
                            quitConfirmIndex = 1;
                            mode = Mode::ConfirmQuit;
                        }
                    }
                } else if (mode == Mode::Settings) {
                    if (key == SDLK_UP) {
                        settingsIndex = (settingsIndex + static_cast<int>(settingsOptions.size()) - 1) % static_cast<int>(settingsOptions.size());
                    } else if (key == SDLK_DOWN) {
                        settingsIndex = (settingsIndex + 1) % static_cast<int>(settingsOptions.size());
                    } else if (key == SDLK_LEFT || key == SDLK_RIGHT) {
                        if (settingsOptions[static_cast<size_t>(settingsIndex)] == "UI Scale") {
                            float delta = (key == SDLK_RIGHT) ? 0.1f : -0.1f;
                            settings.uiScale = clampUiScale(settings.uiScale + delta);
                        } else if (settingsOptions[static_cast<size_t>(settingsIndex)] == "Show Hidden") {
                            settings.showHidden = (key == SDLK_RIGHT);
                            loadEntries(panes[0], settings, &status);
                            loadEntries(panes[1], settings, &status);
                        }
                    } else if (key == SDLK_RETURN) {
                        std::string option = settingsOptions[static_cast<size_t>(settingsIndex)];
                        if (option == "Back") {
                            mode = Mode::AppMenu;
                        } else if (option == "UI Scale") {
                            settings.uiScale = clampUiScale(settings.uiScale + 0.1f);
                        } else if (option == "Show Hidden") {
                            settings.showHidden = !settings.showHidden;
                            loadEntries(panes[0], settings, &status);
                            loadEntries(panes[1], settings, &status);
                        } else {
                        if (option == "FTP Host") {
                            editField = SettingField::FtpHost;
                            editBuffer = settings.ftpHost;
                        } else if (option == "FTP Port") {
                            editField = SettingField::FtpPort;
                            editBuffer = std::to_string(settings.ftpPort);
                        } else if (option == "FTP User") {
                            editField = SettingField::FtpUser;
                            editBuffer = settings.ftpUser;
                        } else if (option == "FTP Password") {
                            editField = SettingField::FtpPass;
                            editBuffer = settings.ftpPass;
                        } else if (option == "Steam Launch Options") {
                            editField = SettingField::SteamLaunchOptions;
                            editBuffer = settings.steamLaunchOptions;
                        } else if (option == "Steam Compatibility Tool") {
                            editField = SettingField::SteamCompatibilityTool;
                            editBuffer = settings.steamCompatibilityToolVersion;
                        }
                        editCursor = editBuffer.size();
                        osk = {};
                        mode = Mode::EditSetting;
                        SDL_SetHint(SDL_HINT_ENABLE_SCREEN_KEYBOARD, "0");
                        SDL_StartTextInput();
                        }
                    }
                } else if (mode == Mode::EditSetting) {
                    if (key == SDLK_LEFT) {
                        if (editCursor > 0) {
                            --editCursor;
                        }
                    } else if (key == SDLK_RIGHT) {
                        if (editCursor < editBuffer.size()) {
                            ++editCursor;
                        }
                    } else if (key == SDLK_BACKSPACE) {
                        backspaceAtCursor(editBuffer, editCursor);
                    } else if (key == SDLK_RETURN) {
                        commitEdit();
                    }
                } else if (mode == Mode::ConfirmQuit) {
                    if (key == SDLK_LEFT || key == SDLK_RIGHT) {
                        quitConfirmIndex = 1 - quitConfirmIndex;
                    } else if (key == SDLK_RETURN) {
                        if (quitConfirmIndex == 0) {
                            running = false;
                        }
                        mode = Mode::Browse;
                    }
                }
            }

            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(event.cbutton.button);
                if (mode == Mode::Browse) {
                    bool hasEntries = !panes[activePane].entries.empty();
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP && hasEntries) {
                        panes[activePane].selected = std::max(0, panes[activePane].selected - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN && hasEntries) {
                        panes[activePane].selected = std::min(static_cast<int>(panes[activePane].entries.size()) - 1, panes[activePane].selected + 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        enterSelected(panes[activePane], settings, &status);
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        goUp(panes[activePane], settings, &status);
                    } else if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                        activePane = 0;
                    } else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                        activePane = 1;
                    } else if (button == SDL_CONTROLLER_BUTTON_X && hasEntries) {
                        Entry entry = panes[activePane].entries[static_cast<size_t>(panes[activePane].selected)];
                        if (!entry.isParent) {
                            action = {entry, activePane};
                            menuIndex = 0;
                            mode = Mode::ActionMenu;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_BACK) {
                        appMenuIndex = 0;
                        mode = Mode::AppMenu;
                    }
                } else if (mode == Mode::ActionMenu) {
                    auto actionOptions = buildActionOptions(action.entry, panes[action.paneIndex]);
                    if (actionOptions.empty()) {
                        mode = Mode::Browse;
                        break;
                    }
                    if (menuIndex < 0 || menuIndex >= static_cast<int>(actionOptions.size())) {
                        menuIndex = 0;
                    }
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                        menuIndex = (menuIndex + static_cast<int>(actionOptions.size()) - 1) % static_cast<int>(actionOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                        menuIndex = (menuIndex + 1) % static_cast<int>(actionOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        mode = Mode::Browse;
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        handleActionSelection(menuIndex, action, panes, settings, status, mode,
                                             confirmIndex, renameBuffer, renameCursor,
                                             createFolderName, createFolderCursor,
                                             addToSteamName, addToSteamCursor,
                                             osk, &transferCtx);
                    }
                } else if (mode == Mode::ConfirmDelete) {
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT || button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                        confirmIndex = 1 - confirmIndex;
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        mode = Mode::Browse;
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (confirmIndex == 0) {
                            std::string error;
                            if (deleteFromPane(panes[action.paneIndex], action.entry, settings, error)) {
                                setStatus(status, "Deleted");
                            } else {
                                setStatus(status, "Delete failed: " + error);
                            }
                            loadEntries(panes[action.paneIndex], settings, &status);
                        }
                        mode = Mode::Browse;
                    }
                } else if (mode == Mode::Rename) {
                    auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                    clampOskSelection(osk, layout);
                    int rows = static_cast<int>(layout.size());
                    if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                        if (renameCursor > 0) {
                            --renameCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                        if (renameCursor < renameBuffer.size()) {
                            ++renameCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP && rows > 0) {
                        osk.row = (osk.row + rows - 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN && rows > 0) {
                        osk.row = (osk.row + 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + cols - 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (!layout.empty() && !layout[osk.row].empty()) {
                            const OskKey& key = layout[osk.row][osk.col];
                            if (key.action == OskAction::None) {
                                insertFiltered(renameBuffer, renameCursor, key.value, false);
                            } else if (key.action == OskAction::Backspace) {
                                backspaceAtCursor(renameBuffer, renameCursor);
                            } else if (key.action == OskAction::Clear) {
                                renameBuffer.clear();
                                renameCursor = 0;
                            } else if (key.action == OskAction::Ok) {
                                commitRename();
                            } else if (key.action == OskAction::Cancel) {
                                cancelRename();
                            } else if (key.action == OskAction::ToggleShift) {
                                osk.uppercase = !osk.uppercase;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            } else if (key.action == OskAction::ToggleSymbols) {
                                osk.symbols = !osk.symbols;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            }
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_X) {
                        backspaceAtCursor(renameBuffer, renameCursor);
                    } else if (button == SDL_CONTROLLER_BUTTON_Y) {
                        renameBuffer.clear();
                        renameCursor = 0;
                    } else if (button == SDL_CONTROLLER_BUTTON_START) {
                        commitRename();
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        cancelRename();
                    }
                } else if (mode == Mode::CreateFolder) {
                    auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                    clampOskSelection(osk, layout);
                    int rows = static_cast<int>(layout.size());
                    if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                        if (createFolderCursor > 0) {
                            --createFolderCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                        if (createFolderCursor < createFolderName.size()) {
                            ++createFolderCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP && rows > 0) {
                        osk.row = (osk.row + rows - 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN && rows > 0) {
                        osk.row = (osk.row + 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + cols - 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (!layout.empty() && !layout[osk.row].empty()) {
                            const OskKey& key = layout[osk.row][osk.col];
                            if (key.action == OskAction::None) {
                                insertFiltered(createFolderName, createFolderCursor, key.value, false);
                            } else if (key.action == OskAction::Backspace) {
                                backspaceAtCursor(createFolderName, createFolderCursor);
                            } else if (key.action == OskAction::Clear) {
                                createFolderName.clear();
                                createFolderCursor = 0;
                            } else if (key.action == OskAction::Ok) {
                                commitCreateFolder();
                            } else if (key.action == OskAction::Cancel) {
                                cancelCreateFolder();
                            } else if (key.action == OskAction::ToggleShift) {
                                osk.uppercase = !osk.uppercase;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            } else if (key.action == OskAction::ToggleSymbols) {
                                osk.symbols = !osk.symbols;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            }
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_X) {
                        backspaceAtCursor(createFolderName, createFolderCursor);
                    } else if (button == SDL_CONTROLLER_BUTTON_Y) {
                        createFolderName.clear();
                        createFolderCursor = 0;
                    } else if (button == SDL_CONTROLLER_BUTTON_START) {
                        commitCreateFolder();
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        cancelCreateFolder();
                    }
                } else if (mode == Mode::AddToSteam) {
                    auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                    clampOskSelection(osk, layout);
                    int rows = static_cast<int>(layout.size());
                    if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                        if (addToSteamCursor > 0) {
                            --addToSteamCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                        if (addToSteamCursor < addToSteamName.size()) {
                            ++addToSteamCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP && rows > 0) {
                        osk.row = (osk.row + rows - 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN && rows > 0) {
                        osk.row = (osk.row + 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + cols - 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (!layout.empty() && !layout[osk.row].empty()) {
                            const OskKey& key = layout[osk.row][osk.col];
                            if (key.action == OskAction::None) {
                                insertFiltered(addToSteamName, addToSteamCursor, key.value, false);
                            } else if (key.action == OskAction::Backspace) {
                                backspaceAtCursor(addToSteamName, addToSteamCursor);
                            } else if (key.action == OskAction::Clear) {
                                addToSteamName.clear();
                                addToSteamCursor = 0;
                            } else if (key.action == OskAction::Ok) {
                                commitAddToSteam();
                            } else if (key.action == OskAction::Cancel) {
                                cancelAddToSteam();
                            } else if (key.action == OskAction::ToggleShift) {
                                osk.uppercase = !osk.uppercase;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            } else if (key.action == OskAction::ToggleSymbols) {
                                osk.symbols = !osk.symbols;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, false);
                                clampOskSelection(osk, updated);
                            }
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_X) {
                        backspaceAtCursor(addToSteamName, addToSteamCursor);
                    } else if (button == SDL_CONTROLLER_BUTTON_Y) {
                        addToSteamName.clear();
                        addToSteamCursor = 0;
                    } else if (button == SDL_CONTROLLER_BUTTON_START) {
                        commitAddToSteam();
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        cancelAddToSteam();
                    }
                } else if (mode == Mode::AppMenu) {
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                        appMenuIndex = (appMenuIndex + static_cast<int>(appMenuOptions.size()) - 1) % static_cast<int>(appMenuOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                        appMenuIndex = (appMenuIndex + 1) % static_cast<int>(appMenuOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        mode = Mode::Browse;
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Settings") {
                            settingsIndex = 0;
                            mode = Mode::Settings;
                        } else if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Connect to FTP") {
                            connectToFtp(panes[activePane], settings, status);
                            mode = Mode::Browse;
                        } else if (appMenuOptions[static_cast<size_t>(appMenuIndex)] == "Quit") {
                            quitConfirmIndex = 1;
                            mode = Mode::ConfirmQuit;
                        }
                    }
                } else if (mode == Mode::Settings) {
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                        settingsIndex = (settingsIndex + static_cast<int>(settingsOptions.size()) - 1) % static_cast<int>(settingsOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                        settingsIndex = (settingsIndex + 1) % static_cast<int>(settingsOptions.size());
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT || button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                        if (settingsOptions[static_cast<size_t>(settingsIndex)] == "UI Scale") {
                            float delta = (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? 0.1f : -0.1f;
                            settings.uiScale = clampUiScale(settings.uiScale + delta);
                        } else if (settingsOptions[static_cast<size_t>(settingsIndex)] == "Show Hidden") {
                            settings.showHidden = (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                            loadEntries(panes[0], settings, &status);
                            loadEntries(panes[1], settings, &status);
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        mode = Mode::AppMenu;
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        std::string option = settingsOptions[static_cast<size_t>(settingsIndex)];
                        if (option == "Back") {
                            mode = Mode::AppMenu;
                        } else if (option == "UI Scale") {
                            settings.uiScale = clampUiScale(settings.uiScale + 0.1f);
                        } else if (option == "Show Hidden") {
                            settings.showHidden = !settings.showHidden;
                            loadEntries(panes[0], settings, &status);
                            loadEntries(panes[1], settings, &status);
                        } else {
                        if (option == "FTP Host") {
                            editField = SettingField::FtpHost;
                            editBuffer = settings.ftpHost;
                        } else if (option == "FTP Port") {
                            editField = SettingField::FtpPort;
                            editBuffer = std::to_string(settings.ftpPort);
                        } else if (option == "FTP User") {
                            editField = SettingField::FtpUser;
                            editBuffer = settings.ftpUser;
                        } else if (option == "FTP Password") {
                            editField = SettingField::FtpPass;
                            editBuffer = settings.ftpPass;
                        } else if (option == "Steam Launch Options") {
                            editField = SettingField::SteamLaunchOptions;
                            editBuffer = settings.steamLaunchOptions;
                        } else if (option == "Steam Compatibility Tool") {
                            editField = SettingField::SteamCompatibilityTool;
                            editBuffer = settings.steamCompatibilityToolVersion;
                        }
                        editCursor = editBuffer.size();
                        osk = {};
                        mode = Mode::EditSetting;
                        SDL_SetHint(SDL_HINT_ENABLE_SCREEN_KEYBOARD, "0");
                        SDL_StartTextInput();
                        }
                    }
                } else if (mode == Mode::EditSetting) {
                    bool numeric = (editField == SettingField::FtpPort);
                    auto layout = buildOskLayout(osk.uppercase, osk.symbols, numeric);
                    clampOskSelection(osk, layout);
                    int rows = static_cast<int>(layout.size());
                    if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                        if (editCursor > 0) {
                            --editCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                        if (editCursor < editBuffer.size()) {
                            ++editCursor;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP && rows > 0) {
                        osk.row = (osk.row + rows - 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN && rows > 0) {
                        osk.row = (osk.row + 1) % rows;
                        osk.col = std::min(osk.col, static_cast<int>(layout[osk.row].size()) - 1);
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + cols - 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT && rows > 0) {
                        int cols = static_cast<int>(layout[osk.row].size());
                        if (cols > 0) {
                            osk.col = (osk.col + 1) % cols;
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (!layout.empty() && !layout[osk.row].empty()) {
                            const OskKey& key = layout[osk.row][osk.col];
                            if (key.action == OskAction::None) {
                                insertFiltered(editBuffer, editCursor, key.value, numeric);
                            } else if (key.action == OskAction::Backspace) {
                                backspaceAtCursor(editBuffer, editCursor);
                            } else if (key.action == OskAction::Clear) {
                                editBuffer.clear();
                                editCursor = 0;
                            } else if (key.action == OskAction::Ok) {
                                commitEdit();
                            } else if (key.action == OskAction::Cancel) {
                                cancelEdit();
                            } else if (key.action == OskAction::ToggleShift) {
                                osk.uppercase = !osk.uppercase;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, numeric);
                                clampOskSelection(osk, updated);
                            } else if (key.action == OskAction::ToggleSymbols) {
                                osk.symbols = !osk.symbols;
                                auto updated = buildOskLayout(osk.uppercase, osk.symbols, numeric);
                                clampOskSelection(osk, updated);
                            }
                        }
                    } else if (button == SDL_CONTROLLER_BUTTON_X) {
                        backspaceAtCursor(editBuffer, editCursor);
                    } else if (button == SDL_CONTROLLER_BUTTON_Y) {
                        editBuffer.clear();
                        editCursor = 0;
                    } else if (button == SDL_CONTROLLER_BUTTON_START) {
                        commitEdit();
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        cancelEdit();
                    }
                } else if (mode == Mode::ConfirmQuit) {
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT || button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                        quitConfirmIndex = 1 - quitConfirmIndex;
                    } else if (button == SDL_CONTROLLER_BUTTON_B) {
                        mode = Mode::Browse;
                    } else if (button == SDL_CONTROLLER_BUTTON_A) {
                        if (quitConfirmIndex == 0) {
                            running = false;
                        }
                        mode = Mode::Browse;
                    }
                }
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window, &width, &height);

        float uiScale = settings.uiScale;
        const int margin = static_cast<int>(std::round(20.0f * uiScale));
        const int gap = static_cast<int>(std::round(20.0f * uiScale));
        const int headerHeight = static_cast<int>(std::round(40.0f * uiScale));
        const int footerHeight = static_cast<int>(std::round(36.0f * uiScale));
        const int paneWidth = std::max(1, (width - margin * 2 - gap) / 2);
        const int paneHeight = std::max(1, height - margin * 2 - footerHeight);
        const int fontScale = std::max(1, static_cast<int>(std::round(2.0f * uiScale)));
        const int rowHeight = std::max(1, 8 * fontScale + static_cast<int>(std::round(6.0f * uiScale)));
        const int smallScale = std::max(1, fontScale - 1);
        const int visibleRows = std::max(1, (paneHeight - headerHeight - 10) / rowHeight);

        for (auto& pane : panes) {
            ensureVisible(pane, visibleRows);
        }

        SDL_SetRenderDrawColor(renderer, 15, 20, 24, 255);
        SDL_RenderClear(renderer);

        SDL_Rect leftPaneRect {margin, margin, paneWidth, paneHeight};
        SDL_Rect rightPaneRect {margin + paneWidth + gap, margin, paneWidth, paneHeight};

        auto drawPane = [&](const Pane& pane, const SDL_Rect& rect, bool isActive) {
            SDL_Color border = isActive ? SDL_Color{80, 220, 140, 255} : SDL_Color{50, 60, 70, 255};
            SDL_Color headerBg = isActive ? SDL_Color{35, 45, 55, 255} : SDL_Color{25, 30, 35, 255};
            SDL_Color textColor {220, 230, 235, 255};

            SDL_SetRenderDrawColor(renderer, 22, 28, 34, 255);
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
            SDL_RenderDrawRect(renderer, &rect);

            SDL_Rect headerRect {rect.x + 1, rect.y + 1, rect.w - 2, headerHeight};
            SDL_SetRenderDrawColor(renderer, headerBg.r, headerBg.g, headerBg.b, headerBg.a);
            SDL_RenderFillRect(renderer, &headerRect);

            int headerX = rect.x + static_cast<int>(std::round(10.0f * uiScale));
            int headerY = rect.y + static_cast<int>(std::round(12.0f * uiScale));
            std::string headerLabel;
            if (pane.source == PaneSource::Ftp) {
                std::string hostLabel = settings.ftpHost.empty() ? "(unset)" : settings.ftpHost;
                headerLabel = "FTP: " + hostLabel + pane.ftpPath;
            } else {
                headerLabel = pane.cwd.string();
            }
            int headerMaxChars = (rect.w - static_cast<int>(std::round(20.0f * uiScale))) / (8 * fontScale + fontScale);
            drawText(renderer, headerX, headerY, fontScale, textColor, ellipsize(headerLabel, headerMaxChars));

            int contentY = rect.y + headerHeight + static_cast<int>(std::round(8.0f * uiScale));
            int contentX = rect.x + static_cast<int>(std::round(12.0f * uiScale));

            int maxChars = (rect.w - static_cast<int>(std::round(24.0f * uiScale))) / (8 * fontScale + fontScale);

            for (int row = 0; row < visibleRows; ++row) {
                int index = pane.scroll + row;
                if (index >= static_cast<int>(pane.entries.size())) {
                    break;
                }
                const Entry& entry = pane.entries[static_cast<size_t>(index)];
                SDL_Rect rowRect {
                    rect.x + static_cast<int>(std::round(6.0f * uiScale)),
                    contentY + row * rowHeight - static_cast<int>(std::round(2.0f * uiScale)),
                    rect.w - static_cast<int>(std::round(12.0f * uiScale)),
                    rowHeight
                };
                if (index == pane.selected && isActive) {
                    SDL_SetRenderDrawColor(renderer, 40, 120, 160, 255);
                    SDL_RenderFillRect(renderer, &rowRect);
                } else if (index == pane.selected) {
                    SDL_SetRenderDrawColor(renderer, 40, 60, 70, 255);
                    SDL_RenderFillRect(renderer, &rowRect);
                }

                std::string label = entry.name;
                if (entry.isDir && !entry.isParent) {
                    label = "[DIR] " + label;
                }

                std::string display = ellipsize(label, maxChars);
                drawText(renderer, contentX, contentY + row * rowHeight, fontScale,
                         textColor, display);
            }
        };

        drawPane(panes[0], leftPaneRect, activePane == 0);
        drawPane(panes[1], rightPaneRect, activePane == 1);

        SDL_Rect footerRect {margin, margin + paneHeight + static_cast<int>(std::round(8.0f * uiScale)), width - margin * 2, footerHeight};
        SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
        SDL_RenderFillRect(renderer, &footerRect);

        SDL_Color footerText {190, 200, 210, 255};
        drawText(renderer, footerRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                 footerRect.y + static_cast<int>(std::round(10.0f * uiScale)),
                 fontScale, footerText,
                 "L1/R1: Active Pane  X: Actions  A: Enter  B: Up  Select: Menu");

        if (mode != Mode::Browse) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
            SDL_Rect overlay {0, 0, width, height};
            SDL_RenderFillRect(renderer, &overlay);

            int modalWidth = static_cast<int>(std::round(440.0f * uiScale));
            int modalHeight = static_cast<int>(std::round(280.0f * uiScale));
            if (mode == Mode::Settings) {
                modalWidth = static_cast<int>(std::round(780.0f * uiScale));
                modalHeight = static_cast<int>(std::round(440.0f * uiScale));
            } else if (mode == Mode::EditSetting || mode == Mode::Rename || mode == Mode::CreateFolder || mode == Mode::AddToSteam) {
                modalWidth = static_cast<int>(std::round(920.0f * uiScale));
                modalHeight = static_cast<int>(std::round(420.0f * uiScale));
            } else if (mode == Mode::AppMenu) {
                modalWidth = static_cast<int>(std::round(360.0f * uiScale));
                modalHeight = static_cast<int>(std::round(240.0f * uiScale));
            }

            SDL_Rect modal {width / 2 - modalWidth / 2, height / 2 - modalHeight / 2, modalWidth, modalHeight};
            SDL_SetRenderDrawColor(renderer, 30, 35, 40, 255);
            SDL_RenderFillRect(renderer, &modal);
            SDL_SetRenderDrawColor(renderer, 80, 220, 140, 255);
            SDL_RenderDrawRect(renderer, &modal);

            SDL_Color modalText {230, 235, 240, 255};
            int padding = static_cast<int>(std::round(20.0f * uiScale));
            int optionHeight = static_cast<int>(std::round(36.0f * uiScale));
            int helpLineHeight = 8 * smallScale + smallScale;

            if (mode == Mode::ActionMenu) {
                auto actionOptions = buildActionOptions(action.entry, panes[action.paneIndex]);
                if (actionOptions.empty()) {
                    actionOptions = {"Back"};
                }
                if (menuIndex < 0 || menuIndex >= static_cast<int>(actionOptions.size())) {
                    menuIndex = 0;
                }
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Actions");
                for (size_t i = 0; i < actionOptions.size(); ++i) {
                    SDL_Rect optionRect {
                        modal.x + padding + static_cast<int>(std::round(10.0f * uiScale)),
                        modal.y + padding + static_cast<int>(std::round(50.0f * uiScale)) + static_cast<int>(i) * optionHeight,
                        modal.w - padding * 2 - static_cast<int>(std::round(20.0f * uiScale)),
                        static_cast<int>(std::round(30.0f * uiScale))
                    };
                    if (static_cast<int>(i) == menuIndex) {
                        SDL_SetRenderDrawColor(renderer, 40, 120, 160, 255);
                        SDL_RenderFillRect(renderer, &optionRect);
                    }
                    drawText(renderer,
                             optionRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                             optionRect.y + static_cast<int>(std::round(6.0f * uiScale)),
                             fontScale, modalText, actionOptions[i]);
                }
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                        smallScale, modalText, "A: Select  B: Back");
            } else if (mode == Mode::ConfirmDelete) {
                drawText(renderer, modal.x + padding, modal.y + padding * 2, fontScale, modalText, "Delete this item?");
                SDL_Rect yesRect {modal.x + padding * 2, modal.y + modal.h / 2, static_cast<int>(std::round(120.0f * uiScale)), static_cast<int>(std::round(40.0f * uiScale))};
                SDL_Rect noRect {modal.x + modal.w - padding * 2 - static_cast<int>(std::round(120.0f * uiScale)),
                                 modal.y + modal.h / 2,
                                 static_cast<int>(std::round(120.0f * uiScale)),
                                 static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, confirmIndex == 0 ? 40 : 25, confirmIndex == 0 ? 120 : 30, confirmIndex == 0 ? 160 : 35, 255);
                SDL_RenderFillRect(renderer, &yesRect);
                SDL_SetRenderDrawColor(renderer, confirmIndex == 1 ? 40 : 25, confirmIndex == 1 ? 120 : 30, confirmIndex == 1 ? 160 : 35, 255);
                SDL_RenderFillRect(renderer, &noRect);
                drawText(renderer,
                         yesRect.x + static_cast<int>(std::round(30.0f * uiScale)),
                         yesRect.y + static_cast<int>(std::round(10.0f * uiScale)),
                         fontScale, modalText, "Yes");
                drawText(renderer,
                         noRect.x + static_cast<int>(std::round(40.0f * uiScale)),
                         noRect.y + static_cast<int>(std::round(10.0f * uiScale)),
                         fontScale, modalText, "No");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText, "A: Confirm  B: Cancel");
            } else if (mode == Mode::Rename) {
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Rename");
                drawText(renderer, modal.x + padding, modal.y + padding + static_cast<int>(std::round(40.0f * uiScale)), smallScale, modalText,
                         "Use OSK/keyboard to type.");

                SDL_Rect fieldRect {modal.x + padding, modal.y + padding + static_cast<int>(std::round(70.0f * uiScale)),
                                    modal.w - padding * 2, static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
                SDL_RenderFillRect(renderer, &fieldRect);
                int fieldMaxChars = (fieldRect.w - static_cast<int>(std::round(20.0f * uiScale))) / (8 * fontScale + fontScale);
                drawInputText(renderer,
                              fieldRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                              fieldRect.y + static_cast<int>(std::round(12.0f * uiScale)),
                              fontScale, modalText, renameBuffer, renameCursor, fieldMaxChars);

                int oskTop = fieldRect.y + fieldRect.h + static_cast<int>(std::round(16.0f * uiScale));
                SDL_Rect oskArea {modal.x + padding, oskTop, modal.w - padding * 2, modal.h - oskTop - padding * 2};
                auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                clampOskSelection(osk, layout);
                drawOsk(renderer, oskArea, fontScale, uiScale, modalText, layout, osk);

                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)) - helpLineHeight,
                         smallScale, modalText,
                         "D-Pad: Move  L1/R1: Cursor  A: Select");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText,
                         "X: Backspace  Y: Clear  Start: Save  B: Cancel");
            } else if (mode == Mode::CreateFolder) {
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Create New Folder");
                drawText(renderer, modal.x + padding, modal.y + padding + static_cast<int>(std::round(40.0f * uiScale)), smallScale, modalText,
                         "Pick a folder name.");

                SDL_Rect fieldRect {modal.x + padding, modal.y + padding + static_cast<int>(std::round(70.0f * uiScale)),
                                    modal.w - padding * 2, static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
                SDL_RenderFillRect(renderer, &fieldRect);
                int fieldMaxChars = (fieldRect.w - static_cast<int>(std::round(20.0f * uiScale))) / (8 * fontScale + fontScale);
                drawInputText(renderer,
                              fieldRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                              fieldRect.y + static_cast<int>(std::round(12.0f * uiScale)),
                              fontScale, modalText, createFolderName, createFolderCursor, fieldMaxChars);

                int oskTop = fieldRect.y + fieldRect.h + static_cast<int>(std::round(16.0f * uiScale));
                SDL_Rect oskArea {modal.x + padding, oskTop, modal.w - padding * 2, modal.h - oskTop - padding * 2};
                auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                clampOskSelection(osk, layout);
                drawOsk(renderer, oskArea, fontScale, uiScale, modalText, layout, osk);

                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)) - helpLineHeight,
                         smallScale, modalText,
                         "D-Pad: Move  L1/R1: Cursor  A: Select");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText,
                         "X: Backspace  Y: Clear  Start: Create  B: Cancel");
            } else if (mode == Mode::AddToSteam) {
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Add to Steam");
                drawText(renderer, modal.x + padding, modal.y + padding + static_cast<int>(std::round(40.0f * uiScale)), smallScale, modalText,
                         "Pick an app name.");

                SDL_Rect fieldRect {modal.x + padding, modal.y + padding + static_cast<int>(std::round(70.0f * uiScale)),
                                    modal.w - padding * 2, static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
                SDL_RenderFillRect(renderer, &fieldRect);
                int fieldMaxChars = (fieldRect.w - static_cast<int>(std::round(20.0f * uiScale))) / (8 * fontScale + fontScale);
                drawInputText(renderer,
                              fieldRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                              fieldRect.y + static_cast<int>(std::round(12.0f * uiScale)),
                              fontScale, modalText, addToSteamName, addToSteamCursor, fieldMaxChars);

                int oskTop = fieldRect.y + fieldRect.h + static_cast<int>(std::round(16.0f * uiScale));
                SDL_Rect oskArea {modal.x + padding, oskTop, modal.w - padding * 2, modal.h - oskTop - padding * 2};
                auto layout = buildOskLayout(osk.uppercase, osk.symbols, false);
                clampOskSelection(osk, layout);
                drawOsk(renderer, oskArea, fontScale, uiScale, modalText, layout, osk);

                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)) - helpLineHeight,
                         smallScale, modalText,
                         "D-Pad: Move  L1/R1: Cursor  A: Select");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText,
                         "X: Backspace  Y: Clear  Start: Add  B: Cancel");
            } else if (mode == Mode::AppMenu) {
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Menu");
                for (size_t i = 0; i < appMenuOptions.size(); ++i) {
                    SDL_Rect optionRect {
                        modal.x + padding,
                        modal.y + padding + static_cast<int>(std::round(50.0f * uiScale)) + static_cast<int>(i) * optionHeight,
                        modal.w - padding * 2,
                        static_cast<int>(std::round(30.0f * uiScale))
                    };
                    if (static_cast<int>(i) == appMenuIndex) {
                        SDL_SetRenderDrawColor(renderer, 40, 120, 160, 255);
                        SDL_RenderFillRect(renderer, &optionRect);
                    }
                    drawText(renderer,
                             optionRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                             optionRect.y + static_cast<int>(std::round(6.0f * uiScale)),
                             fontScale, modalText, appMenuOptions[i]);
                }
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText, "A: Select  B: Back");
            } else if (mode == Mode::Settings) {
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, "Settings");
                int maxChars = (modal.w - padding * 2) / (8 * fontScale + fontScale);
                for (size_t i = 0; i < settingsOptions.size(); ++i) {
                    SDL_Rect optionRect {
                        modal.x + padding,
                        modal.y + padding + static_cast<int>(std::round(50.0f * uiScale)) + static_cast<int>(i) * optionHeight,
                        modal.w - padding * 2,
                        static_cast<int>(std::round(30.0f * uiScale))
                    };
                    if (static_cast<int>(i) == settingsIndex) {
                        SDL_SetRenderDrawColor(renderer, 40, 120, 160, 255);
                        SDL_RenderFillRect(renderer, &optionRect);
                    }
                    std::string label = settingsOptions[i];
                    if (settingsOptions[i] == "FTP Host") {
                        label += ": " + (settings.ftpHost.empty() ? "(unset)" : settings.ftpHost);
                    } else if (settingsOptions[i] == "FTP Port") {
                        label += ": " + std::to_string(settings.ftpPort);
                    } else if (settingsOptions[i] == "FTP User") {
                        label += ": " + (settings.ftpUser.empty() ? "(unset)" : settings.ftpUser);
                    } else if (settingsOptions[i] == "FTP Password") {
                        label += ": " + (settings.ftpPass.empty() ? "(unset)" : maskPassword(settings.ftpPass));
                    } else if (settingsOptions[i] == "Steam Launch Options") {
                        label += ": " + (settings.steamLaunchOptions.empty() ? "(unset)" : settings.steamLaunchOptions);
                    } else if (settingsOptions[i] == "Steam Compatibility Tool") {
                        label += ": " + (settings.steamCompatibilityToolVersion.empty() ? "(unset)" : settings.steamCompatibilityToolVersion);
                    } else if (settingsOptions[i] == "UI Scale") {
                        label += ": " + formatScale(settings.uiScale);
                    } else if (settingsOptions[i] == "Show Hidden") {
                        label += ": " + std::string(settings.showHidden ? "Yes" : "No");
                    }
                    drawText(renderer,
                             optionRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                             optionRect.y + static_cast<int>(std::round(6.0f * uiScale)),
                             fontScale, modalText, ellipsize(label, maxChars));
                }
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText, "A: Edit  B: Back  Left/Right: Scale/Hidden");
            } else if (mode == Mode::EditSetting) {
                std::string editTitle = "Edit ";
                if (editField == SettingField::FtpHost) {
                    editTitle += "FTP Host";
                } else if (editField == SettingField::FtpPort) {
                    editTitle += "FTP Port";
                } else if (editField == SettingField::FtpUser) {
                    editTitle += "FTP User";
                } else if (editField == SettingField::FtpPass) {
                    editTitle += "FTP Password";
                } else if (editField == SettingField::SteamLaunchOptions) {
                    editTitle += "Steam Launch Options";
                } else if (editField == SettingField::SteamCompatibilityTool) {
                    editTitle += "Steam Compatibility Tool";
                }
                drawText(renderer, modal.x + padding, modal.y + padding, fontScale, modalText, editTitle);
                drawText(renderer, modal.x + padding, modal.y + padding + static_cast<int>(std::round(40.0f * uiScale)),
                         smallScale, modalText, "Use OSK/keyboard to type.");

                SDL_Rect fieldRect {modal.x + padding, modal.y + padding + static_cast<int>(std::round(70.0f * uiScale)),
                                    modal.w - padding * 2, static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
                SDL_RenderFillRect(renderer, &fieldRect);
                int fieldMaxChars = (fieldRect.w - static_cast<int>(std::round(20.0f * uiScale))) / (8 * fontScale + fontScale);
                drawInputText(renderer,
                              fieldRect.x + static_cast<int>(std::round(10.0f * uiScale)),
                              fieldRect.y + static_cast<int>(std::round(12.0f * uiScale)),
                              fontScale, modalText, editBuffer, editCursor, fieldMaxChars);

                int oskTop = fieldRect.y + fieldRect.h + static_cast<int>(std::round(16.0f * uiScale));
                SDL_Rect oskArea {modal.x + padding, oskTop, modal.w - padding * 2, modal.h - oskTop - padding * 2};
                bool numeric = (editField == SettingField::FtpPort);
                auto layout = buildOskLayout(osk.uppercase, osk.symbols, numeric);
                clampOskSelection(osk, layout);
                drawOsk(renderer, oskArea, fontScale, uiScale, modalText, layout, osk);

                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)) - helpLineHeight,
                         smallScale, modalText,
                         "D-Pad: Move  L1/R1: Cursor  A: Select");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText,
                         "X: Backspace  Y: Clear  Start: Save  B: Cancel");
            } else if (mode == Mode::ConfirmQuit) {
                drawText(renderer, modal.x + padding, modal.y + padding * 2, fontScale, modalText, "Quit GamepadCommander?");
                SDL_Rect yesRect {modal.x + padding * 2, modal.y + modal.h / 2, static_cast<int>(std::round(120.0f * uiScale)), static_cast<int>(std::round(40.0f * uiScale))};
                SDL_Rect noRect {modal.x + modal.w - padding * 2 - static_cast<int>(std::round(120.0f * uiScale)),
                                 modal.y + modal.h / 2,
                                 static_cast<int>(std::round(120.0f * uiScale)),
                                 static_cast<int>(std::round(40.0f * uiScale))};
                SDL_SetRenderDrawColor(renderer, quitConfirmIndex == 0 ? 40 : 25, quitConfirmIndex == 0 ? 120 : 30, quitConfirmIndex == 0 ? 160 : 35, 255);
                SDL_RenderFillRect(renderer, &yesRect);
                SDL_SetRenderDrawColor(renderer, quitConfirmIndex == 1 ? 40 : 25, quitConfirmIndex == 1 ? 120 : 30, quitConfirmIndex == 1 ? 160 : 35, 255);
                SDL_RenderFillRect(renderer, &noRect);
                drawText(renderer,
                         yesRect.x + static_cast<int>(std::round(30.0f * uiScale)),
                         yesRect.y + static_cast<int>(std::round(10.0f * uiScale)),
                         fontScale, modalText, "Yes");
                drawText(renderer,
                         noRect.x + static_cast<int>(std::round(40.0f * uiScale)),
                         noRect.y + static_cast<int>(std::round(10.0f * uiScale)),
                         fontScale, modalText, "No");
                drawText(renderer,
                         modal.x + padding,
                         modal.y + modal.h - padding - static_cast<int>(std::round(10.0f * uiScale)),
                         smallScale, modalText, "A: Confirm  B: Cancel");
            }
        }

        if (statusActive(status)) {
            int notificationScale = fontScale + 1;
            int paddingX = std::max(4, static_cast<int>(std::round(8.0f * uiScale)));
            int paddingY = std::max(4, static_cast<int>(std::round(6.0f * uiScale)));
            int advance = 8 * notificationScale + notificationScale;
            int maxWidth = std::max(1, width - margin * 2 - paddingX * 2);
            int maxChars = std::max(1, maxWidth / advance);
            std::string display = ellipsize(status.text, maxChars);
            int textW = textWidth(notificationScale, display);
            int textH = 8 * notificationScale;
            SDL_Rect badge {
                std::max(margin, width - margin - textW - paddingX * 2),
                margin,
                textW + paddingX * 2,
                textH + paddingY * 2
            };
            SDL_SetRenderDrawColor(renderer, 20, 30, 35, 255);
            SDL_RenderFillRect(renderer, &badge);
            drawText(renderer,
                     badge.x + paddingX,
                     badge.y + paddingY,
                     notificationScale,
                     SDL_Color{240, 200, 120, 255},
                     display);
        }

        SDL_RenderPresent(renderer);
    }

    if (!configPath.empty()) {
        for (int i = 0; i < 2; ++i) {
            if (panes[i].source == PaneSource::Local) {
                panes[i].lastLocalCwd = panes[i].cwd;
            }
        }
        saveConfig(settings, panes, configPath);
    }

#ifdef USE_CURL
    curl_global_cleanup();
#endif

    for (auto& entry : controllers) {
        SDL_GameControllerClose(entry.second);
    }
    controllers.clear();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
