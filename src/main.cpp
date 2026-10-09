// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop startup and batch/capture entry points.

#include "benchapi.h"
#include "campaign.h"
#include "labbridge.h"
#include "report.h"
#include "valveview.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#include <algorithm>
#include <cmath>
namespace
{
QString option(const QStringList &args, const QString &name, const QString &fallback = {})
{
    const int i = args.indexOf(name);
    return i >= 0 && i + 1 < args.size() ? args[i + 1] : fallback;
}
bool scenario(slx::Session &session, const QString &value, QString *error)
{
    bool integer = false;
    const int index = value.toInt(&integer);
    if (integer)
        return session.load(slx::Session::preset(index), error);
    QFile f(value);
    if (!f.open(QIODevice::ReadOnly))
    {
        *error = f.errorString();
        return false;
    }
    QJsonParseError jsonError;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        *error = jsonError.errorString();
        return false;
    }
    return session.load(doc.object(), error);
}
} // namespace
int main(int argc, char **argv)
{
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args.append(QString::fromLocal8Bit(argv[i]));
    if (args.contains("--run") || args.contains("--suite"))
    {
        QCoreApplication app(argc, argv);
        if (args.contains("--suite"))
        {
            const auto result =
                campaign::execute(option(args, "--plugin"), option(args, "--control-version", "robust-v1"),
                                  option(args, "--scenario-dir"));
            QString error;
            if (!campaign::write(result, option(args, "--report", "campaign.html"), &error))
            {
                QTextStream(stderr) << error << '\n';
                return 3;
            }
            QTextStream(stdout) << result["passed"].toInt() << " / 3 PASS\n";
            return result["all_passed"].toBool() ? 0 : 1;
        }
        slx::Session session;
        QString error;
        if (!scenario(session, option(args, "--scenario", "0"), &error))
        {
            QTextStream(stderr) << error << '\n';
            return 2;
        }
        QJsonObject result;
        if (args.contains("--compare"))
            result = session.compare(option(args, "--compare", "cautious-v2"));
        else
        {
            session.runToEnd();
            result = session.result();
        }
        if (result.contains("error"))
        {
            QTextStream(stderr) << result["error"].toString() << '\n';
            return 2;
        }
        if (!reporting::write(result, option(args, "--report", "report.html"), &error))
        {
            QTextStream(stderr) << error << '\n';
            return 3;
        }
        QTextStream(stdout) << QJsonDocument(result.contains("a")
                                                 ? result["a"].toObject()["metrics"].toObject()
                                                 : result["metrics"].toObject())
                                   .toJson(QJsonDocument::Compact)
                            << '\n';
        return 0;
    }
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);
    if (args.contains("--benchmark-render"))
    {
        format.setSwapInterval(0);
        qputenv("QSG_RENDER_LOOP", "basic");
    }
    QSurfaceFormat::setDefaultFormat(format);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickStyle::setStyle("Basic");
    QGuiApplication app(argc, argv);
    app.setApplicationName("SLX ResilienceLab");
    app.setOrganizationName("Silicon LogiX");
    app.setApplicationVersion(SLX_APP_VERSION);
    app.setWindowIcon(QIcon(":/assets/app-icon.ico"));
    const bool smoke = args.contains("--smoke-test");
    LabBridge bridge;
    BenchApi api(&bridge);
    QString error;
    if (!scenario(bridge.session(), option(args, "--scenario", "0"), &error))
    {
        QTextStream(stderr) << error << '\n';
        return 2;
    }
    if (args.contains("--capture-time"))
    {
        const double time = option(args, "--capture-time").toDouble();
        while (bridge.session().snapshot()["time"].toDouble() < time && !bridge.session().finished())
            bridge.session().step();
    }
    qmlRegisterType<ValveView>("SLX.Resilience", 1, 0, "ValveView");
    bridge.synchronize();
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("lab", &bridge);
    engine.rootContext()->setContextProperty("benchApi", &api);
    engine.rootContext()->setContextProperty("smokeMode", smoke);
    bool qmlErrors = false;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [&](const QList<QQmlError> &) { qmlErrors = true; });
    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 4;
    auto *rootWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (args.contains("--api"))
        if (!api.startLocal(option(args, "--api-port", "47400").toInt()))
        {
            QTextStream(stderr) << api.status()["error"].toString() << '\n';
            return 8;
        }
    if (args.contains("--serial"))
        if (!api.openSerial(option(args, "--serial"), option(args, "--baud", "115200").toInt()))
        {
            QTextStream(stderr) << api.status()["error"].toString() << '\n';
            return 8;
        }
    if (args.contains("--live"))
    {
        bridge.startLive();
        rootWindow->setProperty("page", 0);
    }
    const int capturePage = option(args, "--capture-page", "0").toInt();
    if (args.contains("--capture-page") || args.contains("--capture"))
        rootWindow->setProperty("page", std::clamp(capturePage, 0, 5));
    if (capturePage == 2)
        bridge.compareVersions();
    if (args.contains("--capture-campaign"))
        bridge.runCampaign();
    if (args.contains("--about") || args.contains("--license") || args.contains("--notices"))
        QMetaObject::invokeMethod(rootWindow, "showInformation",
                                  Q_ARG(QVariant, QVariant(args.contains("--license")   ? "license"
                                                           : args.contains("--notices") ? "notices"
                                                                                        : "about")));
    if (args.contains("--normal"))
        if (auto *valve = rootWindow->findChild<ValveView *>("valveViewport"))
            valve->setCutaway(false);
    if (args.contains("--compact"))
        rootWindow->resize(1180, 780);
    if (smoke)
    {
        ValveView asset;
        const bool valid = asset.assetError().isEmpty() && !qmlErrors;
        return valid ? 0 : 5;
    }
    const QString capture = option(args, "--capture");
    QElapsedTimer benchmarkClock;
    QTimer cameraTimer;
    QVector<double> frameIntervals;
    QMutex frameMutex;
    QMetaObject::Connection frameConnection;
    double previousFrame = 0;
    if (args.contains("--benchmark-render"))
    {
        rootWindow->setProperty("page", 0);
        auto *valve = rootWindow->findChild<ValveView *>("valveViewport");
        if (!valve)
            return 7;
        benchmarkClock.start();
        if (args.contains("--benchmark-running"))
            bridge.toggleRun();
        frameConnection = QObject::connect(
            rootWindow, &QQuickWindow::afterRendering, &app,
            [&]
            {
                QMutexLocker lock(&frameMutex);
                const double now = benchmarkClock.nsecsElapsed() / 1000000.0;
                if (previousFrame >= 1000)
                    frameIntervals.append(now - previousFrame);
                previousFrame = now;
            },
            Qt::DirectConnection);
        cameraTimer.setInterval(8);
        cameraTimer.setTimerType(Qt::PreciseTimer);
        QObject::connect(&cameraTimer, &QTimer::timeout, &app,
                         [&, valve]
                         {
                             const double seconds = benchmarkClock.elapsed() / 1000.0;
                             valve->setAzimuth(34 + seconds * 35);
                             valve->setElevation(23 + std::sin(seconds * 2) * 12);
                             // Explicit render/readback also exercises the desktop when the native test
                             // window is hidden.
                             rootWindow->grabWindow();
                         });
        cameraTimer.start();
        QTimer::singleShot(
            6500, &app,
            [&, valve]
            {
                cameraTimer.stop();
                QObject::disconnect(frameConnection);
                QMutexLocker lock(&frameMutex);
                auto sorted = frameIntervals;
                std::sort(sorted.begin(), sorted.end());
                double total = 0;
                int longFrames = 0;
                for (double interval : sorted)
                {
                    total += interval;
                    if (interval > 50)
                        ++longFrames;
                }
                QJsonObject result{
                    {"mode", "desktop render and readback"},
                    {"swap_interval", 0},
                    {"window_width", rootWindow->width()},
                    {"window_height", rootWindow->height()},
                    {"valve_draw_calls",
                     valve->cutaway() ? valve->assets()->cutaway.size() : valve->assets()->normal.size()},
                    {"frames", sorted.size()},
                    {"fps", total > 0 ? sorted.size() * 1000 / total : 0},
                    {"median_ms", sorted.isEmpty() ? 0 : sorted[sorted.size() / 2]},
                    {"p95_ms", sorted.isEmpty()
                                   ? 0
                                   : sorted[std::min(sorted.size() - 1, qsizetype(sorted.size() * .95))]},
                    {"max_ms", sorted.isEmpty() ? 0 : sorted.last()},
                    {"frames_over_50_ms", longFrames},
                    {"simulation_time_s", bridge.session().snapshot()["time"].toDouble()}};
                result["desktop_execution"] = QJsonObject::fromVariantMap(bridge.clock());
                QFile output(option(args, "--benchmark-output", "render-benchmark.json"));
                const bool ok =
                    output.open(QIODevice::WriteOnly) && output.write(QJsonDocument(result).toJson()) > 0;
                app.exit(ok && !qmlErrors && !sorted.isEmpty() ? 0 : 7);
            });
    }
    if (!capture.isEmpty())
        QTimer::singleShot(1800, &app,
                           [&]
                           {
                               auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
                               const auto image = window->grabWindow();
                               const bool ok = !image.isNull() && image.save(capture);
                               app.exit(ok && !qmlErrors ? 0 : 6);
                           });
    const int result = app.exec();
    cameraTimer.stop();
    QObject::disconnect(frameConnection);
    return result;
}
