// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Desktop TraceChart component.

import QtQuick
Canvas {
    id: chart
    property var samples: []
    property var other: []
    property real duration: 12
    property bool sensors: true
    onSamplesChanged: requestPaint()
    onOtherChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        var c = getContext("2d")
        c.reset()
        var left = 30, top = 8, w = width - 42, h = height - 32
        if (w <= 0 || h <= 0) return
        c.font = "10px sans-serif"
        for (var y = 0; y <= 100; y += 20) {
            var py = top + h * (1 - y / 100)
            c.strokeStyle = "#e9ecf4"; c.lineWidth = 1
            c.beginPath(); c.moveTo(left, py); c.lineTo(left + w, py); c.stroke()
            c.fillStyle = "#8790a5"; c.fillText(y, 1, py + 3)
        }
        for (var t = 0; t <= 4; ++t) {
            var x = left + w * t / 4
            c.fillStyle = "#8790a5"; c.fillText((duration*t/4).toFixed(1)+"s", x - 9, height - 4)
        }
        function series(data, key, color, line, sensor) {
            c.strokeStyle = color; c.lineWidth = line; c.beginPath()
            var connected = false
            for (var i = 0; i < data.length; ++i) {
                var s = data[i], v = sensor >= 0 ? (s.readings ? s.readings[sensor].value : s["sensor"+(sensor+1)]) : s[key]
                if (v === null || v === undefined) { connected = false; continue }
                var px = left + w * s.time / duration, py = top + h * (1 - Math.max(0,Math.min(100,v))/100)
                if (connected) c.lineTo(px,py); else c.moveTo(px,py)
                connected = true
            }
            c.stroke()
        }
        if (sensors) {
            series(samples,"","#e6b35a",1,0)
            series(samples,"","#82bfdb",1,1)
            series(samples,"","#ce99cf",1,2)
        }
        series(samples,"target","#a0a8ba",1.5,-1)
        series(samples,"position","#3436dc",2.5,-1)
        series(samples,"fused","#129984",1.5,-1)
        if (other.length) series(other,"position","#e0833a",2,-1)
    }
}
