#include "SteamHelper.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

namespace {
constexpr uint8_t kNul = 0;
constexpr uint8_t kSoh = 1;
constexpr uint8_t kStx = 2;
constexpr uint8_t kBs = 8;

fs::path getHomePath() {
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

bool isDigits(const std::string& text) {
    return !text.empty() &&
           std::all_of(text.begin(), text.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; });
}

void appendByte(std::vector<uint8_t>& out, uint8_t value) {
    out.push_back(value);
}

void appendString(std::vector<uint8_t>& out, const std::string& value) {
    out.insert(out.end(), value.begin(), value.end());
}

void appendUInt32LE(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void appendNumberField(std::vector<uint8_t>& out, const std::string& key, uint32_t value) {
    appendByte(out, kStx);
    appendString(out, key);
    appendByte(out, kNul);
    appendUInt32LE(out, value);
}

void appendStringField(std::vector<uint8_t>& out, const std::string& key, const std::string& value) {
    appendByte(out, kSoh);
    appendString(out, key);
    appendByte(out, kNul);
    appendString(out, value);
    appendByte(out, kNul);
}

void appendStringArrayField(std::vector<uint8_t>& out, const std::string& key, const std::vector<std::string>& values) {
    appendByte(out, kNul);
    appendString(out, key);
    appendByte(out, kNul);
    for (size_t i = 0; i < values.size(); ++i) {
        appendByte(out, kSoh);
        appendString(out, std::to_string(i));
        appendByte(out, kNul);
        appendString(out, values[i]);
        appendByte(out, kNul);
    }
}

std::vector<uint8_t> buildEntryBuffer(uint32_t appId,
                                      const std::string& appName,
                                      const std::string& exe,
                                      const std::string& startDir,
                                      const std::string& launchOptions) {
    std::vector<uint8_t> out;
    appendNumberField(out, "appid", appId);
    appendStringField(out, "AppName", appName);
    appendStringField(out, "Exe", exe);
    appendStringField(out, "StartDir", startDir);
    appendStringField(out, "icon", "");
    appendStringField(out, "ShortcutPath", "");
    appendStringField(out, "LaunchOptions", launchOptions);
    appendNumberField(out, "IsHidden", 0);
    appendNumberField(out, "AllowDesktopConfig", 0);
    appendNumberField(out, "AllowOverlay", 0);
    appendNumberField(out, "OpenVR", 0);
    appendNumberField(out, "Devkit", 0);
    appendStringField(out, "DevkitGameID", "");
    appendNumberField(out, "DevkitOverrideAppID", 0);
    appendNumberField(out, "LastPlayTime", 0);
    appendStringField(out, "FlatpakAppID", "");
    appendStringArrayField(out, "tags", {});
    return out;
}

std::optional<uint32_t> extractAppId(const std::vector<uint8_t>& entry) {
    const std::array<uint8_t, 7> pattern = {kStx, 'a', 'p', 'p', 'i', 'd', kNul};
    auto it = std::search(entry.begin(), entry.end(), pattern.begin(), pattern.end());
    if (it == entry.end()) {
        return std::nullopt;
    }
    size_t offset = static_cast<size_t>(it - entry.begin()) + pattern.size();
    if (offset + 4 > entry.size()) {
        return std::nullopt;
    }
    uint32_t value = static_cast<uint32_t>(entry[offset]) |
                     (static_cast<uint32_t>(entry[offset + 1]) << 8) |
                     (static_cast<uint32_t>(entry[offset + 2]) << 16) |
                     (static_cast<uint32_t>(entry[offset + 3]) << 24);
    return value;
}

size_t findSequence(const std::vector<uint8_t>& data, size_t start, uint8_t first, uint8_t second) {
    for (size_t i = start; i + 1 < data.size(); ++i) {
        if (data[i] == first && data[i + 1] == second) {
            return i;
        }
    }
    return std::string::npos;
}

bool parseShortcuts(const std::vector<uint8_t>& data,
                    std::vector<std::vector<uint8_t>>& entries,
                    std::string& error) {
    entries.clear();
    if (data.empty()) {
        return true;
    }
    const std::string header = "shortcuts";
    if (data.size() < header.size() + 2 || data[0] != kNul) {
        error = "Invalid shortcuts.vdf header";
        return false;
    }
    if (std::string(data.begin() + 1, data.begin() + 1 + header.size()) != header ||
        data[1 + header.size()] != kNul) {
        error = "Invalid shortcuts.vdf header";
        return false;
    }

    size_t pos = 1 + header.size() + 1;
    while (pos < data.size()) {
        if (data[pos] == kBs && pos + 1 < data.size() && data[pos + 1] == kBs) {
            return true;
        }
        if (data[pos] != kNul) {
            error = "Malformed shortcuts.vdf entry";
            return false;
        }
        size_t indexStart = pos + 1;
        size_t indexEnd = indexStart;
        while (indexEnd < data.size() && data[indexEnd] != kNul) {
            ++indexEnd;
        }
        if (indexEnd >= data.size()) {
            error = "Malformed shortcuts.vdf entry";
            return false;
        }
        pos = indexEnd + 1;
        size_t entryEnd = findSequence(data, pos, kBs, kBs);
        if (entryEnd == std::string::npos) {
            error = "Malformed shortcuts.vdf entry";
            return false;
        }
        entries.emplace_back(data.begin() + static_cast<std::vector<uint8_t>::difference_type>(pos),
                             data.begin() + static_cast<std::vector<uint8_t>::difference_type>(entryEnd));
        pos = entryEnd + 2;
    }
    return true;
}

std::vector<uint8_t> buildShortcutsBuffer(const std::vector<std::vector<uint8_t>>& entries) {
    std::vector<uint8_t> out;
    appendByte(out, kNul);
    appendString(out, "shortcuts");
    appendByte(out, kNul);
    for (size_t i = 0; i < entries.size(); ++i) {
        appendByte(out, kNul);
        appendString(out, std::to_string(i));
        appendByte(out, kNul);
        out.insert(out.end(), entries[i].begin(), entries[i].end());
        appendByte(out, kBs);
        appendByte(out, kBs);
    }
    appendByte(out, kBs);
    appendByte(out, kBs);
    return out;
}

bool readFile(const fs::path& path, std::vector<uint8_t>& data, std::string& error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error = "Failed to open shortcuts.vdf";
        return false;
    }
    file.seekg(0, std::ios::end);
    std::streamoff size = file.tellg();
    if (size < 0) {
        error = "Failed to read shortcuts.vdf";
        return false;
    }
    data.resize(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (size > 0) {
        file.read(reinterpret_cast<char*>(data.data()), size);
        if (!file) {
            error = "Failed to read shortcuts.vdf";
            return false;
        }
    }
    return true;
}

bool writeFile(const fs::path& path, const std::vector<uint8_t>& data, std::string& error) {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    if (ec) {
        error = "Failed to create Steam config directory";
        return false;
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "Failed to write shortcuts.vdf";
        return false;
    }
    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!file) {
            error = "Failed to write shortcuts.vdf";
            return false;
        }
    }
    return true;
}

