// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop IntegrationPage component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id: root
    signal pluginRequested()
    signal campaignExportRequested()
    signal documentRequested(string name)
    contentWidth: availableWidth; clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: contentHeight > availableHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    ColumnLayout {
        width: root.availableWidth; spacing: 16
        Rectangle {
            Layout.fillWidth: true; implicitHeight: 98; radius: 12; color: "#111832"
            Column { x: 20; y: 17; width: parent.width-40; spacing: 9
                Text { text: "The simulator provides the plant. The customer provides the control."; color: "white"; font.pixelSize: 20; font.weight: Font.DemiBold }
                Text { width: parent.width; text: "Measurements go from the simulator to the control. The motor command returns to the simulator and moves the virtual valve. Test code on the PC; firmware testing needs a board and a matching adapter."; wrapMode: Text.WordWrap; color: "#bec8e1"; font.pixelSize: 12 }
            }
        }
        ConnectionsPanel { Layout.fillWidth: true; onDocumentRequested: root.documentRequested("bench-api") }
        Panel {
            title: "1 · Customer code on the PC / Software-in-the-Loop"; Layout.fillWidth: true; implicitHeight: 267
            ColumnLayout {
                anchors.fill: parent; spacing: 10
                LoopDiagram { Layout.fillWidth: true }
                Text { Layout.fillWidth: true; text: "AVAILABLE · Receives setpoint, readings, measurement ages and fused measurement; returns the motor command. Separate primary and backup contexts. Fusion, heartbeat and arbitration remain in the SLX host."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                RowLayout {
                    ActionButton { text: "Try example DLL"; primary: true; objectName: "loadExampleController"; onClicked: lab.loadExamplePlugin() }
                    ActionButton { text: "Load your DLL"; onClicked: root.pluginRequested() }
                    ActionButton { text: "C API and contract"; onClicked: root.documentRequested("sdk") }
                    ActionButton { text: "SLX control"; onClicked: lab.removePlugin() }
                    Text { text: "Active: "+lab.controllerLabel; font.pixelSize: 10; color: "#3436dc"; Layout.fillWidth: true; elide: Text.ElideRight }
                }
            }
        }
        Panel {
            title: "2 · Firmware on a physical board / Hardware-in-the-Loop"; Layout.fillWidth: true; implicitHeight: 300
            ColumnLayout {
                anchors.fill: parent; spacing: 10
                LoopDiagram { hardware: true; Layout.fillWidth: true }
                Text { Layout.fillWidth: true; text: "CUSTOM PROJECT PREVIEW · The PC simulates the product; the board receives firmware inputs and returns commands. CAN, Ethernet and electrical I/O need an adapter for the customer's protocol and signal levels."; wrapMode: Text.WordWrap; color: "#9b691b"; font.pixelSize: 11 }
                Text { Layout.fillWidth: true; text: "Full firmware control requires agreed signals, messages, cycle timing and watchdogs. The live serial API above supports bench commands and measurement reads. Guaranteed deadlines require a dedicated realtime target."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                RowLayout {
                    ActionButton { text: "Connect firmware control"; enabled: false; ToolTip.text: "Requires a customer-specific adapter and firmware"; ToolTip.visible: hovered }
                    ActionButton { text: "View integration flow"; onClicked: root.documentRequested("integration") }
                    ActionButton { text: "Request integration ↗"; onClicked: Qt.openUrlExternally("https://www.siliconlogix.it/contatti") }
                }
            }
        }
        Panel {
            title: "Regression campaign / API, results and CI"; Layout.fillWidth: true; implicitHeight: lab.campaign.total ? 264 : 162
            ColumnLayout {
                anchors.fill: parent; spacing: 10
                Text { Layout.fillWidth: true; text: "Run the three tests using the active control. Criteria come from complete runs; reports preserve results, samples and messages. The --suite command returns a failure exit code when a criterion fails, for use in CI."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                RowLayout {
                    ActionButton { text: "Run all 3 tests"; primary: true; onClicked: lab.runCampaign() }
                    ActionButton { text: "Export campaign"; enabled: !!lab.campaign.total; onClicked: root.campaignExportRequested() }
                    Text { text: lab.campaign.total ? lab.campaign.passed+" / "+lab.campaign.total+" criteria passed" : "Campaign not run"; color: lab.campaign.total && lab.campaign.passed===3 ? "#167565" : "#69738c"; font.pixelSize: 12 }
                }
                Repeater {
                    model: lab.campaign.runs || []
                    delegate: RowLayout {
                        required property var modelData; Layout.fillWidth: true
                        Text { text: modelData.passed ? "PASS" : "FAIL"; color: modelData.passed ? "#167565" : "#b4484e"; font.pixelSize: 11; font.weight: Font.Bold; Layout.preferredWidth: 45 }
                        Text { text: modelData.requirement+" · "+modelData.name; color: "#18203b"; font.pixelSize: 11; Layout.fillWidth: true }
                        Text { text: "Error: "+Number(modelData.metrics.final_error_pct).toFixed(2)+" pp"; color: "#69738c"; font.pixelSize: 11 }
                    }
                }
                Text { text: "Valve demonstration criteria. Your product requirements are defined during integration."; color: "#8790a5"; font.pixelSize: 10 }
            }
        }
        Panel {
            title: "Realtime: when it helps"; Layout.fillWidth: true; implicitHeight: 134
            Text { anchors.fill: parent; text: "1× synchronizes virtual time with the PC's monotonic clock and recovers delayed steps. 0.5× helps inspect a fault; 4× accelerates the demo. Batch execution is independent of wall time and repeatable. Physical firmware timing requires a bench designed and measured for that target."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 12; lineHeight: 1.5 }
        }
    }
}
