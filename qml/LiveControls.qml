// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop LiveControls component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
    id: root
    title: "Manual control / live fault injection"
    implicitHeight: 204
    property var snapshot: lab.state
    property var faultKeys: []
    ColumnLayout {
        anchors.fill: parent; spacing: 8
        RowLayout {
            Layout.fillWidth: true
            Text { text: "Requested opening"; color: "#69738c"; font.pixelSize: 11 }
            Slider { objectName: "liveTargetSlider"; from: 0; to: 100; value: root.snapshot.target; Layout.fillWidth: true; enabled: root.snapshot.time < root.snapshot.duration; onMoved: lab.setTarget(value) }
            ValueField { id: openingField; objectName: "liveTargetField"; Layout.preferredWidth: 66; validator: DoubleValidator { bottom: 0; top: 100; locale: "C" }
                enabled: root.snapshot.time < root.snapshot.duration; onEditingFinished: if(acceptableInput && Number(text)!==root.snapshot.target) lab.setTarget(Number(text)) }
            Binding { target: openingField; property: "text"; value: Number(root.snapshot.target).toFixed(1); when: !openingField.activeFocus }
            Text { text: "%"; color: "#69738c"; font.pixelSize: 11 }
            ActionButton { text: "Close"; enabled: root.snapshot.time < root.snapshot.duration; onClicked: lab.setTarget(0) }
            ActionButton { text: "Open"; enabled: root.snapshot.time < root.snapshot.duration; onClicked: lab.setTarget(100) }
            ActionButton { text: "New live bench"; objectName: "startLiveBench"; primary: true; onClicked: lab.startLive() }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 7
            ChoiceBox { id: kind; Layout.fillWidth: true; model: ["Stuck sensor","Sensor drift","Disconnected sensor","Stop primary","Intermittent communication","Jammed actuator","Restore sensor","Restart primary","Restore communication","Release actuator"] }
            ChoiceBox { id: sensor; Layout.preferredWidth: 94; model: ["S1","S2","S3","All three"]; enabled: faultKeys[kind.currentIndex].indexOf("sensor_")===0 }
            ActionButton { text: "Inject"; danger: true; enabled: root.snapshot.time < root.snapshot.duration; onClicked: {
                var key=faultKeys[kind.currentIndex]; var value=key==="communication" ? .65 : key==="sensor_drift" ? 4 : 0
                if(key.indexOf("sensor_")===0) lab.injectSensors(key,sensor.currentIndex===3 ? -1 : sensor.currentIndex,value)
                else lab.inject(key,0,value)
            } }
            ActionButton { text: "Freeze all"; objectName: "freezeAllSensors"; enabled: root.snapshot.time < root.snapshot.duration; onClicked: lab.injectSensors("sensor_stuck",-1) }
            ActionButton { text: "Drop all"; enabled: root.snapshot.time < root.snapshot.duration; onClicked: lab.injectSensors("sensor_disconnect",-1) }
            ActionButton { text: "Restore all"; enabled: root.snapshot.time < root.snapshot.duration; onClicked: lab.injectSensors("sensor_restore",-1) }
        }
        Text {
            Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize: 10; color: "#8790a5"
            text: "Live bench starts a nominal five-minute run at 1×. Actions are recorded for replay. Three frozen, agreeing sensors can keep quorum; a motion request exposes missing progress. At rest, identical frozen readings cannot be diagnosed by voting alone."
        }
        Text { Layout.fillWidth: true; font.pixelSize: 11; color: "#b4484e"; text: root.snapshot[root.snapshot.owner] && root.snapshot[root.snapshot.owner].stopReason==="no_measured_progress" ? "No measured progress: motion stopped. Restore sensors and change the target, or reset the run." : root.snapshot[root.snapshot.owner] && root.snapshot[root.snapshot.owner].stopReason==="customer_api_error" ? "Customer control error: reset required." : ""; visible: text.length>0 }
    }
}
