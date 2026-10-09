// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop SolutionsPage component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id: root
    signal apiRequested()
    signal briefRequested(var needs)
    contentWidth: availableWidth; clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: contentHeight > availableHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    ColumnLayout {
        width: root.availableWidth; spacing: 16
        Text { Layout.fillWidth: true; text: "From demonstrator to your product's test bench"; font.pixelSize: 23; font.weight: Font.DemiBold; color: "#18203b" }
        Text { Layout.fillWidth: true; text: "The available software demonstrates the method. These cards preview commissioned integrations and describe the deliverables and required product data."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 12 }
        GridLayout {
            Layout.fillWidth: true; columns: 2; columnSpacing: 14; rowSpacing: 14
            Repeater {
                model: [
                    {title:"Board and firmware in the test loop",tag:"CUSTOM PROJECT · HIL",detail:"Simulated measurements to the board, commands and heartbeat to the plant. Adapters for serial, CAN, Ethernet or electrical I/O.",need:"Requires board model, protocol and signal map.",delivery:"Deliverable: I/O adapter, fault sequences, watchdog and measured timing reports."},
                    {title:"Customer product model",tag:"CUSTOM PROJECT · MODEL",detail:"Valves, motors, pumps and other systems: product-specific inertia, friction, load, pressure and faults.",need:"Requires physical characteristics and measurement data for calibration.",delivery:"Deliverable: parametric model checked against agreed reference data."},
                    {title:"Dedicated realtime target",tag:"CUSTOM PROJECT · REALTIME",detail:"Run on a target with I/O and timing designed for the firmware. Frequency depends on model and hardware.",need:"Requires cycle period, allowed jitter and target hardware.",delivery:"Deliverable: scheduler and I/O measured on the target. This preview guarantees no cycle frequency."},
                    {title:"Requirements, coverage and extended campaigns",tag:"CUSTOM PROJECT · VERIFICATION",detail:"Customer criteria, fault and parameter sweeps, build comparisons and a requirement–test–evidence matrix.",need:"Requires behavioral requirements and acceptance conditions.",delivery:"Deliverable: versioned suite, regression reports and CI integration."}
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true; Layout.preferredWidth: 420; implicitHeight: 220; radius: 12; color: "white"; border.color: "#dfe4f0"
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 18; spacing: 10
                        Text { text: modelData.tag; color: "#9b691b"; font.pixelSize: 10; font.weight: Font.DemiBold }
                        Text { Layout.fillWidth: true; text: modelData.title; font.pixelSize: 16; font.weight: Font.DemiBold; color: "#18203b"; wrapMode: Text.WordWrap }
                        Text { Layout.fillWidth: true; text: modelData.detail; font.pixelSize: 11; color: "#69738c"; wrapMode: Text.WordWrap }
                        Text { Layout.fillWidth: true; text: modelData.need; font.pixelSize: 11; color: "#3436dc"; wrapMode: Text.WordWrap }
                        Item { Layout.fillHeight: true }
                        ActionButton { text: "View deliverable"; onClicked: {preview.description=modelData;preview.open()} }
                    }
                }
            }
        }
        Panel {
            title: "Available in the demonstrator"; Layout.fillWidth: true; implicitHeight: 120
            ColumnLayout { anchors.fill: parent; spacing: 10
                Text { Layout.fillWidth: true; text: "C API for customer DLLs · seeded replay · A/B comparison · batch PASS/FAIL campaigns · HTML/JSON/CSV reports · 1× desktop realtime with measured lag."; color: "#167565"; font.pixelSize: 12; wrapMode: Text.WordWrap }
                ActionButton { text: "Try API and campaign"; primary: true; onClicked: root.apiRequested() }
            }
        }
        Panel {
            title: "Describe your test bench"; Layout.fillWidth: true; implicitHeight: 260
            ColumnLayout { anchors.fill: parent; spacing: 12
                RowLayout { Layout.fillWidth: true
                    ColumnLayout { Layout.fillWidth: true; Text { text: "Product / board"; color: "#69738c"; font.pixelSize: 11 } ValueField { id: product; Layout.fillWidth: true; placeholderText: "E.g. valve with STM32 / customer board" } }
                    ColumnLayout { Layout.preferredWidth: 160; Text { text: "Required interface"; color: "#69738c"; font.pixelSize: 11 } ChoiceBox { id: protocol; Layout.fillWidth: true; model: ["To be defined","Software only / DLL","USB / serial","CAN","Ethernet","Electrical I/O"] } }
                    ColumnLayout { Layout.preferredWidth: 130; Text { text: "Required cycle (ms)"; color: "#69738c"; font.pixelSize: 11 } ValueField { id: cycle; Layout.fillWidth: true; placeholderText: "To be agreed"; validator: DoubleValidator { bottom: .001; top: 10000; locale: "C" } } }
                }
                ValueField { id: need; Layout.fillWidth: true; placeholderText: "Which behavior or fault do you want to verify?" }
                Text { Layout.fillWidth: true; text: "Save a technical request to attach to your inquiry. The software does not send data automatically or modify the board."; font.pixelSize: 11; color: "#8790a5"; wrapMode: Text.WordWrap }
                RowLayout {
                    ActionButton { text: "Save technical request"; onClicked: root.briefRequested({product:product.text,interface:protocol.currentText,cycle_ms:cycle.text,goal:need.text}) }
                    ActionButton { text: "Contact Silicon LogiX ↗"; primary: true; onClicked: Qt.openUrlExternally("https://www.siliconlogix.it/contatti") }
                }
            }
        }
    }
    Dialog {
        id: preview; parent: Overlay.overlay; property var description: ({})
        title: "Integration preview"; modal: true; width: 570; anchors.centerIn: parent; standardButtons: Dialog.Close
        contentItem: ColumnLayout { spacing: 16
            Text { Layout.fillWidth: true; text: preview.description.title || ""; font.pixelSize: 19; font.weight: Font.DemiBold; color: "#18203b"; wrapMode: Text.WordWrap }
            Text { Layout.fillWidth: true; text: preview.description.delivery || ""; font.pixelSize: 13; color: "#3436dc"; wrapMode: Text.WordWrap }
            Text { Layout.fillWidth: true; text: preview.description.need || ""; font.pixelSize: 12; color: "#69738c"; wrapMode: Text.WordWrap }
            Text { Layout.fillWidth: true; text: "This feature requires development and verification on your product. This screen describes the proposed integration."; font.pixelSize: 11; color: "#9b691b"; wrapMode: Text.WordWrap }
            ActionButton { text: "Discuss your bench ↗"; primary: true; onClicked: Qt.openUrlExternally("https://www.siliconlogix.it/contatti") }
        }
    }
}
