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

BorderImage {
    id: trackpad
    source: "qrc:/images/trackpad.png"
    smooth: true
    border { top: 20; left: 20; right: 20; bottom: 20 }

    MouseArea {
        property int lastX: -1
        property int lastY: -1
        property int startX: -1
        property int startY: -1
        property bool inited: false
        property bool ignoreClick: false

        function manhattanLength(x, y) { return Math.sqrt(x*x + y*y) }

        anchors.fill: parent
        onPressed: { startX = startY = -1; ignoreClick = false; rpcClient.callBool("trackpad.setEnabled", true) }
        onReleased: { inited = false; }
        onClicked: if (!ignoreClick) { rpcClient.callVoid("trackpad.click") }

        onPositionChanged: {
            if (inited) {
                rpcClient.call2("trackpad.moveBy", mouse.x - lastX, mouse.y - lastY)
            }

            if (startX == -1) startX = mouse.x
            if (startY == -1) startY = mouse.y

            if (manhattanLength(startX - mouse.x, startY - mouse.y) > 16)
                ignoreClick = true

            lastX = mouse.x
            lastY = mouse.y
            inited = true
        }
    }
}

