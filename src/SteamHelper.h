#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

bool addShortcutToSteam(const std::filesystem::path& exePath,
                        const std::filesystem::path& startDir,
                        const std::string& appName,
                        const std::string& launchOptions,
                        std::uint32_t& appId,
                        std::string& error);

bool setSteamCompatToolMapping(std::uint32_t appId,
                               const std::string& toolName,
                               std::string& error);
