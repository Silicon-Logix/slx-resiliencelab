// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop session facade and fixed-step scheduling.

#pragma once
#include "simulationpacer.h"
#include <QElapsedTimer>
#include <QJsonArray>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <slx/resilience.h>
class LabBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantList events READ events NOTIFY eventsChanged)
    Q_PROPERTY(bool running READ running NOTIFY statusChanged)
    Q_PROPERTY(QString message READ message NOTIFY statusChanged)
    Q_PROPERTY(QString scenarioName READ scenarioName NOTIFY scenarioChanged)
    Q_PROPERTY(QStringList scenarioNames READ scenarioNames CONSTANT)
    Q_PROPERTY(QVariantMap config READ config NOTIFY scenarioChanged)
    Q_PROPERTY(QVariantList timeline READ timeline NOTIFY scenarioChanged)
    Q_PROPERTY(QVariantMap comparison READ comparison NOTIFY comparisonChanged)
    Q_PROPERTY(QVariantMap guide READ guide NOTIFY guideChanged)
    Q_PROPERTY(QVariantMap clock READ clock NOTIFY clockChanged)
    Q_PROPERTY(double playbackRate READ playbackRate NOTIFY clockChanged)
    Q_PROPERTY(QVariantMap campaign READ campaignSummary NOTIFY campaignChanged)
    Q_PROPERTY(QString applicationVersion READ applicationVersion CONSTANT)
    Q_PROPERTY(QString controllerLabel READ controllerLabel NOTIFY scenarioChanged)
  public:
    explicit LabBridge(QObject *parent = nullptr);
    QVariantMap state() const
    {
        return m_state;
    }
    QVariantList history() const
    {
        return m_history;
    }
    QVariantList events() const
    {
        return m_events;
    }
    QVariantMap config() const
    {
        return m_config;
    }
    QVariantList timeline() const
    {
        return m_timeline;
    }
    QVariantMap comparison() const
    {
        return m_comparisonCache;
    }
    QVariantMap guide() const
    {
        return m_guide;
    }
    bool running() const
    {
        return m_timer.isActive();
    }
    QVariantMap clock() const
    {
        return m_clock;
    }
    double playbackRate() const
    {
        return m_pacer.rate();
    }
    QVariantMap campaignSummary() const;
    QString applicationVersion() const;
    QString controllerLabel() const;
    Q_INVOKABLE QString document(QString name) const;
    Q_INVOKABLE void setPlaybackRate(double rate);
    Q_INVOKABLE void loadExamplePlugin();
    Q_INVOKABLE void runCampaign();
    Q_INVOKABLE void exportCampaign(QUrl url);
    Q_INVOKABLE void saveIntegrationBrief(QUrl url, QVariantMap needs);
    QString message() const
    {
        return m_message;
    }
    QString scenarioName() const
    {
        return m_scenarioName;
    }
    QStringList scenarioNames() const
    {
        return slx::Session::presetNames();
    }
    Q_INVOKABLE void selectScenario(int index);
    Q_INVOKABLE void startDemo(int index);
    // Start a five-minute nominal bench; manual actions remain replayable.
    Q_INVOKABLE void startLive();
    Q_INVOKABLE void injectSensors(QString kind, int sensor, double value = 0);
    Q_INVOKABLE void toggleRun();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void step();
    Q_INVOKABLE void complete();
    Q_INVOKABLE void setTarget(double value);
    Q_INVOKABLE void inject(QString kind, int sensor, double value);
    Q_INVOKABLE void configure(QVariantMap changes, int seed, QString version);
    Q_INVOKABLE void saveScenario(QUrl url);
    Q_INVOKABLE void loadScenario(QUrl url);
    Q_INVOKABLE void exportReport(QUrl url);
    Q_INVOKABLE void compareVersions();
    Q_INVOKABLE void loadPlugin(QUrl url);
    Q_INVOKABLE void removePlugin();
    Q_INVOKABLE void clearTimeline();
    Q_INVOKABLE void scheduleFault(double at, QString kind, int sensor, double value);
    void synchronize(); // For CLI/capture callers using the public Session directly.
    slx::Session &session()
    {
        return m_session;
    }
  signals:
    void changed();
    void stateChanged();
    void historyChanged();
    void eventsChanged();
    void statusChanged();
    void scenarioChanged();
    void comparisonChanged();
    void guideChanged();
    void clockChanged();
    void campaignChanged();

  private:
    slx::Session m_session;
    QTimer m_timer;
    QElapsedTimer m_wallClock, m_chartClock;
    SimulationPacer m_pacer;
    QVariantMap m_clock;
    QJsonObject m_campaign;
    QString m_message = "Ready. Select a scenario and run the simulation.";
    QJsonObject m_comparison;
    QVariantMap m_state, m_config, m_comparisonCache, m_guide;
    QVariantList m_history, m_events, m_timeline;
    QString m_scenarioName;
    QJsonObject m_scenarioCache, m_lastComparison;
    qsizetype m_historyCount = -1, m_eventCount = -1;
    void refresh(bool force);
    void updateGuide();
    void load(QJsonObject scenario);
    void pauseExecution();
    void updateClock();
    double wallSeconds() const;
};
