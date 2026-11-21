//
// Created by alan on 20/11/2026.
//

#ifndef DERANGED_RHI_DEBUG_H
#define DERANGED_RHI_DEBUG_H

#include <string>
#include <format>
#include <source_location>

class Debug {
public:
    static void LogInfo(std::string message);
    static void LogWarning(std::string message);
    static void LogError(std::string message);
    static void LogCritical(std::string message);

    static void Assert(bool condition, std::string message, std::source_location location = std::source_location::current());

};

#endif //DERANGED_RHI_DEBUG_H
