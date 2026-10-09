// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Monotonic desktop playback with bounded step recovery.

#include "simulationpacer.h"
#include <algorithm>
#include <cmath>
void SimulationPacer::reset()
{
    m_active = false;
    m_elapsed = m_peakLag = 0;
    m_lateDispatches = 0;
}
void SimulationPacer::begin(double wall, double virtualTime)
{
    m_wallStart = wall;
    m_virtualStart = virtualTime;
    m_active = true;
}
void SimulationPacer::pause(double wall)
{
    if (m_active)
        m_elapsed += std::max(0.0, wall - m_wallStart);
    m_active = false;
}
bool SimulationPacer::setRate(double rate)
{
    if (!std::isfinite(rate) || (rate != .5 && rate != 1 && rate != 4))
        return false;
    m_rate = rate;
    return true;
}
int SimulationPacer::dueSteps(double wall, double virtualTime, double dt)
{
    if (!m_active || dt <= 0)
        return 0;
    const double target = m_virtualStart + std::max(0.0, wall - m_wallStart) * m_rate;
    const double overdue = std::max(0.0, target - virtualTime);
    m_peakLag = std::max(m_peakLag, overdue / m_rate);
    const double steps = std::floor((overdue + 1e-9) / dt);
    if (steps > 1)
        ++m_lateDispatches;
    return int(std::min(double(MaxCatchUpSteps), steps));
}
QVariantMap SimulationPacer::status(double wall, double virtualTime) const
{
    const double segment = m_active ? std::max(0.0, wall - m_wallStart) : 0;
    const double target = m_virtualStart + segment * m_rate;
    return {{"mode", "desktop-soft-realtime"},
            {"rate", m_rate},
            {"active", m_active},
            {"wall_elapsed_s", m_elapsed + segment},
            {"virtual_time_s", virtualTime},
            {"lag_ms", m_active ? std::max(0.0, target - virtualTime) * 1000 / m_rate : 0},
            {"peak_lag_ms", m_peakLag * 1000},
            {"late_dispatches", m_lateDispatches},
            {"hard_realtime", false}};
}
