// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop IntroPage component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    signal demoRequested(int scenario)
    signal labRequested()
    signal integrationRequested()
    contentWidth: availableWidth
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ColumnLayout {
        width: root.availableWidth
        spacing: 16
        Rectangle {
            Layout.fillWidth: true; implicitHeight: 185; radius: 14; color: "#111832"
            Column {
                x: 24; y: 22; width: parent.width-205; spacing: 12
                Text { text: "A COMPLETE SOFTWARE TEST BENCH"; color: "#a5b4fc"; font.pixelSize: 10; font.letterSpacing: 1.4 }
                Text { text: "Challenge the control,\nbefore testing the real product."; color: "white"; font.pixelSize: 27; font.weight: Font.DemiBold }
                Text { width: parent.width; text: "Simulate the valve, execute C++ control and inject a fault. Observe whether motion continues, transfers to backup or stops: every decision leaves evidence in the report."; wrapMode: Text.WordWrap; color: "#bec8e1"; font.pixelSize: 12; lineHeight: 1.4 }
            }
            Image { anchors.right: parent.right; anchors.rightMargin: 25; anchors.verticalCenter: parent.verticalCenter; width: 125; height: 125; source: "qrc:/assets/app-icon.svg" }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 6
            Text { text: "Choose a question and test the answer"; color: "#18203b"; font.pixelSize: 19; font.weight: Font.DemiBold }
            Text { text: "Each test lasts 12 simulated seconds. Faults are applied during motion."; color: "#69738c"; font.pixelSize: 11 }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 14
            Repeater {
                model: [
                    {title:"One sensor freezes",symbol:"S1",color:"#9b691b",question:"Can control continue?",fault:"One reading freezes while the valve moves.",expected:"It must isolate the disagreeing sensor and use the other two."},
                    {title:"Primary controller stops",symbol:"A → B",color:"#3436dc",question:"Who takes control?",fault:"The primary stops sending commands and heartbeat.",expected:"Backup must take over after timeout, with only one controller authorized."},
                    {title:"Two measurements disappear",symbol:"STOP",color:"#b4484e",question:"Does the motor stop?",fault:"Two sensors stop sending data. One reading is insufficient.",expected:"Control must detect insufficient trustworthy measurements and stop motion."}
                ]
                delegate: Rectangle {
                    required property var modelData; required property int index
                    Layout.fillWidth: true; Layout.preferredWidth: 300; implicitHeight: 240; radius: 12; color: "white"; border.color: "#dfe4f0"
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 18; spacing: 10
                        Text { text: modelData.symbol; color: modelData.color; font.pixelSize: 20; font.weight: Font.DemiBold }
                        Text { text: modelData.title; color: "#18203b"; font.pixelSize: 16; font.weight: Font.DemiBold }
                        Text { text: modelData.question; color: modelData.color; font.pixelSize: 12 }
                        Text { Layout.fillWidth: true; text: modelData.fault; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                        Text { Layout.fillWidth: true; text: modelData.expected; wrapMode: Text.WordWrap; color: "#28324e"; font.pixelSize: 11 }
                        Item { Layout.fillHeight: true }
                        ActionButton { text: "Run this test"; primary: true; Layout.fillWidth: true; onClicked: root.demoRequested(index) }
                    }
                }
            }
        }
        Panel {
            title: "How to read the laboratory"; Layout.fillWidth: true; implicitHeight: 112
            RowLayout {
                anchors.fill: parent; spacing: 18
                Repeater {
                    model: [
                        {title:"1 · Target",text:"The operator requests an opening, for example 80%."},
                        {title:"2 · Actual position",text:"The valve moves according to the applied motor command."},
                        {title:"3 · Sensors",text:"Readings can differ from actual position."},
                        {title:"4 · Decision",text:"Control uses received measurements and messages to decide motion."}
                    ]
                    delegate: ColumnLayout {
                        required property var modelData; Layout.fillWidth: true; Layout.preferredWidth: 220; spacing: 7
                        Text { text: modelData.title; color: "#3436dc"; font.pixelSize: 12; font.weight: Font.DemiBold }
                        Text { Layout.fillWidth: true; text: modelData.text; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                    }
                }
            }
        }
        Panel {
            title: "What a customer can commission"; Layout.fillWidth: true; implicitHeight: 140
            ColumnLayout {
                anchors.fill: parent; spacing: 9
                Text { Layout.fillWidth: true; text: "A test environment for your product and control: plant model, protocols, repeatable fault sequences and reports. The valve and SLX control demonstrate the approach; a customer DLL can replace the motor algorithm."; wrapMode: Text.WordWrap; color: "#28324e"; font.pixelSize: 12 }
                RowLayout {
                    ActionButton { text: "Control integration"; onClicked: root.integrationRequested() }
                    ActionButton { text: "Open laboratory"; onClicked: root.labRequested() }
                    Text { text: "Software demonstrator · no safety certification"; color: "#8790a5"; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight }
                }
            }
        }
    }
}
