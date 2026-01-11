#include "SteamHelper.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
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

struct VdfToken {
    enum class Type { String, LBrace, RBrace };
    Type type;
    std::string value;
    size_t start = 0;
    size_t end = 0;
};

struct VdfObject;

struct VdfEntry {
    std::string key;
    size_t keyStart = 0;
    size_t keyEnd = 0;
    size_t valueStart = 0;
    size_t valueEnd = 0;
    std::string stringValue;
    std::unique_ptr<VdfObject> objectValue;
};

struct VdfObject {
    size_t braceStart = std::string::npos;
    size_t braceEnd = std::string::npos;
    std::vector<VdfEntry> entries;
};

bool isWhitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

std::string escapeVdfString(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char ch : value) {
        if (ch == '\\' || ch == '"') {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    return out;
}

std::string getLineIndent(const std::string& text, size_t pos) {
    if (text.empty()) {
        return {};
    }
    size_t lineStart = text.rfind('\n', pos);
    if (lineStart == std::string::npos) {
        lineStart = 0;
    } else {
        lineStart += 1;
    }
    if (lineStart < text.size() && text[lineStart] == '\r') {
        ++lineStart;
    }
    size_t i = lineStart;
    while (i < text.size() && (text[i] == ' ' || text[i] == '\t')) {
        ++i;
    }
    return text.substr(lineStart, i - lineStart);
}

std::string detectIndentUnit(const std::string& baseIndent) {
    if (baseIndent.find('\t') != std::string::npos) {
        return "\t";
    }
    return "    ";
}

bool tokenizeVdf(const std::string& text, std::vector<VdfToken>& tokens, std::string& error) {
    tokens.clear();
    size_t i = 0;
    while (i < text.size()) {
        char ch = text[i];
        if (isWhitespace(ch)) {
            ++i;
            continue;
        }
        if (ch == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            i += 2;
            while (i < text.size() && text[i] != '\n') {
                ++i;
            }
            continue;
        }
        if (ch == '{') {
            tokens.push_back({VdfToken::Type::LBrace, {}, i, i + 1});
            ++i;
            continue;
        }
        if (ch == '}') {
            tokens.push_back({VdfToken::Type::RBrace, {}, i, i + 1});
            ++i;
            continue;
        }
        if (ch == '"') {
            size_t start = i;
            ++i;
            std::string value;
            while (i < text.size()) {
                char current = text[i];
                if (current == '\\' && i + 1 < text.size()) {
                    value.push_back(text[i + 1]);
                    i += 2;
                    continue;
                }
                if (current == '"') {
                    ++i;
                    tokens.push_back({VdfToken::Type::String, value, start, i});
                    break;
                }
                value.push_back(current);
                ++i;
            }
            if (tokens.empty() || tokens.back().start != start) {
                error = "Unterminated string in config.vdf";
                return false;
            }
            continue;
        }
        error = "Unexpected token in config.vdf";
        return false;
    }
    return true;
}

bool parseObjectTokens(const std::vector<VdfToken>& tokens, size_t& index, VdfObject& out, std::string& error) {
    while (index < tokens.size()) {
        const auto& token = tokens[index];
        if (token.type == VdfToken::Type::RBrace) {
            out.braceEnd = token.start;
            ++index;
            return true;
        }
        if (token.type != VdfToken::Type::String) {
            error = "Malformed config.vdf";
            return false;
        }
        VdfEntry entry;
        entry.key = token.value;
        entry.keyStart = token.start;
        entry.keyEnd = token.end;
        ++index;
        if (index >= tokens.size()) {
            error = "Malformed config.vdf";
            return false;
        }
        if (tokens[index].type == VdfToken::Type::String) {
            entry.stringValue = tokens[index].value;
            entry.valueStart = tokens[index].start;
            entry.valueEnd = tokens[index].end;
            ++index;
        } else if (tokens[index].type == VdfToken::Type::LBrace) {
            entry.valueStart = tokens[index].start;
            auto child = std::make_unique<VdfObject>();
            child->braceStart = tokens[index].start;
            ++index;
            if (!parseObjectTokens(tokens, index, *child, error)) {
                return false;
            }
            entry.valueEnd = child->braceEnd;
            entry.objectValue = std::move(child);
        } else {
            error = "Malformed config.vdf";
            return false;
        }
        out.entries.push_back(std::move(entry));
    }
    return true;
}

bool parseRootTokens(const std::vector<VdfToken>& tokens, VdfObject& out, std::string& error) {
    size_t index = 0;
    while (index < tokens.size()) {
        const auto& token = tokens[index];
        if (token.type != VdfToken::Type::String) {
            error = "Malformed config.vdf";
            return false;
        }
        VdfEntry entry;
        entry.key = token.value;
        entry.keyStart = token.start;
        entry.keyEnd = token.end;
        ++index;
        if (index >= tokens.size()) {
            error = "Malformed config.vdf";
            return false;
        }
        if (tokens[index].type == VdfToken::Type::String) {
            entry.stringValue = tokens[index].value;
            entry.valueStart = tokens[index].start;
            entry.valueEnd = tokens[index].end;
            ++index;
        } else if (tokens[index].type == VdfToken::Type::LBrace) {
            entry.valueStart = tokens[index].start;
            auto child = std::make_unique<VdfObject>();
            child->braceStart = tokens[index].start;
            ++index;
            if (!parseObjectTokens(tokens, index, *child, error)) {
                return false;
            }
            entry.valueEnd = child->braceEnd;
            entry.objectValue = std::move(child);
        } else {
            error = "Malformed config.vdf";
            return false;
        }
        out.entries.push_back(std::move(entry));
    }
    return true;
}

VdfEntry* findEntry(VdfObject& obj, const std::string& key, bool caseSensitive = true) {
    for (auto& entry : obj.entries) {
        if (caseSensitive) {
            if (entry.key == key) {
                return &entry;
            }
        } else {
            if (entry.key.size() == key.size() &&
                std::equal(entry.key.begin(), entry.key.end(), key.begin(),
                           [](char a, char b) {
                               return std::tolower(static_cast<unsigned char>(a)) ==
                                      std::tolower(static_cast<unsigned char>(b));
                           })) {
                return &entry;
            }
        }
    }
    return nullptr;
}

std::string detectNewline(const std::string& text) {
    if (text.find("\r\n") != std::string::npos) {
        return "\r\n";
    }
    return "\n";
}

std::vector<fs::path> steamConfigCandidates() {
    std::vector<fs::path> candidates;
#ifdef _WIN32
    return candidates;
#else
    fs::path home = getHomePath();
    std::vector<fs::path> roots = {
        home / ".local/share/Steam",
        home / ".steam/root",
        home / ".steam/steam",
        home / ".steam/debian-installation"
    };
    std::vector<fs::path> uniqueRoots;
    for (const auto& root : roots) {
        std::error_code ec;
        fs::path real = fs::weakly_canonical(root, ec);
        fs::path resolved = ec ? root : real;
        if (std::find(uniqueRoots.begin(), uniqueRoots.end(), resolved) == uniqueRoots.end()) {
            uniqueRoots.push_back(resolved);
        }
    }
    for (const auto& root : uniqueRoots) {
        candidates.push_back(root / "config" / "config.vdf");
    }
    candidates.push_back(home / ".var/app/com.valvesoftware.Steam/.local/share/Steam/config/config.vdf");
    candidates.push_back(home / "snap/steam/common/.steam/root/config/config.vdf");
    candidates.push_back(home / "steam/.steam/config/config.vdf");
    return candidates;
#endif
}

fs::path findConfigVdfPath(std::string& error) {
    for (const auto& candidate : steamConfigCandidates()) {
        if (!candidate.empty() && fs::exists(candidate)) {
            return candidate;
        }
    }
    error = "Steam config.vdf not found";
    return {};
}

bool replaceNameValue(std::string& text, const VdfEntry& nameEntry, const std::string& toolName) {
    if (nameEntry.valueEnd <= nameEntry.valueStart || nameEntry.valueEnd > text.size()) {
        return false;
    }
    std::string replacement = "\"" + escapeVdfString(toolName) + "\"";
    text.replace(nameEntry.valueStart, nameEntry.valueEnd - nameEntry.valueStart, replacement);
    return true;
}

std::string buildCompatEntryBlock(const std::string& appId,
                                  const std::string& toolName,
                                  const std::string& entryIndent,
                                  const std::string& fieldIndent,
                                  const std::string& newline) {
    std::string block;
    block += entryIndent + "\"" + appId + "\"" + newline;
    block += entryIndent + "{" + newline;
    block += fieldIndent + "\"name\" \"" + escapeVdfString(toolName) + "\"" + newline;
    block += fieldIndent + "\"config\" \"\"" + newline;
    block += fieldIndent + "\"priority\" \"250\"" + newline;
    block += entryIndent + "}" + newline;
    return block;
}

std::string buildCompatMappingBlock(const std::string& appId,
                                    const std::string& toolName,
                                    const std::string& steamEntryIndent,
                                    const std::string& entryIndent,
                                    const std::string& fieldIndent,
                                    const std::string& newline) {
    std::string block;
    block += steamEntryIndent + "\"CompatToolMapping\"" + newline;
    block += steamEntryIndent + "{" + newline;
    block += buildCompatEntryBlock(appId, toolName, entryIndent, fieldIndent, newline);
    block += steamEntryIndent + "}" + newline;
    return block;
}

bool insertBefore(std::string& text, size_t pos, const std::string& insertion) {
    if (pos > text.size()) {
        return false;
    }
    text.insert(pos, insertion);
    return true;
}
} // namespace

