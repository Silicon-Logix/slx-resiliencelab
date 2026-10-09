// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ComparePage component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: root
    property var comparison: ({})
    property bool ready: !!comparison.a
    signal exportRequested()
    spacing: 16
    Panel { title: "A / B experiment"; Layout.fillWidth: true; Layout.preferredHeight: 160
        RowLayout { anchors.fill: parent; spacing: 24
            ColumnLayout { Layout.fillWidth: true; spacing: 9
                Text { text: "Change the control. Measure the effect."; color: "#18203b"; font.pixelSize: 20; font.weight: Font.DemiBold }
                Text { Layout.fillWidth: true; text: "Run two complete replays with identical seed, plant parameters, noise, transport and faults. A uses the selected control; B uses the SLX alternative. Includes errors, detections and stops."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 12 }
            }
            ActionButton { text: "Run comparison"; primary: true; onClicked: lab.compareVersions() }
            ActionButton { text: "Export A / B"; enabled: root.ready; onClicked: root.exportRequested() }
        }
    }
    RowLayout { Layout.fillWidth: true; spacing: 14
        Repeater { model: root.ready ? [root.comparison.a,root.comparison.b] : []
            delegate: Panel { required property var modelData; required property int index; title: (index===0 ? "A · " : "B · ")+(modelData.scenario.customer_plugin ? "Customer control" : modelData.scenario.control_version); Layout.fillWidth: true; Layout.preferredHeight: 206
                GridLayout { anchors.fill: parent; columns: 2; rowSpacing: 8
                    Text { text: "Final error"; color: "#69738c"; font.pixelSize: 12 }
                    Text { text: Number(modelData.metrics.final_error_pct).toFixed(3)+" %"; color: index===0 ? "#3436dc" : "#d57632"; font.pixelSize: 21; font.weight: Font.DemiBold }
                    Text { text: "Mean absolute error"; color: "#69738c"; font.pixelSize: 12 }
                    Text { text: Number(modelData.metrics.mean_absolute_error_pct).toFixed(3)+" %"; color: "#18203b"; font.pixelSize: 15 }
                    Text { text: "Takeovers / stops"; color: "#69738c"; font.pixelSize: 12 }
                    Text { text: modelData.metrics.takeovers+" / "+modelData.metrics.stops; color: "#18203b"; font.pixelSize: 15 }
                    Text { text: "Dropped messages / rejected commands"; color: "#69738c"; font.pixelSize: 12 }
                    Text { text: modelData.metrics.messages_dropped+" / "+modelData.metrics.commands_rejected; color: "#18203b"; font.pixelSize: 15 }
                }
            }
        }
    }
    Panel { title: "Actual position trajectories"; Layout.fillWidth: true; Layout.fillHeight: true
        ColumnLayout { anchors.fill: parent; spacing: 12
            TraceChart { Layout.fillWidth: true; Layout.fillHeight: true; sensors: false; samples: root.ready ? root.comparison.a.samples : []; other: root.ready ? root.comparison.b.samples : []; duration: root.ready ? root.comparison.a.final.duration : 12 }
            Text { text: root.ready ? "Purple: A   ·   Orange: B   ·   Green: A fused measurement   ·   Grey: target" : "Run the comparison to view results."; color: "#69738c"; font.pixelSize: 12 }
        }
    }
}
