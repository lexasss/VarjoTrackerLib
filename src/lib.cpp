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

// DLL exports

extern "C" _declspec(dllexport) bool Init() {

    SetConsoleOutputCP(CP_UTF8);

    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    _putenv_s("VARJO_LOGGER_STDOUT_DISABLED", "1");

    try {
        // Initialize session
        auto session = std::make_shared<Session>();
        if (!session->isValid()) {
            throw std::runtime_error("Failed to initialize session. Is Varjo system running?");
        }

        // Initialize application
        g_application = std::make_unique<Application>(std::move(session));
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error caught: " << e.what();
        return false;
    }

    std::cout << std::endl << "GazeTracking initialized" << std::endl;

	return true;
}

extern "C" _declspec(dllexport) bool Run(GazeCallback gazeCallback) {

    std::cout << "GazeTracking loop started" << std::endl;

    try {
        g_application->run(gazeCallback);
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error caught: " << e.what();
        return false;
    }

    std::cout << "GazeTracking loop ended" << std::endl;

    return true;
}

extern "C" _declspec(dllexport) void Terminate() {

    std::cout << "Terminating GazeTracking loop" << std::endl;

    try {
        if (g_application) {
            g_application->terminate();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error caught: " << e.what();
    }
}
