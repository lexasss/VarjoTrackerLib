// Copyright 2021 Varjo Technologies Oy. All rights reserved.

/* Gaze Tracking Example console application
 *
 * - Showcases Varjo Gaze API features
 * - Just run the example and it prints out usage instructions
 * - For CSV data export option, see command line help
 *
 * - If you are interested how to visualize user's gaze, you might want to
 *   take look at Benchmark application instead.
 */

#include <clocale>
#include <iostream>
#include <io.h>

#include <cxxopts.hpp>
#include <Windows.h>

#include "Application.hpp"
#include "Session.hpp"

// Application instance
std::unique_ptr<Application> g_application;

// Callback for handling Ctrl+C
BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
    if (g_application) {
        g_application->terminate();
    }
    return TRUE;
}

// Console application entry point
int main(int argc, char** argv)
{
    // Use UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Setup Ctrl+C handler to exit application cleanly
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    // Disable VarjoLib logging to stdout as it would mess console application UI.
    // Log messages are still printed to debug output visible in debuggers.
    _putenv_s("VARJO_LOGGER_STDOUT_DISABLED", "1");

    try {
        // Initialize session
        auto session = std::make_shared<Session>();
        if (!session->isValid()) {
            throw std::runtime_error("Failed to initialize session. Is Varjo system running?");
        }

        // Initialize application
        g_application = std::make_unique<Application>(std::move(session));

        // Execute application
        g_application->run(nullptr);

        // Application finished
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Critical error caught: " << e.what();
        return EXIT_FAILURE;
    }
}
