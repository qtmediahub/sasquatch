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

Item {
    id: root
    width: grid.width
    height: grid.height
    property bool _debug: false
    signal left()
    signal right()
    signal center()
    signal up()
    signal down()

    Image {
        id: grid
        source: "qrc:/images/navpad_control.png"
        smooth: true
    }
    Image {
        id: up
        x: 75
        y: 2
        source: "qrc:/images/navpad_up.png"
        opacity: root._debug?1.0:0.0
    }
    MouseArea {
        x: 22
        y: -4
        anchors.fill: up
        onPressed:  elementPressed(up)
        onReleased: elementReleased(up)
        onClicked: root.up()
    }
    Image {
        id: down
        x: 74
        y: 176
        source: "qrc:/images/navpad_down.png"
        opacity: root._debug?1.0:0.0
    }
    MouseArea {
        x: 21
        y: 59
        anchors.fill: down
        onPressed:  elementPressed(down)
        onReleased: elementReleased(down)
        onClicked: root.down()
    }
    Image {
        id: left
        x: 2
        y: 77
        source: "qrc:/images/navpad_left.png"
        opacity: root._debug?1.0:0.0
    }
    MouseArea {
        x: -6
        y: 22
        width: 28
        anchors.leftMargin: 0
        anchors.fill: left
        onPressed:  elementPressed(left)
        onReleased: elementReleased(left)
        onClicked: root.left()
    }
    Image {
        id: right
        x: 179
        y: 75
        source: "qrc:/images/navpad_right.png"
        opacity: root._debug?1.0:0.0
    }
    MouseArea {
        x: 59
        y: 22
        anchors.fill: right
        onPressed:  elementPressed(right)
        onReleased: elementReleased(right)
        onClicked: root.right()
    }
    Image {
        id: center
        x: 85
        y: 86
        source: "qrc:/images/navpad_center.png"
        opacity: root._debug?1.0:0.0
    }
    MouseArea {
        x: 59
        y: 22
        anchors.fill: center
        onPressed:  elementPressed(center)
        onReleased: elementReleased(center)
        onClicked: root.center()

    }

    function elementPressed(element) {
        element.opacity = 0.7;
        grid.scale = 0.98
    }

    function elementReleased(element) {
        element.opacity = 0.0;
        grid.scale = 1
    }

}
