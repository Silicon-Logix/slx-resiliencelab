// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Executed demonstration acceptance criteria and campaign evidence.

#pragma once
#include <QJsonObject>
#include <QString>
namespace campaign
{
// Fixed demonstrator acceptance criteria, not product safety requirements.
QJsonObject execute(const QString &plugin = {}, const QString &version = "robust-v1",
                    const QString &scenarioDirectory = {});
bool write(const QJsonObject &result, const QString &htmlPath, QString *error);
} // namespace campaign
