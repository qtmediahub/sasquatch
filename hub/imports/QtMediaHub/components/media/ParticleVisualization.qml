/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

import QtQuick 1.0
import Qt.labs.particles 1.0

Rectangle {
    id: root
    color: "black"

    property variant resourcePath: "particles/"
    property bool running : false
    property int particleCountFactor : 3

    QtObject {
        id: d
        property bool running: root.running && Qt.application.active
    }

    Timer {
        interval: 250; running: d.running; repeat: true
        onTriggered: part3.burst(2*particleCountFactor )
    }

    ParallelAnimation {
        id: anim
        running: d.running

        property real setting1 : Math.random()
        property real setting2 : Math.random()
        property real setting3 : Math.random()

        PropertyAnimation {
            target: part3
            property: "x"
            to: root.width*anim.setting1
            duration: 3000
            easing.type: Easing.InOutBack
        }
        PropertyAnimation {
            target: part3
            property: "y"
            to: root.height*anim.setting2
            duration: 3000
            easing.type: Easing.InOutBack
        }
        PropertyAnimation {
            target: part3
            property: "scale"
            to: anim.setting3
            duration: 4000
            easing.type: Easing.InOutBack
        }

        onCompleted: {
            if (d.running) {
                anim.setting1 = Math.random()
                anim.setting2 = Math.random()
                anim.setting3 = Math.random()
                anim.start()
            }
        }
    }

    Particles {
        id: part3
        y: parent.height/2.0
        width: 1
        height: 1
        source: resourcePath + "particle2.png"
        lifeSpan: 5000
        count: d.running ? 50*particleCountFactor  : 0
        angle: 0
        scale: 0.5
        opacity:  0.5
        angleDeviation: 360
        velocity: 250
        velocityDeviation: 500
    }

    Particles {
        id: part1
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width*2
        height: 30
        source: resourcePath + "particle1.png"
        lifeSpan: 5000
        count: d.running ? 40*particleCountFactor  : 0
        angle: 70
        angleDeviation: 36
        velocity: 300
        scale:  0.5
        velocityDeviation: 10
        opacity: 0.5
        ParticleMotionWander {
            xvariance: 0
            pace: 300
        }
    }
    Particles {
        id: part2
        width: parent.width
        height: parent.height
        source: resourcePath + "particle.png"
        lifeSpan: 5000
        count: d.running ? 20*particleCountFactor  : 0
        angle: 270
        angleDeviation: 45
        velocity: 50
        velocityDeviation: 30
        ParticleMotionGravity {
            yattractor: 1000
            xattractor: 0
            acceleration: 25
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 500
            }
        }
    }

    function metronomTick() {
        if (d.running) {
            part2.opacity = Math.random()
        }
    }
}
