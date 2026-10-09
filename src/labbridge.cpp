// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop session facade and fixed-step scheduling.

#include "labbridge.h"
#include "campaign.h"
#include "report.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <cmath>
LabBridge::LabBridge(QObject *parent) : QObject(parent)
{
    connect(this, &LabBridge::changed, this, [this] { refresh(true); });
    m_wallClock.start();
    m_chartClock.start();
    m_timer.setInterval(10);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this,
            [this]
            {
                const auto before = m_session.snapshot();
                const int due =
                    m_pacer.dueSteps(wallSeconds(), before["time"].toDouble(),
                                     m_session.scenario()["config"].toObject()["dt_s"].toDouble(.02));
                QElapsedTimer budget;
                budget.start();
                int steps = 0;
                while (steps < due && !m_session.finished())
                {
                    m_session.step();
                    ++steps;
                    if (budget.nsecsElapsed() >= 4000000)
                        break;
                }
                if (m_session.finished())
                {
                    pauseExecution();
                    m_message = "Scenario complete. Export the report or compare control versions.";
                }
                updateClock();
                if (steps > 0)
                    refresh(m_session.finished());
            });
    refresh(true);
    updateClock();
}
void LabBridge::synchronize()
{
    pauseExecution();
    m_pacer.reset();
    updateClock();
    refresh(true);
}
double LabBridge::wallSeconds() const
{
    return m_wallClock.nsecsElapsed() / 1000000000.0;
}
void LabBridge::updateClock()
{
    m_clock = m_pacer.status(wallSeconds(), m_session.snapshot()["time"].toDouble());
    emit clockChanged();
}
void LabBridge::pauseExecution()
{
    m_timer.stop();
    m_pacer.pause(wallSeconds());
    updateClock();
}
void LabBridge::setPlaybackRate(double rate)
{
    if (rate != .5 && rate != 1 && rate != 4)
        return;
    const bool wasRunning = running();
    m_pacer.pause(wallSeconds());
    m_pacer.setRate(rate);
    if (wasRunning)
        m_pacer.begin(wallSeconds(), m_session.snapshot()["time"].toDouble());
    updateClock();
}
QString LabBridge::applicationVersion() const
{
    return QCoreApplication::applicationVersion();
}
QString LabBridge::controllerLabel() const
{
    const auto plugin = m_scenarioCache["customer_plugin"].toString();
    return plugin.isEmpty() ? m_scenarioCache["control_version"].toString() : QFileInfo(plugin).fileName();
}
QString LabBridge::document(QString name) const
{
    const QMap<QString, QString> documents{{"license", ":/legal/LICENSE"},
                                           {"notices", ":/legal/third-party-notices"},
                                           {"qt-license", ":/legal/qt-license"},
                                           {"sdk", ":/docs/sdk.md"},
                                           {"integration", ":/docs/customer-integration.md"},
                                           {"bench-api", ":/docs/bench-api.md"}};
    QFile file(documents.value(name));
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : "Document unavailable.";
}
void LabBridge::refresh(bool force)
{
    m_state = m_session.snapshot();
    emit stateChanged();
    const auto samples = m_session.samples();
    if (force || (m_chartClock.elapsed() >= 100 && m_historyCount != samples.size()))
    {
        QVariantList chart;
        auto append = [&](const QVariantMap &sample)
        {
            const auto readings = sample.value("readings").toList();
            QVariantMap point{{"time", sample.value("time")},
                              {"target", sample.value("target")},
                              {"position", sample.value("position")},
                              {"fused", sample.value("fused")}};
            for (int i = 0; i < readings.size(); ++i)
                point[QString("sensor%1").arg(i + 1)] = readings[i].toMap().value("value");
            chart.append(point);
        };
        for (qsizetype i = 0; i < samples.size(); i += 3)
            append(samples[i]);
        if (!samples.isEmpty() && (samples.size() - 1) % 3 != 0)
            append(samples.last());
        m_history = std::move(chart);
        m_historyCount = samples.size();
        emit historyChanged();
        m_chartClock.restart();
    }
    const auto events = m_session.events();
    if (force || m_eventCount != events.size())
    {
        m_events.clear();
        for (auto i = events.crbegin(); i != events.crend(); ++i)
            m_events.append(*i);
        m_eventCount = events.size();
        emit eventsChanged();
    }
    if (force)
    {
        const auto scenario = m_session.scenario();
        if (scenario != m_scenarioCache)
        {
            m_scenarioCache = scenario;
            m_scenarioName = scenario["name"].toString();
            m_config = scenario["config"].toObject().toVariantMap();
            m_timeline = scenario["faults"].toArray().toVariantList();
            emit scenarioChanged();
        }
        if (m_comparison != m_lastComparison)
        {
            m_lastComparison = m_comparison;
            m_comparisonCache = m_comparison.toVariantMap();
            emit comparisonChanged();
        }
        emit statusChanged();
    }
    updateGuide();
}
void LabBridge::updateGuide()
{
    const int index = slx::Session::presetNames().indexOf(m_scenarioName);
    const QStringList questions{"Does control recognize a sensor that no longer follows the valve?",
                                "Does control continue if the primary stops?",
                                "Does the motor stop when trustworthy measurements are insufficient?"};
    const QStringList failures{"S1 stops updating while the valve moves.",
                               "The primary stops executing control and sending heartbeat.",
                               "S1 and S2 stop sending measurements; only S3 remains."};
    const QStringList expectations{"Exclude S1 and reach the requested opening using S2 and S3.",
                                   "Stop the motor temporarily and grant control to backup.",
                                   "Detect expired measurements and keep the motor command at zero."};
    const QStringList codes{"sensor_isolated", "takeover", "safety_stop"};
    QVariantMap guide;
    if (index < 0)
        guide = {{"question", "What happens to control when the product fails?"},
                 {"fault", "Custom scenario: inspect the fault timeline."},
                 {"expected", "Observe measurements, decisions and motion; preserve evidence in the report."},
                 {"phase", "Custom test"},
                 {"observed", "Events show decisions actually executed."},
                 {"status", "neutral"}};
    else
    {
        bool injected = false, responded = false;
        double responseTime = 0;
        for (const auto &entry : m_events)
        {
            const auto event = entry.toMap();
            const auto code = event["code"].toString();
            if (code == "fault_injected")
                injected = true;
            if (code == codes[index] && (index != 0 || event["sensor"].toInt() == 0))
            {
                responded = true;
                responseTime = event["time"].toDouble();
            }
        }
        const bool finished = m_session.finished();
        const double error =
            std::abs(m_state.value("target").toDouble() - m_state.value("position").toDouble());
        const bool ok =
            responded &&
            (index == 0   ? m_state.value("quorum").toBool() && error <= .5
             : index == 1 ? m_state.value("owner").toString() == "backup" && error <= .5
                          : !m_state.value("quorum").toBool() && m_state.value("motor").toDouble() == 0);
        QString phase = finished    ? (ok ? "Response verified" : "Response needs analysis")
                        : responded ? "Control response observed"
                        : injected  ? "Fault applied"
                                    : "Before the fault";
        QString observed =
            !injected    ? "Run the test: the valve attempts to reach the requested opening."
            : !responded ? "Fault injected. Control must recognize it from measurements or missing messages."
            : index == 0 ? QString("S1 isolated at %1 s. Control uses agreeing measurements.")
                               .arg(responseTime, 0, 'f', 2)
            : index == 1
                ? QString("Backup authorized at %1 s. The gateway decides who commands the motor.")
                      .arg(responseTime, 0, 'f', 2)
                : QString("Stop applied at %1 s: two readings are unavailable.").arg(responseTime, 0, 'f', 2);
        if (finished && index < 2)
            observed += QString(" Final error: %1 percentage points.").arg(error, 0, 'f', 2);
        if (finished && !ok)
            observed += " Demonstration criterion failed: inspect the events.";
        guide = {{"question", questions[index]},
                 {"fault", failures[index]},
                 {"expected", expectations[index]},
                 {"phase", phase},
                 {"observed", observed},
                 {"status", finished    ? (ok ? "success" : "attention")
                            : responded ? "success"
                                        : "neutral"}};
    }
    if (guide != m_guide)
    {
        m_guide = std::move(guide);
        emit guideChanged();
    }
}
void LabBridge::load(QJsonObject scenario)
{
    QString error;
    pauseExecution();
    if (m_session.load(scenario, &error))
    {
        m_comparison = {};
        m_pacer.reset();
        m_campaign = {};
        emit campaignChanged();
        updateClock();
        m_message = "Scenario ready: " + scenario["name"].toString();
    }
    else
        m_message = "Error: " + error;
    emit changed();
}
void LabBridge::selectScenario(int index)
{
    load(slx::Session::preset(index));
}
void LabBridge::startDemo(int index)
{
    selectScenario(index);
    toggleRun();
}
void LabBridge::startLive()
{
    auto scenario = m_session.scenario();
    scenario["name"] = "Live bench";
    auto config = scenario["config"].toObject();
    config["duration_s"] = 300;
    config["initial_percent"] = 0;
    config["target_percent"] = 0;
    scenario["config"] = config;
    scenario["faults"] = QJsonArray();
    load(scenario);
    setPlaybackRate(1);
    toggleRun();
}
void LabBridge::injectSensors(QString kind, int sensor, double value)
{
    if (!kind.startsWith("sensor_") || !slx::Session::faultNames().contains(kind) || sensor < -1 ||
        sensor > 2)
        return;
    for (int channel = 0; channel < 3; ++channel)
        if (sensor == -1 || sensor == channel)
            m_session.inject(kind, channel, value);
    m_comparison = {};
    m_message = "Sensor action scheduled for the next step: " + kind;
    emit changed();
}
void LabBridge::toggleRun()
{
    if (running())
    {
        pauseExecution();
        m_message = "Simulation paused.";
    }
    else
    {
        if (m_session.finished())
        {
            m_session.reset();
            m_pacer.reset();
        }
        m_comparison = {};
        m_pacer.begin(wallSeconds(), m_session.snapshot()["time"].toDouble());
        m_timer.start();
        updateClock();
        m_message = "Simulation running.";
    }
    emit changed();
}
void LabBridge::reset()
{
    pauseExecution();
    m_session.reset();
    m_pacer.reset();
    updateClock();
    m_comparison = {};
    m_message = "Scenario reset using the same seed.";
    emit changed();
}
void LabBridge::step()
{
    pauseExecution();
    m_session.step();
    updateClock();
    emit changed();
}
void LabBridge::complete()
{
    pauseExecution();
    m_session.runToEnd();
    updateClock();
    m_message = "Scenario complete.";
    emit changed();
}
void LabBridge::setTarget(double value)
{
    m_session.setTarget(value);
    m_comparison = {};
    emit changed();
}
void LabBridge::inject(QString kind, int sensor, double value)
{
    m_session.inject(kind, sensor, value);
    m_comparison = {};
    m_message = "Fault scheduled for the next step: " + kind;
    emit changed();
}
void LabBridge::configure(QVariantMap changes, int seed, QString version)
{
    auto doc = m_session.scenario();
    auto cfg = doc["config"].toObject();
    for (auto i = changes.cbegin(); i != changes.cend(); ++i)
        cfg[i.key()] = QJsonValue::fromVariant(i.value());
    doc["config"] = cfg;
    doc["seed"] = seed;
    doc["control_version"] = version;
    load(doc);
}
void LabBridge::saveScenario(QUrl url)
{
    QSaveFile file(url.toLocalFile());
    if (file.open(QIODevice::WriteOnly) && file.write(QJsonDocument(m_session.scenario()).toJson()) >= 0 &&
        file.commit())
        m_message = "Scenario saved: " + url.toLocalFile();
    else
        m_message = "Save error: " + file.errorString();
    emit changed();
}
void LabBridge::loadScenario(QUrl url)
{
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly))
    {
        m_message = file.errorString();
        emit changed();
        return;
    }
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
    {
        m_message = "Invalid JSON: " + error.errorString();
        emit changed();
        return;
    }
    load(doc.object());
}
void LabBridge::exportReport(QUrl url)
{
    QString error;
    auto report = m_comparison.isEmpty() ? m_session.result() : m_comparison;
    if (m_comparison.isEmpty() && m_clock["wall_elapsed_s"].toDouble() > 0)
        report["desktop_execution"] = QJsonObject::fromVariantMap(m_clock);
    if (reporting::write(report, url.toLocalFile(), &error))
        m_message = "HTML, JSON and CSV reports exported: " + url.toLocalFile();
    else
        m_message = "Export error: " + error;
    emit changed();
}
void LabBridge::compareVersions()
{
    pauseExecution();
    m_message = "Comparison running…";
    emit changed();
    m_comparison = m_session.compare(
        m_session.scenario()["control_version"].toString() == "robust-v1" ? "cautious-v2" : "robust-v1");
    m_message = m_comparison.contains("error") ? m_comparison["error"].toString()
                                               : "A/B comparison complete: same scenario and seed.";
    emit changed();
}
void LabBridge::loadPlugin(QUrl url)
{
    auto doc = m_session.scenario();
    doc["customer_plugin"] = url.toLocalFile();
    load(doc);
}
void LabBridge::loadExamplePlugin()
{
#if defined(Q_OS_WIN)
    const auto file = "SLX.CustomerController.Example.dll";
#elif defined(Q_OS_DARWIN)
    const auto file = "SLX.CustomerController.Example.dylib";
#else
    const auto file = "SLX.CustomerController.Example.so";
#endif
    loadPlugin(QUrl::fromLocalFile(QDir(QCoreApplication::applicationDirPath()).filePath(file)));
}
void LabBridge::runCampaign()
{
    pauseExecution();
    const auto scenario = m_session.scenario();
    m_campaign = campaign::execute(scenario["customer_plugin"].toString(),
                                   scenario["control_version"].toString("robust-v1"));
    m_message = m_campaign.contains("error")
                    ? m_campaign["error"].toString()
                    : QString("Campaign complete: %1 / 3 criteria passed.").arg(m_campaign["passed"].toInt());
    emit campaignChanged();
    emit statusChanged();
}
QVariantMap LabBridge::campaignSummary() const
{
    if (m_campaign.isEmpty() || m_campaign.contains("error"))
        return m_campaign.toVariantMap();
    QVariantList runs;
    for (const auto &entry : m_campaign["runs"].toArray())
    {
        const auto run = entry.toObject();
        const auto result = run["result"].toObject();
        runs.append(QVariantMap{{"requirement", run["requirement"].toString()},
                                {"criterion", run["criterion"].toString()},
                                {"passed", run["passed"].toBool()},
                                {"name", result["scenario"].toObject()["name"].toString()},
                                {"metrics", result["metrics"].toObject().toVariantMap()}});
    }
    return {{"passed", m_campaign["passed"].toInt()},
            {"total", 3},
            {"runs", runs},
            {"customer_plugin_sha256", m_campaign["customer_plugin_sha256"].toString()}};
}
void LabBridge::exportCampaign(QUrl url)
{
    QString error;
    if (m_campaign.isEmpty())
        m_message = "Run the campaign first.";
    else
        m_message = campaign::write(m_campaign, url.toLocalFile(), &error)
                        ? "Campaign and complete reports exported: " + url.toLocalFile()
                        : "Error: " + error;
    emit statusChanged();
}
void LabBridge::saveIntegrationBrief(QUrl url, QVariantMap needs)
{
    QJsonObject brief{
        {"schema", "siliconlogix.integration-request.v1"},
        {"application", "SLX ResilienceLab"},
        {"application_version", applicationVersion()},
        {"request", QJsonObject::fromVariantMap(needs)},
        {"available", "C ABI v1 / SIL; deterministic replay; desktop soft realtime; batch regression"},
        {"requested_extensions",
         "Hardware I/O and dedicated realtime target require a commissioned integration"}};
    QSaveFile file(url.toLocalFile());
    const auto data = QJsonDocument(brief).toJson();
    m_message = file.open(QIODevice::WriteOnly) && file.write(data) == data.size() && file.commit()
                    ? "Technical request saved. Attach it to your inquiry: " + url.toLocalFile()
                    : "Error: " + file.errorString();
    emit statusChanged();
}
void LabBridge::removePlugin()
{
    auto doc = m_session.scenario();
    doc.remove("customer_plugin");
    load(doc);
}
void LabBridge::clearTimeline()
{
    auto doc = m_session.scenario();
    doc["faults"] = QJsonArray();
    doc["name"] = "Custom scenario";
    load(doc);
}
void LabBridge::scheduleFault(double at, QString kind, int sensor, double value)
{
    auto doc = m_session.scenario();
    auto faults = doc["faults"].toArray();
    faults.append(QJsonObject{{"at_s", at}, {"kind", kind}, {"sensor", sensor}, {"value", value}});
    doc["faults"] = faults;
    load(doc);
}
