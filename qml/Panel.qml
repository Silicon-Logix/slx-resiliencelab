// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop Panel component.

import QtQuick
Rectangle {
    id: root
    property string title: ""
    default property alias content: body.data
    radius: 12
    color: "white"
    border.color: "#dfe4f0"
    Text { x: 18; y: 17; text: root.title; color: "#18203b"; font.pixelSize: 13; font.weight: Font.DemiBold }
    Item { id: body; anchors { left: parent.left; right: parent.right; top: parent.top; bottom: parent.bottom; leftMargin: 16; rightMargin: 16; topMargin: root.title.length ? 48 : 16; bottomMargin: 16 } }
}
