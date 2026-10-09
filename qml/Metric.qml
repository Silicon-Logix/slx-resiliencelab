// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop Metric component.

import QtQuick
import QtQuick.Layouts
Rectangle {
    id: root
    property string label: ""
    property string value: ""
    property string unit: ""
    property string detail: ""
    property color accent: "#3436dc"
    implicitHeight: 90
    radius: 10
    color: "white"
    border.color: "#dfe4f0"
    Column { x: 16; y: 13; spacing: 4
        Text { text: root.label; color: "#69738c"; font.pixelSize: 11 }
        Row { spacing: 6
            Text { text: root.value; color: root.accent; font.pixelSize: 27; font.weight: Font.DemiBold }
            Text { text: root.unit; color: "#8790a5"; font.pixelSize: 13; anchors.bottom: parent.bottom; anchors.bottomMargin: 5 }
        }
        Text { text: root.detail; color: "#69738c"; font.pixelSize: 10 }
    }
}
