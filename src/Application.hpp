// Copyright 2021 Varjo Technologies Oy. All rights reserved.

#pragma once

#include <atomic>
#include <filesystem>
#include <optional>

#include <Session.hpp>

#include "GazeTracking.hpp"

typedef bool(__stdcall* GazeCallback)(int64_t timestamp,
    const double gazeX, const double gazeY, const double gazeZ,
    const double headPitch, const double headYaw, const double headRoll,
    const float pupilOpennessLeft, const float pupilSizeLeft,
    const float pupilOpennessRight, const float pupilSizeRight);

struct ApplicationState {
    std::optional<double> headsetIPD;
    std::optional<double> requestedIPD;
    std::string ipdAdjustmentMode;
    GazeTracking::OutputFilterType outputFilterType{};
    GazeTracking::OutputFrequency outputFrequency{};
    GazeTracking::Status status{};
    std::string lastError;
    varjo_Gaze gaze{};
    varjo_EyeMeasurements eyeMeasurements{};

    bool hasError() const { return !lastError.empty(); }
};

// Application logic
class Application
{
public:
    Application(const std::shared_ptr<Session>& session);
    void run(GazeCallback cb);
    void terminate();

private:
    // Type of requested interpupillary distance (IPD) change
    enum class IPDChange { DECREMENT = -1, INCREMENT = 1 };

    // Interpupillary distance (IPD) change step in millimeters
    static constexpr double c_stepIPD = 0.5;

    bool update(std::array<double, 3>& gaze);

    bool checkError(const std::string& messagePrefix);
    void resetErrorState();

    ApplicationState m_state;
    std::shared_ptr<Session> m_session;
    GazeTracking m_gazeTracking;
    std::atomic_bool m_running = true;
    bool m_initialized = false;
};
