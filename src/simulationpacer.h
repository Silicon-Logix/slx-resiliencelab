// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Monotonic desktop playback with bounded step recovery.

#pragma once
#include <QVariantMap>

// Desktop pacing only. Session::step remains a fixed, deterministic virtual step.
// Callers supply a monotonic wall clock; work is bounded to keep the UI responsive.
class SimulationPacer
{
  public:
    void reset();
    void begin(double wallSeconds, double virtualSeconds);
    void pause(double wallSeconds);
    bool setRate(double rate);
    double rate() const
    {
        return m_rate;
    }
    int dueSteps(double wallSeconds, double virtualSeconds, double dt);
    QVariantMap status(double wallSeconds, double virtualSeconds) const;
    static constexpr int MaxCatchUpSteps = 8;

  private:
    bool m_active = false;
    double m_rate = 1, m_wallStart = 0, m_virtualStart = 0, m_elapsed = 0;
    double m_peakLag = 0;
    int m_lateDispatches = 0;
};