std::vector<fs::path> steamSearchRoots() {
    std::vector<fs::path> roots;
#ifdef _WIN32
    const char* programFilesX86 = std::getenv("PROGRAMFILES(X86)");
    if (programFilesX86 && programFilesX86[0] != '\0') {
        roots.emplace_back(fs::path(programFilesX86) / "Steam");
    }
    const char* programFiles = std::getenv("PROGRAMFILES");
    if (programFiles && programFiles[0] != '\0') {
        roots.emplace_back(fs::path(programFiles) / "Steam");
    }
#elif defined(__APPLE__)
    fs::path home = getHomePath();
    roots.emplace_back(home / "Library/Application Support/Steam");
#else
    fs::path home = getHomePath();
    roots.emplace_back(home / ".steam/steam");
    roots.emplace_back(home / ".local/share/Steam");
    roots.emplace_back(home / "snap/steam");
    roots.emplace_back(home / ".var/app/com.valvesoftware.Steam/.steam/steam");
#endif
    return roots;
}

fs::path findShortcutsPath(std::string& error) {
    fs::path fallback;
    for (const auto& root : steamSearchRoots()) {
        fs::path userdata = root / "userdata";
        if (!fs::exists(userdata) || !fs::is_directory(userdata)) {
            continue;
        }
        std::vector<fs::path> users;
        for (const auto& entry : fs::directory_iterator(userdata)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::string name = entry.path().filename().string();
            if (isDigits(name)) {
                users.push_back(entry.path());
            }
        }
        std::sort(users.begin(), users.end());
        for (const auto& userDir : users) {
            fs::path candidate = userDir / "config" / "shortcuts.vdf";
            if (fs::exists(candidate)) {
                return candidate;
            }
            if (fallback.empty()) {
                fallback = candidate;
            }
        }
    }
    if (!fallback.empty()) {
        return fallback;
    }
    error = "Steam shortcuts.vdf not found";
    return {};
}

std::optional<uint32_t> generateUniqueAppId(const std::unordered_set<uint32_t>& existing) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<uint32_t> dist(1000000000u, 0xFFFFFFFFu);
    for (int i = 0; i < 100; ++i) {
        uint32_t candidate = dist(rng);
        if (existing.find(candidate) == existing.end()) {
            return candidate;
        }
    }
    return std::nullopt;
}
} // namespace

bool addShortcutToSteam(const fs::path& exePath,
                        const fs::path& startDir,
                        const std::string& appName,
                        const std::string& launchOptions,
                        std::string& error) {
    if (appName.empty()) {
        error = "App name is required";
        return false;
    }

    fs::path shortcutsPath = findShortcutsPath(error);
    if (shortcutsPath.empty()) {
        return false;
    }

    std::vector<std::vector<uint8_t>> entries;
    if (fs::exists(shortcutsPath)) {
        std::vector<uint8_t> data;
        if (!readFile(shortcutsPath, data, error)) {
            return false;
        }
        if (!parseShortcuts(data, entries, error)) {
            return false;
        }
    }

    std::unordered_set<uint32_t> existingIds;
    for (const auto& entry : entries) {
        if (auto appId = extractAppId(entry)) {
            existingIds.insert(*appId);
        }
    }

    auto newAppId = generateUniqueAppId(existingIds);
    if (!newAppId) {
        error = "Failed to generate unique app ID";
        return false;
    }

    fs::path absExe = fs::absolute(exePath);
    fs::path absStart = startDir.empty() ? absExe.parent_path() : fs::absolute(startDir);
    std::string exeValue = absExe.string();
    std::string dirValue = absStart.string();
    entries.push_back(buildEntryBuffer(*newAppId, appName, exeValue, dirValue, launchOptions));

    std::vector<uint8_t> output = buildShortcutsBuffer(entries);
    if (!writeFile(shortcutsPath, output, error)) {
        return false;
    }

    return true;
}
