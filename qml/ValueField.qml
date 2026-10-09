// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ValueField component.

import QtQuick
import QtQuick.Controls
TextField {
    id: control
    implicitHeight: 36
    font.pixelSize: 12
    color: "#28324e"
    leftPadding: 10
    background: Rectangle { radius: 6; color: "white"; border.color: control.activeFocus ? "#3436dc" : "#dfe4f0" }
}
