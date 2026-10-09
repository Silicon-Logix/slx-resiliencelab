// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Standalone simulation evidence export.

#include "report.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
namespace
{
QString number(const QJsonValue &value, int precision = 3)
{
    return value.isDouble() ? QString::number(value.toDouble(), 'f', precision) : QString("—");
}
bool save(const QString &path, const QByteArray &bytes, QString *error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
    {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}
QString chart(const QJsonArray &samples)
{
    QString svg = "<svg viewBox='0 0 1000 240' role='img' aria-label='Requested opening, actual position and "
                  "control measurement "
                  "used'>";
    const double duration = samples.isEmpty() ? 1 : samples.last().toObject()["time"].toDouble();
    for (int y = 0; y <= 100; y += 20)
        svg += QString("<line x1='42' y1='%1' x2='985' y2='%1' stroke='#e3e8ef'/><text x='5' y='%2' "
                       "fill='#64748b' font-size='12'>%3</text>")
                   .arg(215 - y * 1.9)
                   .arg(219 - y * 1.9)
                   .arg(y);
    const QStringList keys{"target", "position", "fused"}, colors{"#7b83a4", "#4338ca", "#0f9f8c"};
    for (int k = 0; k < 3; ++k)
    {
        QString path;
        bool connected = false;
        for (const auto &v : samples)
        {
            auto s = v.toObject();
            if (!s[keys[k]].isDouble())
            {
                connected = false;
                continue;
            }
            path += QString("%1%2,%3 ")
                        .arg(connected ? "L" : "M")
                        .arg(42 + s["time"].toDouble() / duration * 943)
                        .arg(215 - s[keys[k]].toDouble() * 1.9);
            connected = true;
        }
        svg += QString("<path d='%1' fill='none' stroke='%2' stroke-width='2'/>").arg(path, colors[k]);
    }
    return svg + "</svg>";
}
QString section(const QJsonObject &r, const QString &label)
{
    const auto metrics = r["metrics"].toObject(), scenario = r["scenario"].toObject();
    QString html = "<h2>" + label.toHtmlEscaped() + "</h2><p>Scenario: <b>" +
                   scenario["name"].toString().toHtmlEscaped() + "</b> · Seed " +
                   number(scenario["seed"], 0) + " · Controllo " +
                   scenario["control_version"].toString().toHtmlEscaped() + "</p>";
    html += "<table><tr><th>Final error</th><th>Mean absolute error</th><th>Backup "
            "takeovers</th><th>Stops</th><th>Dropped messages</th></tr><tr>";
    for (const QString &k :
         QStringList{"final_error_pct", "mean_absolute_error_pct", "takeovers", "stops", "messages_dropped"})
        html += "<td>" + number(metrics[k], k.endsWith("pct") ? 3 : 0) + (k.endsWith("pct") ? " %" : "") +
                "</td>";
    html += "</tr></table>" + chart(r["samples"].toArray()) +
            "<p class='legend'>Grey: target · Purple: actual position · Green: control "
            "measurement</p><h3>Fault detection</h3><table><tr><th>Fault</th><th>Sensor "
            "(1–3)</th><th>Injection (s)</th><th>Detection (s)</th><th>Latency (s)</th></tr>";
    for (const auto &value : r["detections"].toArray())
    {
        auto d = value.toObject();
        html += "<tr><td>" + d["fault"].toString().toHtmlEscaped() + "</td><td>" +
                (d["fault"].toString().startsWith("sensor") ? QString::number(d["sensor"].toInt() + 1)
                                                            : QString("—")) +
                "</td><td>" + number(d["injected_s"]) + "</td><td>" + number(d["detected_s"]) + "</td><td>" +
                number(d["latency_s"]) + "</td></tr>";
    }
    html +=
        "</table><p>An empty latency means the fault was not detected during this run. "
        "Timeouts use the original message age, rather than "
        "arrival time.</p><h3>Timeline</h3><table><tr><th>Time (s)</th><th>Module</th><th>Event</th></tr>";
    for (const auto &value : r["events"].toArray())
    {
        auto e = value.toObject();
        html += "<tr><td>" + number(e["time"]) + "</td><td>" + e["source"].toString().toHtmlEscaped() +
                "</td><td>" + e["text"].toString().toHtmlEscaped() + "</td></tr>";
    }
    html += "</table><details><summary>Configuration and repeatable timeline</summary><pre>" +
            QString::fromUtf8(QJsonDocument(scenario).toJson()).toHtmlEscaped() + "</pre></details>";
    return html;
}
QString csv(const QJsonObject &r, const QString &run)
{
    QString text;
    for (const auto &v : r["samples"].toArray())
    {
        auto s = v.toObject();
        const auto readings = s["readings"].toArray();
        text += run + "," + number(s["time"], 5) + "," + number(s["target"], 5) + "," +
                number(s["position"], 5) + "," + number(s["fused"], 5).replace("—", "") + "," +
                number(s["motor"], 5) + "," + s["owner"].toString();
        for (const auto &value : readings)
        {
            auto sensor = value.toObject();
            text += "," + number(sensor["value"], 5).replace("—", "") + "," + sensor["status"].toString();
        }
        text += "\n";
    }
    return text;
}
} // namespace
namespace reporting
{
bool write(const QJsonObject &r, const QString &path, QString *error)
{
    const bool comparison = r.contains("a");
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath()))
    {
        if (error)
            *error = "Cannot create directory";
        return false;
    }
    const QString base = info.absolutePath() + "/" + info.completeBaseName();
    QString html =
        "<!doctype html><html lang='en'><meta charset='utf-8'><title>SLX ResilienceLab · "
        "Report</title><style>body{font:15px "
        "system-ui,sans-serif;color:#18203b;background:#f3f5fb;margin:0}main{max-width:1040px;margin:32px "
        "auto;background:white;padding:40px;border-radius:16px}h1{color:#3436dc}h2{margin-top:32px}table{"
        "border-collapse:collapse;width:100%;margin:16px "
        "0}td,th{text-align:left;padding:10px;border-bottom:1px solid "
        "#dfe4f0}th{background:#f3f5fb;font-size:12px}p,.legend{color:#69738c;line-height:1.6}pre{overflow:"
        "auto;background:#f3f5fb;padding:16px}svg{width:100%}@media "
        "print{body{background:white}main{margin:0;padding:16px}details{display:none}}</style><main><h1>SLX "
        "ResilienceLab</h1><p>Silicon LogiX · Software simulation evidence · core "
        "0.2.0</p><p>Motorized valve, three virtual sensors, primary and backup C++ controllers. This "
        "demonstrator has no safety certification and does not replace hardware testing.</p>";
    QString rows = "run,time_s,target_pct,actual_pct,fused_pct,motor,owner,s1_pct,s1_status,s2_pct,s2_status,"
                   "s3_pct,s3_status\n";
    if (comparison)
    {
        html += "<p>Complete A/B comparison using the same seed and fault timeline.</p>";
        html += section(r["a"].toObject(), "A · Reference control") +
                section(r["b"].toObject(), "B · Control alternative");
        rows += csv(r["a"].toObject(), "A") + csv(r["b"].toObject(), "B");
    }
    else
    {
        html += section(r, "Run results");
        rows += csv(r, "A");
    }
    if (r.contains("desktop_execution"))
    {
        const auto clock = r["desktop_execution"].toObject();
        html += "<h3>PC execution</h3><p>Desktop soft realtime synchronization. Active PC time: " +
                number(clock["wall_elapsed_s"]) + " s; rate at export: " + number(clock["rate"]) +
                "×; peak observed synchronization lag: " + number(clock["peak_lag_ms"]) +
                " ms. Simulation retains fixed steps; Windows/Qt provide no guaranteed "
                "hard realtime.</p>";
    }
    html += "<p>The accompanying JSON includes configuration, samples, events and sent messages with "
            "destination, timestamp, sequence number and transport outcome.</p></main></html>";
    return save(base + ".json", QJsonDocument(r).toJson(), error) &&
           save(base + ".csv", rows.toUtf8(), error) && save(path, html.toUtf8(), error);
}
} // namespace reporting
