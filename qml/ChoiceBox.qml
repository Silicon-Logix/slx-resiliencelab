// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ChoiceBox component.

import QtQuick
import QtQuick.Controls
ComboBox {
    id: control
    implicitHeight: 36
    font.pixelSize: 11
    background: Rectangle { radius: 7; color: control.enabled ? "white" : "#f0f2f7"; border.color: control.activeFocus ? "#3436dc" : "#dfe4f0" }
    contentItem: Text { leftPadding: 12; rightPadding: 30; text: control.displayText; font: control.font; color: control.enabled ? "#28324e" : "#8790a5"; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
    indicator: Text { x: control.width-24; anchors.verticalCenter: parent.verticalCenter; text: "⌄"; color: "#69738c"; font.pixelSize: 18 }
    delegate: ItemDelegate { width: control.width; text: modelData; font: control.font; highlighted: control.highlightedIndex===index }
}
