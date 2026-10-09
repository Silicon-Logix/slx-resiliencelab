// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ConnectionsPanel component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
    id: root
    signal documentRequested()
    title: "Live bench API / USB serial and PC automation"
    implicitHeight: 236
    property var apiStatus: typeof benchApi !== "undefined" ? benchApi.status : ({})
    ColumnLayout {
        anchors.fill: parent; spacing: 10
        Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11; text: "AVAILABLE · Read actual position for bench supervision, or read controller measurements without plant truth. Set an opening, run/pause and inject faults. USB serial uses JSON lines, 8N1; local TCP uses the same protocol. The motor algorithm still runs in SLX or your DLL." }
        RowLayout {
            ChoiceBox { id: ports; Layout.preferredWidth: 105; model: typeof benchApi !== "undefined" ? benchApi.ports : []; onActivated: port.text=currentText }
            ValueField { id: port; Layout.preferredWidth: 115; placeholderText: "COM3 / ttyACM0" }
            ChoiceBox { id: baud; model: ["115200","57600","9600"]; Layout.preferredWidth: 100 }
            ActionButton { text: "Refresh ports"; onClicked: benchApi.refreshPorts() }
            ActionButton { text: apiStatus.serial ? "Disconnect USB" : "Connect USB"; primary: true; onClicked: apiStatus.serial ? benchApi.closeSerial() : benchApi.openSerial(port.text.trim().length ? port.text : ports.currentText,Number(baud.currentText)) }
            Text { text: apiStatus.serial ? "Connected: "+apiStatus.serial_port : "Serial disconnected"; color: apiStatus.serial ? "#167565" : "#8790a5"; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight }
        }
        RowLayout {
            ActionButton { text: apiStatus.local ? "Disable local API" : "Enable local API"; onClicked: apiStatus.local ? benchApi.stopLocal() : benchApi.startLocal() }
            ActionButton { text: "Read via API"; objectName: "readLocalApi"; enabled: !!apiStatus.local; onClicked: benchApi.readLocal() }
            Text { Layout.fillWidth: true; text: apiStatus.local ? "TCP 127.0.0.1:"+apiStatus.port+" · JSON lines" : "Local API disabled"; color: "#3436dc"; font.pixelSize: 11 }
            ActionButton { text: "Protocol and examples"; onClicked: root.documentRequested() }
        }
        Text { Layout.fillWidth: true; text: typeof benchApi !== "undefined" ? benchApi.preview : ""; color: "#167565"; font.pixelSize: 11; wrapMode: Text.WordWrap }
        Text { Layout.fillWidth: true; text: apiStatus.error || "Frames received: "+(apiStatus.received || 0)+" · rejected: "+(apiStatus.rejected || 0)+". A board needs firmware implementing this contract; this is not a generic firmware emulator."; color: apiStatus.error ? "#b4484e" : "#8790a5"; font.pixelSize: 10; wrapMode: Text.WordWrap }
    }
}
