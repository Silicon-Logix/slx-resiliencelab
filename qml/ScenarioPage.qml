// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop ScenarioPage component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
RowLayout {
    id: root
    property var config: ({})
    property var state: ({})
    property var faultKeys: []
    signal saveRequested()
    signal pluginRequested()
    spacing: 14
    Panel { title: "Experiment parameters"; Layout.fillWidth: true; Layout.fillHeight: true
        ColumnLayout { anchors.fill: parent; spacing: 14
            Text { text: "Edit parameters, apply and replay the scenario."; color: "#69738c"; font.pixelSize: 12 }
            ScrollView { id: parameterScroll; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; contentWidth: availableWidth
                GridLayout { id: grid; width: parameterScroll.availableWidth-18; columns: 2; rowSpacing: 11; columnSpacing: 14
                    Repeater { id: fields; model: [
                        {key:"target_percent",name:"Initial requested opening (%)"},
                        {key:"initial_percent",name:"Initial actual position (%)"},
                        {key:"speed_percent_s",name:"Maximum speed (%/s)"},
                        {key:"duration_s",name:"Scenario duration (s)"},
                        {key:"sensor_threshold_percent",name:"Disagreement threshold (%)"},
                        {key:"discordance_dwell_s",name:"Disagreement dwell (s)"},
                        {key:"measurement_timeout_s",name:"Measurement timeout (s)"},
                        {key:"recovery_dwell_s",name:"Sensor recovery dwell (s)"},
                        {key:"heartbeat_timeout_s",name:"Heartbeat timeout (s)"},
                        {key:"command_timeout_s",name:"Motor command timeout (s)"},
                        {key:"bus_delay_s",name:"Bus delay (s)"},
                        {key:"bus_jitter_s",name:"Bus jitter (s)"},
                        {key:"bus_loss",name:"Loss probability (0–1)"},
                        {key:"sensor_noise_percent",name:"Sensor noise (±%)"}]
                        delegate: ColumnLayout { required property var modelData; property alias entry: input.text; Layout.fillWidth: true; Layout.minimumWidth: (grid.width-14)/2; spacing: 4
                            Text { text: modelData.name; color: "#69738c"; font.pixelSize: 10 }
                            ValueField { id: input; Layout.fillWidth: true; text: String(root.config[modelData.key]); selectByMouse: true; validator: DoubleValidator { locale: "C" } }
                        }
                    }
                    ColumnLayout { Layout.fillWidth: true; spacing: 4
                        Text { text: "Random seed"; color: "#69738c"; font.pixelSize: 10 }
                        ValueField { id: seed; text: String(root.state.seed); validator: IntValidator { bottom: 0 } Layout.fillWidth: true; selectByMouse: true }
                    }
                    ColumnLayout { Layout.fillWidth: true; spacing: 4
                        Text { text: "Control version"; color: "#69738c"; font.pixelSize: 10 }
                        ChoiceBox { id: version; model: ["robust-v1","cautious-v2"]; currentIndex: root.state.version==="cautious-v2"?1:0; Layout.fillWidth: true }
                    }
                }
            }
            RowLayout { Layout.fillWidth: true
                ActionButton { text: "Apply and reset"; primary: true; onClicked: {var changes={};for(var i=0;i<fields.count;++i){var f=fields.itemAt(i);changes[f.modelData.key]=Number(f.entry)}lab.configure(changes,Number(seed.text),version.currentText)} }
                ActionButton { text: "Save scenario"; onClicked: root.saveRequested() }
                Item { Layout.fillWidth: true }
            }
        }
    }
    ColumnLayout { Layout.preferredWidth: 370; Layout.minimumWidth: 370; Layout.maximumWidth: 370; Layout.fillHeight: true; spacing: 14
        Panel { title: "Fault timeline"; Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout { anchors.fill: parent; spacing: 12
                ListView { Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: lab.timeline; spacing: 9; ScrollBar.vertical: ScrollBar {}
                    delegate: Rectangle { required property var modelData; width: ListView.view.width-12; height: 52; radius: 7; color: "#f3f5fb"
                        Column { x: 10; y: 9; spacing: 4
                            Text { text: "t = "+Number(modelData.at_s).toFixed(2)+" s"; color: "#3436dc"; font.pixelSize: 11; font.weight: Font.DemiBold }
                            Text { text: modelData.kind+(modelData.kind.indexOf("sensor_")===0 ? " · S"+(modelData.sensor+1) : ""); color: "#69738c"; font.pixelSize: 11 }
                        }
                    }
                }
                Text { text: "Add an event (applies and resets the run)."; font.pixelSize: 10; color: "#69738c" }
                RowLayout { Layout.fillWidth: true
                    ValueField { id: time; text: "3.0"; Layout.preferredWidth: 72; validator: DoubleValidator { bottom: 0; locale: "C" } placeholderText: "Time s" }
                    ChoiceBox { id: scheduledKind; model: root.faultKeys; Layout.fillWidth: true }
                }
                RowLayout { Layout.fillWidth: true
                    ChoiceBox { id: scheduledSensor; model: ["S1","S2","S3"]; Layout.preferredWidth: 68 }
                    ValueField { id: faultValue; text: "0.65"; validator: DoubleValidator { locale: "C" } Layout.fillWidth: true; placeholderText: "Valore" }
                    ActionButton { text: "Add"; onClicked: lab.scheduleFault(Number(time.text),scheduledKind.currentText,scheduledSensor.currentIndex,Number(faultValue.text)) }
                }
                Text { Layout.fillWidth: true; text: "Value: communication = loss 0–1; sensor_drift = %/s. Other faults ignore the value."; font.pixelSize: 10; color: "#8790a5"; wrapMode: Text.WordWrap }
                ActionButton { text: "Clear timeline"; danger: true; onClicked: lab.clearTimeline() }
            }
        }
        Panel { title: "Customer control integration"; Layout.fillWidth: true; Layout.preferredHeight: 184
            ColumnLayout { anchors.fill: parent; spacing: 12
                Text { Layout.fillWidth: true; text: "Versioned C API, independent controller contexts. Customer code receives setpoint and measurements; fusion, bus and arbitration remain observable."; wrapMode: Text.WordWrap; color: "#69738c"; font.pixelSize: 11 }
                RowLayout {
                    ActionButton { text: "Load DLL"; onClicked: root.pluginRequested() }
                    ActionButton { text: "SLX control"; onClicked: lab.removePlugin() }
                }
                Text { text: "Active: "+root.state.version; color: "#3436dc"; font.pixelSize: 11 }
            }
        }
    }
}
