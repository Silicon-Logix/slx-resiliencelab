// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Serial/local TCP bench transport and validated request handling.

#include "benchapi.h"
#include "labbridge.h"
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QSerialPortInfo>
#include <QTcpSocket>
#include <cmath>
#include <memory>
BenchApi::BenchApi(LabBridge *lab, QObject *parent) : QObject(parent), m_lab(lab)
{
    refreshPorts();
    attach(&m_serial);
    connect(&m_serial, &QSerialPort::errorOccurred, this,
            [this](QSerialPort::SerialPortError error)
            {
                if (error != QSerialPort::NoError)
                {
                    m_error = m_serial.errorString();
                    if (error == QSerialPort::ResourceError)
                        m_serial.close();
                    emit statusChanged();
                }
            });
    connect(&m_server, &QTcpServer::newConnection, this,
            [this]
            {
                while (auto *socket = m_server.nextPendingConnection())
                {
                    if (m_clients >= 8)
                    {
                        socket->abort();
                        socket->deleteLater();
                        continue;
                    }
                    ++m_clients;
                    socket->setReadBufferSize(32768);
                    attach(socket);
                    connect(socket, &QTcpSocket::disconnected, this,
                            [this, socket]
                            {
                                --m_clients;
                                socket->deleteLater();
                                emit statusChanged();
                            });
                }
                emit statusChanged();
            });
}
QVariantMap BenchApi::status() const
{
    return {{"local", m_server.isListening()},
            {"port", m_server.serverPort()},
            {"serial", m_serial.isOpen()},
            {"serial_port", m_serial.portName()},
            {"baud", m_serial.baudRate()},
            {"clients", m_clients},
            {"received", QVariant::fromValue(m_received)},
            {"rejected", QVariant::fromValue(m_rejected)},
            {"error", m_error},
            {"protocol", "slx.bench.v1"}};
}
void BenchApi::refreshPorts()
{
    m_ports.clear();
    for (const auto &port : QSerialPortInfo::availablePorts())
        m_ports.append(port.portName());
    m_ports.sort();
    emit portsChanged();
}
bool BenchApi::startLocal(int port)
{
    if (port < 0 || port > 65535)
    {
        m_error = "Port must be between 0 and 65535";
        emit statusChanged();
        return false;
    }
    stopLocal();
    const bool ok = m_server.listen(QHostAddress::LocalHost, quint16(port));
    m_error = ok ? QString() : m_server.errorString();
    emit statusChanged();
    return ok;
}
void BenchApi::stopLocal()
{
    m_server.close();
    for (auto *socket : m_server.findChildren<QTcpSocket *>())
        socket->abort();
    emit statusChanged();
}
bool BenchApi::openSerial(QString port, int baud)
{
    if (port.trimmed().isEmpty() || (baud != 9600 && baud != 57600 && baud != 115200))
    {
        m_error = "Select a port and a supported baud rate";
        emit statusChanged();
        return false;
    }
    m_serial.close();
    m_serial.setPortName(port.trimmed());
    m_serial.setBaudRate(baud);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);
    m_serial.setReadBufferSize(32768);
    const bool ok = m_serial.open(QIODevice::ReadWrite);
    m_error = ok ? QString() : m_serial.errorString();
    emit statusChanged();
    return ok;
}
void BenchApi::closeSerial()
{
    m_serial.close();
    emit statusChanged();
}
void BenchApi::readLocal()
{
    if (!m_server.isListening())
    {
        m_preview = "Enable the local API first";
        emit statusChanged();
        return;
    }
    auto *socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this,
            [socket] { socket->write("{\"id\":1,\"op\":\"read\"}\n"); });
    connect(socket, &QTcpSocket::readyRead, this,
            [this, socket]
            {
                if (!socket->canReadLine())
                    return;
                const auto data = QJsonDocument::fromJson(socket->readLine()).object()["data"].toObject();
                m_preview =
                    QString("API read · t=%1 s · target=%2% · actual=%3% · fused=%4% · motor=%5")
                        .arg(data["time"].toDouble(), 0, 'f', 2)
                        .arg(data["target"].toDouble(), 0, 'f', 1)
                        .arg(data["position"].toDouble(), 0, 'f', 1)
                        .arg(data["fused"].isDouble() ? QString::number(data["fused"].toDouble(), 'f', 1)
                                                      : "unavailable")
                        .arg(data["motor"].toDouble(), 0, 'f', 2);
                emit statusChanged();
                socket->disconnectFromHost();
            });
    connect(socket, &QTcpSocket::errorOccurred, this,
            [this, socket]
            {
                m_preview = socket->errorString();
                emit statusChanged();
                socket->deleteLater();
            });
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    QTimer::singleShot(3000, socket,
                       [socket]
                       {
                           socket->abort();
                           socket->deleteLater();
                       });
    socket->connectToHost(QHostAddress::LocalHost, m_server.serverPort());
}
void BenchApi::attach(QIODevice *device)
{
    auto pending = std::make_shared<QByteArray>();
    connect(device, &QIODevice::aboutToClose, this, [pending] { pending->clear(); });
    connect(device, &QIODevice::readyRead, this,
            [this, device, pending]
            {
                pending->append(device->readAll());
                if (pending->size() > 32768 || device->bytesToWrite() > 65536)
                {
                    ++m_rejected;
                    pending->clear();
                    device->close();
                    emit statusChanged();
                    return;
                }
                int frames = 0;
                while (pending->contains('\n'))
                {
                    if (++frames > 32)
                    {
                        ++m_rejected;
                        pending->clear();
                        device->close();
                        break;
                    }
                    const int end = pending->indexOf('\n');
                    const auto frame = pending->left(end).trimmed();
                    pending->remove(0, end + 1);
                    QJsonParseError error;
                    const auto json = QJsonDocument::fromJson(frame, &error);
                    QJsonObject reply;
                    if (frame.size() > 16384 || error.error != QJsonParseError::NoError || !json.isObject())
                    {
                        ++m_received;
                        ++m_rejected;
                        reply = {{"ok", false}, {"error", "Invalid JSON object or frame exceeds 16 KiB"}};
                    }
                    else
                        reply = dispatch(json.object());
                    const auto bytes = QJsonDocument(reply).toJson(QJsonDocument::Compact) + '\n';
                    if (device->write(bytes) != bytes.size())
                    {
                        m_error = "Transport write failed";
                        device->close();
                        break;
                    }
                }
                emit statusChanged();
            });
}
QJsonObject BenchApi::dispatch(const QJsonObject &request)
{
    ++m_received;
    auto fail = [&](const QString &message)
    {
        ++m_rejected;
        emit statusChanged();
        return QJsonObject{
            {"schema", "slx.bench.v1"}, {"id", request["id"]}, {"ok", false}, {"error", message}};
    };
    const auto op = request["op"].toString();
    QJsonObject data;
    if (op == "read")
    {
        data = QJsonObject::fromVariantMap(m_lab->state());
        data["running"] = m_lab->running();
        data["clock"] = QJsonObject::fromVariantMap(m_lab->clock());
    }
    else if (op == "measurements")
    {
        const auto state = m_lab->state();
        const QString controller = request["controller"].toString(state["owner"].toString());
        if (controller != "main" && controller != "backup")
            return fail("Controller must be main or backup");
        const auto inputs = state[controller].toMap();
        // Explicit allowlist: firmware-facing reads contain no plant truth or fault configuration.
        data = {{"time_s", state["time"].toDouble()},
                {"controller", controller},
                {"controller_alive", state[controller == "main" ? "mainAlive" : "backupAlive"].toBool()},
                {"requested_percent", QJsonValue::fromVariant(inputs["requested"])},
                {"readings", QJsonArray::fromVariantList(inputs["readings"].toList())},
                {"fused_percent", QJsonValue::fromVariant(inputs["fused"])},
                {"quorum", inputs["quorum"].toBool()},
                {"granted", inputs["granted"].toBool()},
                {"motor_normalized", inputs["motor"].toDouble()},
                {"stopped", inputs["stopped"].toBool()},
                {"diagnostic", inputs["stopReason"].toString()}};
    }
    else if (op == "live")
    {
        m_lab->startLive();
        data["running"] = m_lab->running();
    }
    else if (op == "pause")
    {
        if (m_lab->running())
            m_lab->toggleRun();
        data["running"] = false;
    }
    else if (op == "reset")
    {
        m_lab->reset();
        data["running"] = false;
    }
    else if (op == "run")
    {
        if (m_lab->session().finished())
            return fail("Run finished; reset or start a new live bench");
        if (!m_lab->running())
            m_lab->toggleRun();
        data["running"] = m_lab->running();
    }
    else if (op == "step")
    {
        if (m_lab->running() || m_lab->session().finished())
            return fail("Step requires a paused, unfinished run");
        m_lab->step();
        data["time_s"] = m_lab->state()["time"].toDouble();
    }
    else if (op == "target" || op == "fault")
    {
        if (m_lab->session().finished())
            return fail("Run finished; reset or start a new live bench");
        const auto number = request["value"];
        if (op == "target")
        {
            if (!number.isDouble() || !std::isfinite(number.toDouble()) || number.toDouble() < 0 ||
                number.toDouble() > 100)
                return fail("Target must be a finite number between 0 and 100");
            m_lab->setTarget(number.toDouble());
        }
        else
        {
            const auto kind = request["kind"].toString();
            if (!slx::Session::faultNames().contains(kind))
                return fail("Unknown fault kind");
            if (request.contains("value") && (!number.isDouble() || !std::isfinite(number.toDouble())))
                return fail("Fault value must be finite and numeric");
            const double value = number.toDouble();
            if (kind == "communication" && (value < 0 || value > 1))
                return fail("Loss must be between 0 and 1");
            QJsonArray sensors = request["sensors"].toArray();
            if (request.contains("sensors") &&
                (!request["sensors"].isArray() || sensors.isEmpty() || sensors.size() > 3))
                return fail("Sensors must be an array of one to three indices");
            if (sensors.isEmpty())
                sensors.append(0);
            int mask = 0;
            for (const auto &sensor : sensors)
            {
                if (!sensor.isDouble() || sensor.toDouble() != sensor.toInt(-1) || sensor.toInt() < 0 ||
                    sensor.toInt() > 2)
                    return fail("Sensor indices must be integers 0, 1 or 2");
                if (mask & (1 << sensor.toInt()))
                    return fail("Duplicate sensor index");
                mask |= 1 << sensor.toInt();
            }
            for (int channel = 0; channel < 3; ++channel)
                if (mask & (1 << channel))
                {
                    m_lab->inject(kind, channel, value);
                    if (!kind.startsWith("sensor_"))
                        break;
                }
        }
        data = {{"scheduled", true}, {"applies_at_s", m_lab->state()["time"].toDouble()}};
    }
    else
        return fail("Unknown operation");
    emit statusChanged();
    return {{"schema", "slx.bench.v1"}, {"id", request["id"]}, {"ok", true}, {"data", data}};
}
