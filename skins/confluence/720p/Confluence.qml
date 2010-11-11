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
import CustomCursor 1.0
//import "weather"

FocusScope {
    id: confluence

    property string themeResourcePath: skinPath + "/confluence/3rdparty/skin.confluence"

    //Will scale if loading 720p theme at different res
    height: 720; width: 1280
    focus: true; clip: true

    state: "showingRootMenu"

    states: [
        State {
            name: "showingRootMenu"
            PropertyChanges {
                target: blade
                state: "open"
            }
        },
        State {
            name: "showingWeatherDialog"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: weatherDialog
                state: "visible"
            }
        },
        State {
            name: "showingSystemDialog"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: systemDialog
                state: "visible"
            }
        },
        State {
            name: "showingWebDialog"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: webDialog
                state: "visible"
            }
        },
        State {
            name: "showingMapsDialog"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: webDialog
                state: "visible"
                //FIXME: Obviously remove hard coded path
                url: resourcePath + "/Google\ Maps/Nokia.html"
            }
        },
        State {
            name: "showingExitDialog"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: exitDialog
                state: "visible"
            }
        },
        State {
            name: "showingDashboard"
            PropertyChanges {
                target: blade
                state: "closed"
            }
            PropertyChanges {
                target: dashBoard
                state: "visible"
            }
        }
    ]

    transitions: Transition {
        reversible: true
        SequentialAnimation {
            NumberAnimation { property: "x"; duration: 300; easing.type: Easing.InOutQuad }
        }
    }

    Keys.onEscapePressed:
        confluence.state = "showingRootMenu"

    Background{ id: background }

    MainBlade { id: blade; focus: true }

    ExitDialog { id: exitDialog }

//    WeatherDialog { id: weatherDialog }

    SystemDialog { id: systemDialog }

    Ticker { id: ticker; anchors { right: parent.right; bottom: parent.bottom } }

    WebDialog { id: webDialog }

    Dashboard { id: dashBoard }

    CustomCursor {
        idleTimeout: 2
        defaultCursorPath: themeResourcePath + "/media/pointer-focus.png"
        clickedCursorPath: themeResourcePath + "/media/pointer-focus-click.png"
    }

    Image {
        id: banner
        z: 1000
        source: themeResourcePath + "/media/Confluence_Logo.png"
    }
}
