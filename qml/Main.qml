// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop Main component.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import SLX.Resilience 1.0

ApplicationWindow {
    id: app
    width: 1460; height: 950
    minimumWidth: 1180; minimumHeight: 780
    visible: !smokeMode
    title: "SLX ResilienceLab · Silicon LogiX"
    color: "#f3f5fb"
    font.family: "Segoe UI"
    property int page: 3
    property var s: lab.state
    property var cfg: lab.config
    property var cmp: lab.comparison
    property var timing: lab.clock
    Theme { id: appTheme }
    function showInformation(kind) { if(kind==="about") about.open(); else showDocument(kind) }
    function showDocument(name) {
        about.close()
        var names={license:"Evaluation license",notices:"Third-party notices","qt-license":"Qt open-source license",sdk:"C API · Customer control",integration:"PC / board integration","bench-api":"Live bench API / USB serial"}
        documentDialog.documentTitle=names[name] || "Document"
        documentDialog.documentSubtitle="SILICON LOGIX / RESILIENCELAB"
        documentDialog.markdown=name==="sdk" || name==="integration" || name==="bench-api"
        documentDialog.licenseText=lab.document(name)
        documentDialog.open()
    }
    property var faultKeys: ["sensor_stuck","sensor_drift","sensor_disconnect","main_stop","communication","actuator_jam","sensor_restore","main_restore","communication_restore","actuator_restore"]
    function pct(v) { return v === null || v === undefined ? "—" : Number(v).toFixed(1) }
    function statusText() { return s.safeStop ? "MOTION STOPPED" : s.watchdog ? "WAITING FOR COMMANDS" : Math.abs(s.motor)>0.001 ? "CONTROLLED MOTION" : "POSITION HELD" }

    FileDialog { id: saveScenario; title: "Save repeatable scenario"; fileMode: FileDialog.SaveFile; defaultSuffix: "json"; nameFilters: ["Scenario JSON (*.json)"]; onAccepted: lab.saveScenario(selectedFile) }
    FileDialog { id: openScenario; title: "Open scenario"; nameFilters: ["Scenario JSON (*.json)"]; onAccepted: lab.loadScenario(selectedFile) }
    FileDialog { id: saveReport; title: "Export HTML, JSON and CSV report"; fileMode: FileDialog.SaveFile; defaultSuffix: "html"; nameFilters: ["Report HTML (*.html)"]; onAccepted: lab.exportReport(selectedFile) }
    FileDialog { id: openPlugin; title: "Load customer controller · C ABI v1"; nameFilters: ["Control library (*.dll *.so *.dylib)"]; onAccepted: lab.loadPlugin(selectedFile) }

    property var pendingBrief: ({})
    FileDialog { id: saveCampaign; title: "Export campaign and three run reports"; fileMode: FileDialog.SaveFile; defaultSuffix: "html"; nameFilters: ["Campaign HTML (*.html)"]; onAccepted: lab.exportCampaign(selectedFile) }
    FileDialog { id: saveBrief; title: "Save technical request"; fileMode: FileDialog.SaveFile; defaultSuffix: "json"; nameFilters: ["Technical request JSON (*.json)"]; onAccepted: lab.saveIntegrationBrief(selectedFile,app.pendingBrief) }
    Rectangle {
        id: sidebar
        width: 206; anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        color: "#111832"
        Image { x: 22; y: 28; width: 156; height: 50; source: "qrc:/assets/logo-light.svg"; fillMode: Image.PreserveAspectFit }
        Rectangle { x: 22; y: 97; width: 162; height: 1; color: "#2a324b" }
        Text { x: 22; y: 119; text: "RESILIENCELAB"; color: "#c1c7de"; font.pixelSize: 11; font.letterSpacing: 2; font.weight: Font.DemiBold }
        Column {
            x: 12; y: 158; spacing: 8
            Repeater {
                model: [{name:"Getting started",hint:"Purpose and guided tests",symbol:"◇",page:3},{name:"Laboratory",hint:"Simulation and faults",symbol:"◈",page:0},{name:"Scenarios",hint:"Timeline and parameters",symbol:"≡",page:1},{name:"Comparison",hint:"A / B verification",symbol:"⇄",page:2},{name:"API and boards",hint:"PC / physical board",symbol:"⇆",page:4},{name:"Custom test bench",hint:"Custom integrations",symbol:"+",page:5}]
                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: 182; height: 66; radius: 9
                    color: app.page===modelData.page ? "#3436dc" : navMouse.containsMouse ? "#202944" : "transparent"
                    Text { x: 13; y: 17; text: modelData.symbol; color: "#dbe0ff"; font.pixelSize: 21 }
                    Column { x: 45; y: 15; spacing: 5
                        Text { text: modelData.name; color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        Text { text: modelData.hint; color: app.page===modelData.page ? "#d4d5ff" : "#8e99b7"; font.pixelSize: 10 }
                    }
                    MouseArea { id: navMouse; anchors.fill: parent; hoverEnabled: true; onClicked: app.page=modelData.page; cursorShape: Qt.PointingHandCursor }
                }
            }
        }
        Column { x: 22; anchors.bottom: parent.bottom; anchors.bottomMargin: 28; spacing: 12
            Text { text: "Control. Faults. Evidence."; color: "#acb6d0"; font.pixelSize: 11 }
            Text { text: "Software demonstrator\nv"+lab.applicationVersion+" · C++ / Qt Quick / OpenGL"; color: "#7482a4"; font.pixelSize: 10; lineHeight: 1.5 }
            ActionButton { text: "About and licenses"; implicitWidth: 162; onClicked: about.open() }
        }
    }
    ColumnLayout {
        anchors { left: sidebar.right; right: parent.right; top: parent.top; bottom: parent.bottom; margins: 24 }
        spacing: 16
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout { spacing: 5
                Text { text: app.page===4 ? "API and board connections" : app.page===5 ? "A bench for your product" : app.page===3 ? "SLX ResilienceLab" : app.page===0 ? "Control laboratory" : app.page===1 ? "Repeatable scenarios" : "Control comparison"; font.pixelSize: 25; font.weight: Font.DemiBold; color: "#18203b" }
                Text { text: app.page===4 ? "Choose where control runs: on the PC or in board firmware." : app.page===5 ? "Preview integrations Silicon LogiX can develop for your product." : app.page===3 ? "Repeatable faults. Observable decisions. Comparable results." : app.page===0 ? "Motorized valve · 3 sensors · 2 controllers · virtual bus" : app.page===1 ? "Every test preserves configuration, fault timeline and seed." : "Two implementations, the same experiment, measured results."; color: "#69738c"; font.pixelSize: 11 }
            }
            Item { Layout.fillWidth: true }
            ActionButton { text: "Open scenario"; onClicked: openScenario.open() }
            ActionButton { text: "Export report"; primary: true; enabled: lab.history.length>0 || !!app.cmp.a; onClicked: saveReport.open() }
        }
        Rectangle {
            visible: app.page<3; Layout.fillWidth: true; implicitHeight: 64; radius: 10; color: "white"; border.color: "#dfe4f0"
            RowLayout { anchors.fill: parent; anchors.margins: 12; spacing: 12
                Text { text: "SCENARIO"; color: "#8790a5"; font.pixelSize: 10; font.letterSpacing: 1 }
                ChoiceBox { id: presets; model: lab.scenarioNames; displayText: lab.scenarioName; Layout.preferredWidth: 200; onActivated: lab.selectScenario(currentIndex) }
                ChoiceBox { model: ["0.5× slow motion","1× PC realtime","4× accelerated"]; currentIndex: lab.playbackRate===.5 ? 0 : lab.playbackRate===1 ? 1 : 2; Layout.preferredWidth: 158; onActivated: lab.setPlaybackRate([.5,1,4][currentIndex]) }
                Item { Layout.fillWidth: true }
                Text { text: "t = "+Number(s.time).toFixed(2)+" s"; color: "#18203b"; font.family: "Consolas"; font.pixelSize: 13 }
                ActionButton { text: "Reset"; onClicked: lab.reset() }
                ActionButton { text: "Step"; enabled: !lab.running && s.time<s.duration; onClicked: lab.step() }
                ActionButton { text: lab.running ? "Ⅱ  Pause" : "▶  Run"; primary: true; onClicked: lab.toggleRun() }
            }
        }
        StackLayout {
            currentIndex: app.page; Layout.fillWidth: true; Layout.fillHeight: true
            ScrollView {
                id: labScroll
                objectName: "laboratoryScroll"
                clip: true
                contentWidth: availableWidth
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: labScroll.contentHeight > labScroll.availableHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            ColumnLayout {
                width: labScroll.availableWidth
                height: Math.max(labScroll.availableHeight, implicitHeight)
                spacing: 12
                LiveControls { faultKeys: app.faultKeys; Layout.fillWidth: true }
                ScenarioGuide { guide: lab.guide; Layout.fillWidth: true; visible: lab.scenarioName!=="Live bench" }
                RowLayout { Layout.fillWidth: true; spacing: 12
                    Metric { Layout.fillWidth: true; label: "REQUESTED OPENING"; value: pct(s.target); unit: "%"; detail: "Setpoint sent to controllers"; accent: "#69738c" }
                    Metric { Layout.fillWidth: true; label: "ACTUAL POSITION"; value: pct(s.position); unit: "%"; detail: "Virtual plant · "+pct(s.velocity)+" %/s" }
                    Metric { Layout.fillWidth: true; label: "CONTROL MEASUREMENT"; value: pct(s.fused); unit: "%"; detail: s.quorum ? "Fusion of trustworthy readings" : "Insufficient quorum"; accent: s.quorum ? "#167565" : "#b4484e" }
                    Metric { Layout.fillWidth: true; label: "AUTHORIZED CONTROL"; value: s.owner==="main" ? "Primary" : "Backup"; detail: "Arbitration · epoch "+s.epoch; accent: s.owner==="main" ? "#3436dc" : "#9b691b" }
                }
                RowLayout {
                    Layout.fillWidth: true; Layout.fillHeight: true; spacing: 14
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 265; Layout.minimumWidth: 400
                        radius: 12; color: "#101625"; clip: true
                        ValveView { id: valve; objectName: "valveViewport"; anchors.fill: parent; anchors.topMargin: 30; anchors.bottomMargin: 55; position: s.position }
                        MouseArea { objectName: "valveCameraInteraction"; anchors.fill: valve; preventStealing: true; acceptedButtons: Qt.LeftButton; cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor; property real lastX: 0; property real lastY: 0
                            onPressed: function(mouse){lastX=mouse.x;lastY=mouse.y}
                            onPositionChanged: function(mouse){if(pressed){valve.orbit(mouse.x-lastX,mouse.y-lastY);lastX=mouse.x;lastY=mouse.y}}
                            onWheel: function(wheel){valve.zoom+=wheel.angleDelta.y/1200;wheel.accepted=true}
                        }
                        Text { x: 18; y: 17; text: "VIRTUAL PLANT  /  SLX VALVE"; color: "#aab9d1"; font.pixelSize: 10; font.letterSpacing: 1.4 }
                        Row { anchors.right: parent.right; anchors.rightMargin: 14; y: 10; spacing: 6
                            ActionButton { text: valve.cutaway ? "Normal view" : "Cutaway view"; implicitHeight: 30; onClicked: valve.cutaway=!valve.cutaway }
                            ActionButton { text: "Center"; implicitHeight: 30; implicitWidth: 65; onClicked: {valve.azimuth=34;valve.elevation=23;valve.zoom=1} }
                        }
                        Text { anchors.centerIn: parent; visible: valve.assetError.length>0; text: valve.assetError; color: "#ff8e8e" }
                        Rectangle { x: 16; anchors.bottom: parent.bottom; anchors.bottomMargin: 18; width: 7; height: 7; radius: 4; color: s.safeStop ? "#f18a90" : "#4ee0bc" }
                        Text { x: 31; anchors.bottom: parent.bottom; anchors.bottomMargin: 15; text: statusText(); color: "#dce4f6"; font.pixelSize: 10; font.letterSpacing: .7 }
                        Text { anchors.right: parent.right; anchors.rightMargin: 18; anchors.bottom: parent.bottom; anchors.bottomMargin: 15; text: "ROTOR "+pct(s.rotorAngle)+"°  ·  drag to rotate"; color: "#8192b0"; font.pixelSize: 10 }
                    }
                    ColumnLayout { Layout.preferredWidth: 295; Layout.minimumWidth: 295; Layout.maximumWidth: 295; Layout.fillHeight: true; spacing: 12
                        Panel { title: "Measurements received by control"; Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 178
                            ColumnLayout { anchors.fill: parent; spacing: 8
                                Repeater { model: 3
                                    delegate: Rectangle {
                                        required property int index
                                        property var modelData: app.s.readings[index] || {}
                                        Layout.fillWidth: true; Layout.fillHeight: true; radius: 7; color: modelData.status==="ok" ? "#f4f7fb" : "#fff4ed"
                                        Rectangle { x: 9; y: 8; width: 4; height: parent.height-16; radius: 2; color: ["#dca348","#65b0d5","#bc80c1"][index] }
                                        Text { x: 21; y: 8; text: "SENSOR "+(index+1); font.pixelSize: 10; color: "#69738c" }
                                        Text { x: 21; y: 23; text: modelData.status+(modelData.trusted ? " · used" : " · excluded"); font.pixelSize: 10; color: modelData.status==="ok" ? "#167565" : "#b4484e" }
                                        Text { objectName: "sensorValue"+index; anchors.right: parent.right; anchors.rightMargin: 10; anchors.verticalCenter: parent.verticalCenter; text: pct(modelData.value)+" %"; font.pixelSize: 20; font.weight: Font.DemiBold; color: "#18203b" }
                                    }
                                }
                            }
                        }
                        Panel { title: "Controllers and heartbeat"; Layout.fillWidth: true; Layout.preferredHeight: 116
                            ColumnLayout { anchors.fill: parent; spacing: 5
                                Repeater { model: 2
                                    delegate: Item { required property int index; property var modelData: ({name:index===0 ? "Primary" : "Backup",alive:index===0 ? app.s.mainAlive : app.s.backupAlive,active:app.s.owner===(index===0 ? "main" : "backup"),age:index===0 ? app.s.mainHeartbeatAge : app.s.backupHeartbeatAge}); Layout.fillWidth: true; implicitHeight: 22
                                        Rectangle { x: 0; anchors.verticalCenter: parent.verticalCenter; width: 7; height: 7; radius: 4; color: modelData.alive ? "#22a68d" : "#c95862" }
                                        Text { x: 16; anchors.verticalCenter: parent.verticalCenter; text: modelData.name; color: "#18203b"; font.pixelSize: 11 }
                                        Text { anchors.right: ageLabel.left; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; text: !modelData.alive ? "TIMEOUT" : modelData.active ? "ACTIVE" : "STANDBY"; color: !modelData.alive ? "#b4484e" : modelData.active ? "#3436dc" : "#8790a5"; font.pixelSize: 10; font.weight: Font.DemiBold }
                                        Text { id: ageLabel; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Number(modelData.age).toFixed(2)+"s"; color: "#8790a5"; font.pixelSize: 10 }
                                    }
                                }
                            }
                        }
                    }
                }
                RowLayout { Layout.fillWidth: true; Layout.preferredHeight: 174; spacing: 14
                    Panel { title: "Opening and measurements over time"; Layout.fillWidth: true; Layout.fillHeight: true
                        ColumnLayout { anchors.fill: parent; spacing: 3
                            TraceChart { Layout.fillWidth: true; Layout.fillHeight: true; samples: lab.history; duration: s.duration }
                            Text { textFormat: Text.RichText; text: "<font color='#a0a8ba'>● Target</font>   <font color='#3436dc'>● Actual</font>   <font color='#129984'>● Fused</font>   <font color='#dca348'>S1</font> / <font color='#65b0d5'>S2</font> / <font color='#bc80c1'>S3</font>"; font.pixelSize: 10 }
                        }
                    }
                    Panel { title: "Event timeline"; Layout.preferredWidth: 295; Layout.fillHeight: true
                        ListView { anchors.fill: parent; clip: true; spacing: 10; model: lab.events; ScrollBar.vertical: ScrollBar {}
                            delegate: Column { required property var modelData; width: ListView.view.width-12; spacing: 3
                                Text { text: Number(modelData.time).toFixed(2)+" s  ·  "+modelData.source; color: "#8790a5"; font.pixelSize: 9 }
                                Text { width: parent.width; text: modelData.text; wrapMode: Text.WordWrap; color: modelData.code==="fault_injected" ? "#9b691b" : "#28324e"; font.pixelSize: 10 }
                            }
                        }
                    }
                }

            }
            }
            ScenarioPage { config: app.cfg; state: app.s; faultKeys: app.faultKeys; onSaveRequested: saveScenario.open(); onPluginRequested: openPlugin.open() }
            ComparePage { comparison: app.cmp; onExportRequested: saveReport.open() }
            IntroPage { onDemoRequested: function(scenario){lab.startDemo(scenario);app.page=0}; onLabRequested: app.page=0; onIntegrationRequested: app.page=4 }
            IntegrationPage { onPluginRequested: openPlugin.open(); onDocumentRequested: function(name){app.showDocument(name)}; onCampaignExportRequested: saveCampaign.open() }
            SolutionsPage { onApiRequested: app.page=4; onBriefRequested: function(needs){app.pendingBrief=needs;saveBrief.open()} }
        }
        RowLayout { Layout.fillWidth: true
            Text { text: lab.message; color: "#69738c"; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideMiddle }
            Text { visible: app.page===0 && app.timing.wall_elapsed_s>0; text: "PC "+Number(app.timing.wall_elapsed_s).toFixed(1)+" s · lag "+Number(app.timing.lag_ms).toFixed(0)+" ms"; color: "#8790a5"; font.pixelSize: 10 }
            Text { text: "BUS  "+s.delivered+" delivered  /  "+s.dropped+" dropped   ·   Seed "+s.seed; color: "#8790a5"; font.pixelSize: 10 }
        }
    }
    AboutDialog {
        id: about; anchors.centerIn: parent; theme: appTheme; appVersionText: lab.applicationVersion
        onLicenseRequested: app.showDocument("license")
        onNoticesRequested: app.showDocument("notices")
    }
    LicenseDialog {
        id: documentDialog; anchors.centerIn: parent; theme: appTheme; licenseText: ""
        onDocumentLinkActivated: function(link){ if(link==="licenses/Qt-Open-Source-LICENSE.txt") app.showDocument("qt-license"); else if(link.indexOf("https://")===0) Qt.openUrlExternally(link) }
    }
}
