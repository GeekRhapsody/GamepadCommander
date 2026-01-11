#pragma once

#include <filesystem>
#include <string>

bool addShortcutToSteam(const std::filesystem::path& exePath,
                        const std::filesystem::path& startDir,
                        const std::string& appName,
                        const std::string& launchOptions,
                        std::string& error);
