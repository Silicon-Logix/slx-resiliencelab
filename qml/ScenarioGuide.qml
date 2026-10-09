// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ScenarioGuide component.

import QtQuick
import QtQuick.Layouts
Rectangle {
    id: root
    property var guide: ({})
    radius: 10; color: "white"; border.color: "#dfe4f0"
    implicitHeight: Math.max(102, explanation.implicitHeight+20)
    ColumnLayout {
        id: explanation
        anchors.fill: parent; anchors.margins: 10; spacing: 5
        RowLayout {
            Layout.fillWidth: true
            Text { text: root.guide.question || ""; font.pixelSize: 13; font.weight: Font.DemiBold; color: "#18203b"; Layout.fillWidth: true; elide: Text.ElideRight }
            Rectangle {
                implicitWidth: phase.implicitWidth+20; implicitHeight: 25; radius: 6
                color: root.guide.status==="success" ? "#e5f6f0" : root.guide.status==="attention" ? "#fff1df" : "#eef0ff"
                Text { id: phase; anchors.centerIn: parent; text: root.guide.phase || ""; color: root.guide.status==="success" ? "#167565" : root.guide.status==="attention" ? "#9b691b" : "#3436dc"; font.pixelSize: 10 }
            }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 18
            ColumnLayout {
                Layout.fillWidth: true; Layout.preferredWidth: 350; spacing: 4
                Text { Layout.fillWidth: true; text: "FAULT · "+(root.guide.fault || ""); wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.preferredWidth: 350; spacing: 4
                Text { Layout.fillWidth: true; text: "EXPECTED · "+(root.guide.expected || ""); wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
            }
        }
        Text { Layout.fillWidth: true; text: root.guide.observed || ""; wrapMode: Text.WordWrap; color: "#28324e"; font.pixelSize: 11 }
    }
}
