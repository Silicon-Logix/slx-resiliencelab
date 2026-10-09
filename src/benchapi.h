// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Serial/local TCP bench transport and validated request handling.

#pragma once
#include <QJsonObject>
#include <QObject>
#include <QSerialPort>
#include <QTcpServer>
#include <QVariantMap>
class LabBridge;

// Newline-delimited JSON bench API. Serial and loopback TCP share one contract.
// Bench commands use Session messages; this adapter never writes plant position.
class BenchApi final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString preview READ preview NOTIFY statusChanged)
  public:
    explicit BenchApi(LabBridge *lab, QObject *parent = nullptr);
    QVariantMap status() const;
    QStringList ports() const
    {
        return m_ports;
    }
    QString preview() const
    {
        return m_preview;
    }
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE bool startLocal(int port = 47400);
    Q_INVOKABLE void stopLocal();
    Q_INVOKABLE bool openSerial(QString port, int baud = 115200);
    Q_INVOKABLE void closeSerial();
    Q_INVOKABLE void readLocal();
    QJsonObject dispatch(const QJsonObject &request);
    // Attach any duplex stream for software integration or transport testing.
    void attach(QIODevice *device);
  signals:
    void statusChanged();
    void portsChanged();

  private:
    LabBridge *m_lab;
    QTcpServer m_server;
    QSerialPort m_serial;
    QStringList m_ports;
    QString m_error, m_preview;
    quint64 m_received = 0, m_rejected = 0;
    int m_clients = 0;
};
