// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Project resilience implementation.

#pragma once
#include <QJsonObject>
#include <QString>
#include <QVariantMap>
#include <QVector>
#include <memory>
#if defined(_WIN32)
#if defined(SLX_CORE_BUILD)
#define SLX_API __declspec(dllexport)
#else
#define SLX_API __declspec(dllimport)
#endif
#else
#define SLX_API __attribute__((visibility("default")))
#endif

namespace slx
{
// Public facade: Qt 6 ABI. Implementation, algorithms and fault state stay private.
// All operations run on the calling thread; one Session per independent replay.
class SLX_API Session final
{
  public:
    Session();
    ~Session();
    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;
    static QJsonObject preset(int index);
    static QStringList presetNames();
    static QStringList faultNames();
    static QString version();
    bool load(const QJsonObject &scenario, QString *error = nullptr);
    void reset();
    void step(); // Fixed, scenario-defined virtual clock; independent of UI timing.
    void runToEnd();
    bool finished() const;
    void setTarget(double percent); // Recorded as a replayable timeline event.
    void inject(const QString &kind, int sensor = 0, double value = 0);
    QJsonObject scenario() const;
    QVariantMap snapshot() const;
    QVector<QVariantMap> samples() const;
    QVector<QVariantMap> events() const;
    QJsonObject result() const;
    QJsonObject compare(const QString &otherVersion) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
} // namespace slx
