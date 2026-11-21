//
// Created by alan on 20/11/2026.
//

#include "Debug.h"
#include <spdlog/spdlog.h>
#include <format>

void Debug::Init() {

}

void Debug::Shutdown() {

}

void Debug::LogInfo(std::string message) {
    spdlog::info(message);
}

void LogWarning(std::string message) {
    spdlog::warn(message);
}

void Debug::LogError(std::string message, std::source_location location) {
    spdlog::error(std::format(
            "File:[{0}], Func:[{1}], Line:[{2}] -> {3}",
            location.file_name(), location.function_name(), location.line(), message
        ));
}

void Debug::Assert(bool condition, std::string message, std::source_location location) {
    if (!condition) {
        LogError(message, location);
        std::terminate();
    }
}