// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Executed demonstration acceptance criteria and campaign evidence.

#include "campaign.h"
#include "report.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <slx/resilience.h>
namespace campaign
{
QJsonObject execute(const QString &plugin, const QString &version, const QString &directory)
{
    const QStringList files{"sensor-stuck.json", "main-loss.json", "measurement-loss.json"};
    const QStringList ids{"DEMO-SENSOR-01", "DEMO-FAILOVER-01", "DEMO-STOP-01"};
    const QStringList criteria{"S1 isolated; final error ≤ 0.5 percentage points; no stops or takeovers.",
                               "Backup owns control; one takeover; final error ≤ 0.5 percentage points.",
                               "No quorum; zero motor command; at least one stop."};
    QJsonArray runs;
    int passed = 0;
    QString fingerprint;
    QString pluginPath = plugin;
    if (!plugin.isEmpty())
    {
        if (!QFileInfo(plugin).isAbsolute())
            pluginPath = QFileInfo::exists(plugin)
                             ? QFileInfo(plugin).absoluteFilePath()
                             : QDir(QCoreApplication::applicationDirPath()).filePath(plugin);
        QFile f(pluginPath);
        if (!f.open(QIODevice::ReadOnly))
            return {{"error", "Cannot read customer DLL: " + f.errorString()}};
        fingerprint =
            QString::fromLatin1(QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256).toHex());
    }
    for (int i = 0; i < 3; ++i)
    {
        auto scenario = slx::Session::preset(i);
        if (!directory.isEmpty())
        {
            QFile f(QDir(directory).filePath(files[i]));
            if (!f.open(QIODevice::ReadOnly))
                return {{"error", f.errorString()}};
            QJsonParseError parseError;
            const auto doc = QJsonDocument::fromJson(f.readAll(), &parseError);
            if (parseError.error != QJsonParseError::NoError || !doc.isObject())
                return {{"error", "Invalid scenario: " + files[i]}};
            scenario = doc.object();
        }
        scenario["control_version"] = version;
        if (!pluginPath.isEmpty())
            scenario["customer_plugin"] = pluginPath;
        slx::Session session;
        QString error;
        if (!session.load(scenario, &error))
            return {{"error", error}};
        session.runToEnd();
        const auto result = session.result();
        const auto metrics = result["metrics"].toObject();
        const auto final = result["final"].toObject();
        bool sensorIsolated = false;
        for (const auto &event : result["events"].toArray())
        {
            const auto record = event.toObject();
            sensorIsolated |= record["code"].toString() == "sensor_isolated" && record["sensor"].toInt() == 0;
        }
        const bool ok = i == 0 ? sensorIsolated && metrics["final_error_pct"].toDouble() <= .5 &&
                                     metrics["stops"].toInt() == 0 && metrics["takeovers"].toInt() == 0
                        : i == 1
                            ? final["owner"].toString() == "backup" && metrics["takeovers"].toInt() == 1 &&
                                  metrics["final_error_pct"].toDouble() <= .5
                            : !final["quorum"].toBool() && final["motor"].toDouble() == 0 &&
                                  metrics["stops"].toInt() >= 1;
        passed += ok ? 1 : 0;
        runs.append(QJsonObject{
            {"requirement", ids[i]}, {"criterion", criteria[i]}, {"passed", ok}, {"result", result}});
    }
    return {{"schema", "siliconlogix.campaign.v1"},
            {"execution", "deterministic-batch"},
            {"passed", passed},
            {"total", 3},
            {"all_passed", passed == 3},
            {"customer_plugin_sha256", fingerprint},
            {"runs", runs}};
}
bool write(const QJsonObject &result, const QString &path, QString *error)
{
    if (result.contains("error"))
    {
        if (error)
            *error = result["error"].toString();
        return false;
    }
    QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath()))
    {
        if (error)
            *error = "Cannot create campaign directory";
        return false;
    }
    const QString base = info.absolutePath() + "/" + info.completeBaseName();
    QString html =
        "<!doctype html><html lang='en'><meta charset='utf-8'><title>SLX · "
        "Campaign</title><style>body{font:15px "
        "system-ui;background:#f3f5fb;color:#18203b}main{max-width:1000px;margin:32px "
        "auto;padding:32px;background:white;border-radius:16px}table{width:100%;border-collapse:collapse}td,"
        "th{text-align:left;padding:12px;border-bottom:1px solid "
        "#dfe4f0}h1,a{color:#3436dc}.pass{color:#167565}.fail{color:#b4484e}p{line-height:1.5}</"
        "style><main><h1>SLX ResilienceLab · Regression campaign</h1><p>" +
        QString::number(result["passed"].toInt()) +
        " / 3 demonstration criteria passed. Deterministic batch execution; no "
        "safety certification or testing on "
        "hardware.</p><table><tr><th>Criterion</th><th>Expected</th><th>Result</th><th>Evidence</th></tr>";
    int i = 0;
    for (const auto &entry : result["runs"].toArray())
    {
        const auto run = entry.toObject();
        const QString runPath = base + QString("-run-%1.html").arg(++i);
        if (!reporting::write(run["result"].toObject(), runPath, error))
            return false;
        html += "<tr><td>" + run["requirement"].toString().toHtmlEscaped() + "</td><td>" +
                run["criterion"].toString().toHtmlEscaped() + "</td><td class='" +
                (run["passed"].toBool() ? "pass'>PASS" : "fail'>FAIL") + "</td><td><a href='" +
                QFileInfo(runPath).fileName().toHtmlEscaped() + "'>Complete report</a></td></tr>";
    }
    html += "</table><p>Results are calculated from recorded runs. Criteria apply to this "
            "demonstration case.</p></main></html>";
    auto save = [&](const QString &fileName, const QByteArray &data)
    {
        QSaveFile f(fileName);
        if (f.open(QIODevice::WriteOnly) && f.write(data) == data.size() && f.commit())
            return true;
        if (error)
            *error = f.errorString();
        return false;
    };
    return save(base + ".json", QJsonDocument(result).toJson()) && save(path, html.toUtf8());
}
} // namespace campaign
