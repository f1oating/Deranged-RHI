//
// Created by alan on 20/11/2026.
//

#include "Debug.h"
#include <spdlog/spdlog.h>

void Debug::LogInfo(std::string message) {
    spdlog::info(message);
}

void Debug::LogWarning(std::string message) {
    spdlog::warn(message);
}

void Debug::LogError(std::string message) {
    spdlog::error(message);
}

void Debug::LogCritical(std::string message) {
    spdlog::critical(message);
}

void Debug::Assert(bool condition, std::string message, std::source_location location) {
    if (!condition) {
        LogError(std::format(
            "File:[{0}], Func:[{1}], Line:[{2}] -> {3}",
            location.file_name(), location.function_name(), location.line(), message
        ));
        std::terminate();
    }
}