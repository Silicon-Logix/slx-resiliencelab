// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Standalone simulation evidence export.

#pragma once
#include <QJsonObject>
#include <QString>
namespace reporting
{
bool write(const QJsonObject &result, const QString &htmlPath, QString *error);
}
