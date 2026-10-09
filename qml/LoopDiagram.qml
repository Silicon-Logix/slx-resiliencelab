// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop LoopDiagram component.

import QtQuick
import QtQuick.Layouts
Rectangle {
    id: root
    property bool hardware: false
    implicitHeight: 126; radius: 10; color: "#f3f5fb"
    RowLayout {
        anchors.fill: parent; anchors.margins: 14; spacing: 12
        Rectangle {
            Layout.fillWidth: true; Layout.preferredWidth: 230; Layout.fillHeight: true; radius: 8; color: "white"; border.color: "#dfe4f0"
            Column { anchors.centerIn: parent; spacing: 7
                Text { text: "RESILIENCELAB · PC"; color: "#3436dc"; font.pixelSize: 11; font.weight: Font.Bold }
                Text { text: "Simulated valve and sensors\nFaults and recording"; color: "#69738c"; font.pixelSize: 11; lineHeight: 1.5 }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; Layout.preferredWidth: 260; spacing: 12
            Text { Layout.fillWidth: true; text: "measurements + requested opening  →"; horizontalAlignment: Text.AlignHCenter; color: "#167565"; font.pixelSize: 12; wrapMode: Text.WordWrap }
            Text { Layout.fillWidth: true; text: root.hardware ? "←  motor command + heartbeat" : "←  motor command"; horizontalAlignment: Text.AlignHCenter; color: "#3436dc"; font.pixelSize: 12; wrapMode: Text.WordWrap }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.preferredWidth: 230; Layout.fillHeight: true; radius: 8; color: "#111832"
            Column { anchors.centerIn: parent; spacing: 7
                Text { text: root.hardware ? "CUSTOMER BOARD" : "CUSTOMER CONTROL"; color: "white"; font.pixelSize: 11; font.weight: Font.Bold }
                Text { text: root.hardware ? "Firmware on the board\nCustom I/O adapter" : "C/C++ DLL running on the PC\nC API v1 available"; color: "#bec8e1"; font.pixelSize: 11; lineHeight: 1.5 }
            }
        }
    }
}
