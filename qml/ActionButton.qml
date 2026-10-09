// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ActionButton component.

import QtQuick
import QtQuick.Controls
Button {
    id: control
    property bool primary: false
    property bool danger: false
    implicitHeight: 36
    implicitWidth: Math.max(84, contentItem.implicitWidth + 28)
    font.pixelSize: 12
    font.weight: Font.DemiBold
    hoverEnabled: true
    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? "#9da5b7" : control.primary ? "white" : control.danger ? "#b4484e" : "#28324e"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        radius: 7
        color: !control.enabled ? "#eef0f5" : control.primary ? (control.down ? "#292bb0" : control.hovered ? "#4547ed" : "#3436dc") : control.down ? "#e5e8f3" : control.hovered ? "#f0f2fa" : "#ffffff"
        border.color: control.activeFocus ? "#3436dc" : control.primary ? "transparent" : "#dfe4f0"
    }
}