bool addShortcutToSteam(const fs::path& exePath,
                        const fs::path& startDir,
                        const std::string& appName,
                        const std::string& launchOptions,
                        std::uint32_t& appId,
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
    appId = *newAppId;

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

bool setSteamCompatToolMapping(std::uint32_t appId, const std::string& toolName, std::string& error) {
    if (toolName.empty()) {
        return true;
    }
    fs::path configPath = findConfigVdfPath(error);
    if (configPath.empty()) {
        return false;
    }

    std::ifstream file(configPath);
    if (!file) {
        error = "Failed to open config.vdf";
        return false;
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::vector<VdfToken> tokens;
    if (!tokenizeVdf(text, tokens, error)) {
        return false;
    }
    VdfObject root;
    if (!parseRootTokens(tokens, root, error)) {
        return false;
    }

    VdfEntry* install = findEntry(root, "InstallConfigStore");
    if (!install || !install->objectValue) {
        error = "InstallConfigStore not found";
        return false;
    }
    VdfEntry* software = findEntry(*install->objectValue, "Software");
    if (!software || !software->objectValue) {
        error = "Software section not found";
        return false;
    }
    VdfEntry* valve = findEntry(*software->objectValue, "Valve");
    if (!valve || !valve->objectValue) {
        valve = findEntry(*software->objectValue, "valve");
    }
    if (!valve || !valve->objectValue) {
        error = "Valve section not found";
        return false;
    }
    VdfEntry* steam = findEntry(*valve->objectValue, "Steam");
    if (!steam || !steam->objectValue) {
        error = "Steam section not found";
        return false;
    }

    std::string newline = detectNewline(text);
    std::string appIdText = std::to_string(appId);

    VdfEntry* compat = findEntry(*steam->objectValue, "CompatToolMapping");
    if (compat && compat->objectValue) {
        VdfEntry* mappingEntry = findEntry(*compat->objectValue, appIdText);
        if (mappingEntry && mappingEntry->objectValue) {
            VdfEntry* nameEntry = findEntry(*mappingEntry->objectValue, "name");
            if (nameEntry && nameEntry->valueEnd > nameEntry->valueStart) {
                if (!replaceNameValue(text, *nameEntry, toolName)) {
                    error = "Failed to update compatibility tool";
                    return false;
                }
            } else {
                std::string entryIndent = getLineIndent(text, mappingEntry->keyStart);
                std::string fieldIndent;
                if (!mappingEntry->objectValue->entries.empty()) {
                    fieldIndent = getLineIndent(text, mappingEntry->objectValue->entries.front().keyStart);
                } else {
                    fieldIndent = entryIndent + detectIndentUnit(entryIndent);
                }
                std::string insertion;
                if (mappingEntry->objectValue->braceEnd > 0 &&
                    text[mappingEntry->objectValue->braceEnd - 1] != '\n' &&
                    text[mappingEntry->objectValue->braceEnd - 1] != '\r') {
                    insertion += newline;
                }
                insertion += fieldIndent + "\"name\" \"" + escapeVdfString(toolName) + "\"" + newline;
                if (!insertBefore(text, mappingEntry->objectValue->braceEnd, insertion)) {
                    error = "Failed to update compatibility tool";
                    return false;
                }
            }
        } else {
            std::string mappingIndent = getLineIndent(text, compat->keyStart);
            std::string entryIndent;
            std::string fieldIndent;
            if (!compat->objectValue->entries.empty()) {
                entryIndent = getLineIndent(text, compat->objectValue->entries.front().keyStart);
                if (!compat->objectValue->entries.front().objectValue ||
                    compat->objectValue->entries.front().objectValue->entries.empty()) {
                    fieldIndent = entryIndent + detectIndentUnit(entryIndent);
                } else {
                    fieldIndent = getLineIndent(text, compat->objectValue->entries.front().objectValue->entries.front().keyStart);
                }
            } else {
                std::string indentUnit = detectIndentUnit(mappingIndent);
                entryIndent = mappingIndent + indentUnit;
                fieldIndent = entryIndent + indentUnit;
            }
            std::string insertion;
            if (compat->objectValue->braceEnd > 0 &&
                text[compat->objectValue->braceEnd - 1] != '\n' &&
                text[compat->objectValue->braceEnd - 1] != '\r') {
                insertion += newline;
            }
            insertion += buildCompatEntryBlock(appIdText, toolName, entryIndent, fieldIndent, newline);
            if (!insertBefore(text, compat->objectValue->braceEnd, insertion)) {
                error = "Failed to update compatibility tool";
                return false;
            }
        }
    } else {
        std::string steamEntryIndent;
        std::string entryIndent;
        std::string fieldIndent;
        if (!steam->objectValue->entries.empty()) {
            steamEntryIndent = getLineIndent(text, steam->objectValue->entries.front().keyStart);
            std::string indentUnit = detectIndentUnit(steamEntryIndent);
            entryIndent = steamEntryIndent + indentUnit;
            fieldIndent = entryIndent + indentUnit;
        } else {
            std::string steamIndent = getLineIndent(text, steam->keyStart);
            std::string indentUnit = detectIndentUnit(steamIndent);
            steamEntryIndent = steamIndent + indentUnit;
            entryIndent = steamEntryIndent + indentUnit;
            fieldIndent = entryIndent + indentUnit;
        }
        std::string insertion;
        if (steam->objectValue->braceEnd > 0 &&
            text[steam->objectValue->braceEnd - 1] != '\n' &&
            text[steam->objectValue->braceEnd - 1] != '\r') {
            insertion += newline;
        }
        insertion += buildCompatMappingBlock(appIdText, toolName, steamEntryIndent, entryIndent, fieldIndent, newline);
        if (!insertBefore(text, steam->objectValue->braceEnd, insertion)) {
            error = "Failed to update compatibility tool";
            return false;
        }
    }

    std::ofstream out(configPath, std::ios::trunc);
    if (!out) {
        error = "Failed to write config.vdf";
        return false;
    }
    out << text;
    return true;
}
