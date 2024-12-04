// Copyright 2021 Varjo Technologies Oy. All rights reserved.
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "Application.hpp"

#include "Varjo_math.h"

#include <conio.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <array>
#include <cmath>

ApplicationState getInitialState()
{
    ApplicationState state;
    state.outputFilterType = GazeTracking::OutputFilterType::STANDARD;
    state.outputFrequency = GazeTracking::OutputFrequency::_100HZ;
    state.status = GazeTracking::Status::NOT_AVAILABLE;
    return state;
}

Application::Application(const std::shared_ptr<Session>& session)
    : m_state(getInitialState())
    , m_session(session)
    , m_gazeTracking(session)
{
}

void Application::run(GazeCallback cb)
{
    while (m_running) {
        varjo_FrameInfo* frameInfo = varjo_CreateFrameInfo(*m_session);
        varjo_WaitSync(*m_session, frameInfo);

        auto matrix = varjo_FrameGetPose(*m_session, varjo_PoseType_Center);
        auto angles = varjo_GetEulerAngles(&matrix, varjo_EulerOrder_ZYX, varjo_RotationDirection_CounterClockwise, varjo_Handedness_RightHanded);

        constexpr double radiansToDegrees = 180.0 / M_PI;
        std::array<double, 3> headRotation = { radiansToDegrees * angles.x, radiansToDegrees * angles.y, radiansToDegrees * angles.z };

        std::array<double, 3> gazeAngles{};
        if (update(gazeAngles) && cb)
        {
            if (!cb(frameInfo->displayTime, gazeAngles[0], gazeAngles[1], gazeAngles[2], headRotation[0], headRotation[1], headRotation[2]))
            {
                terminate();
            }
        }

        varjo_FreeFrameInfo(frameInfo);

        // Note: This value can be increased to lower CPU usage, but time between
        // GazeTracking.getGazeData() (varjo_GetGazeArray) calls should not exceed
        // 500ms or samples might be lost.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Application::terminate() { m_running = false; }

namespace
{
bool isGazeTrackingAvailable(GazeTracking::Status status)
{
    return (status != GazeTracking::Status::NOT_AVAILABLE) && (status != GazeTracking::Status::NOT_CONNECTED);
}
}  // namespace

bool Application::update(std::array<double, 3>& gaze)
{
    // Check gaze tracking status (we can get status without initialization)
    const auto previousStatus = m_state.status;
    m_state.status = m_gazeTracking.getStatus();

    if (m_state.hasError()) {
        // Reinitialize automatically, if gaze tracking is now available
        if ((m_state.status != previousStatus) && !isGazeTrackingAvailable(previousStatus) && isGazeTrackingAvailable(m_state.status)) {
            m_initialized = false;
            resetErrorState();

        } else {
            // Don't continue, if we have error state. User can trigger reinitialization.
            return false;
        }
    }

    // Initialize IPD parameters state
    m_state.headsetIPD = m_gazeTracking.getHeadsetIPD();
    m_state.ipdAdjustmentMode = m_gazeTracking.getIPDAdjustmentMode();
    // Reset last requested IPD if headset IPD is not available (e.g. if headset got disconnected)
    if (!m_state.headsetIPD.has_value()) {
        m_state.requestedIPD.reset();
    }

    // Initialize gaze tracking, if needed
    if (!m_initialized) {
        m_gazeTracking.initialize(m_state.outputFilterType, m_state.outputFrequency);
        if (!checkError("Failed to initialize gaze tracking")) {
            return false;
        }
        m_initialized = true;
    }

    // Fetch gaze data
    const auto gazeDataWithEyeMeasurements = m_gazeTracking.getGazeDataWithEyeMeasurements();
    if (gazeDataWithEyeMeasurements.empty()) {
        if (!checkError("Failed to read gaze data")) {
            return false;
        }
    }

    // Update state with latest gaze
    if (!gazeDataWithEyeMeasurements.empty()) {
        m_state.gaze = gazeDataWithEyeMeasurements.back().first;
        m_state.eyeMeasurements = gazeDataWithEyeMeasurements.back().second;

        for (int i = 0; i < 3; i++)
            gaze[i] = m_state.gaze.gaze.forward[i];

        return m_state.gaze.status != varjo_GazeStatus_Invalid;
    }

    return false;
}

bool Application::checkError(const std::string& messagePrefix)
{
    m_state.lastError = m_session->getError();
    if (!m_state.lastError.empty()) {
        m_state.lastError = messagePrefix + ": " + m_state.lastError;
        return false;
    }

    return true;
}

void Application::resetErrorState() { m_state.lastError.clear(); }
