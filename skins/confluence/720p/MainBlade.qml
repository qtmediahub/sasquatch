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
import "components"

Blade {
    id: mainBlade
    clip: false

    Keys.onLeftPressed:
        buttonGrid.focusUpperItem()

    bladeWidth: 400
    bladePixmap: themeResourcePath + "/media/HomeBlade.png"

    property variant audioLoader: Qt.createComponent("QMHAudio.qml");
    property variant clickSample

    ListModel {
        //Eventually plugins
        id: menuList
        ListElement {
            name: "Scripts"
            backgroundImage: "programs.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Weather"
            backgroundImage: "weather.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Pictures"
            backgroundImage: "pictures.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Videos"
            backgroundImage: "videos.jpg"
            mHasSubBlade: true
        }
        ListElement {
            name: "Music"
            backgroundImage: "music.jpg"
            mHasSubBlade: true
        }
        ListElement {
            name: "Programs"
            backgroundImage: "programs.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "System"
            backgroundImage: "system.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Web"
            backgroundImage: "system.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Maps"
            backgroundImage: "system.jpg"
            mHasSubBlade: false
        }
        ListElement {
            name: "Dashboard"
            backgroundImage: "system.jpg"
            mHasSubBlade: false
        }
    }

    Item {
        id: bannerPlaceHolder
        height: banner.height
    }

    ListView {
        id: mainBladeList

        signal itemTriggered
        signal itemSelected

        //Oversized fonts being downscaled
        spacing: -30
        focus: true
        keyNavigationWraps: true
        //highlightFollowsCurrentItem: true

        anchors { right: blade.right; rightMargin: 30; top: bannerPlaceHolder.bottom }

        height: parent.height - 200
        width: bladeWidth

        model: menuList
        delegate:
            BladeListItem { }
        onItemTriggered: {
            if(currentItem.text == "Weather")
                confluence.state = "showingWeatherDialog"
            if(currentItem.text == "System")
                confluence.state = "showingSystemDialog"
            if(currentItem.text == "Web")
                confluence.state = "showingWebDialog"
            if(currentItem.text == "Maps")
                confluence.state = "showingMapsDialog"
            if(currentItem.text == "Dashboard")
                confluence.state = "showingDashboard"
        }
        onItemSelected: {
            background.asyncSetImage(currentItem.background)
            if(clickSample == undefined) {
                if(audioLoader.status == Loader.Ready) {
                    clickSample = audioLoader.createObject(mainBladeList)
                    clickSample.source = themeResourcePath + "/sounds/click.wav"
                }
            } else {
                clickSample.play()
            }
        }
        Keys.onEnterPressed:
            itemTriggered()
        Keys.onReturnPressed:
            itemTriggered()
        Keys.onRightPressed:
            if(currentItem.hasSubBlade)
                mediaBlade.state = "open"
    }

    ButtonList {
        id: buttonGrid
        x: mainBlade.bladeX + 5; y: 650;
        spacing: 2

        onUpperBoundExceeded: {
            mainBladeList.focus = true
        }

        PixmapButton { basePixmap: "home-playmedia"; focusedPixmap: "home-playmedia-FO" }
        PixmapButton { basePixmap: "home-favourites"; focusedPixmap: "home-favourites-FO" }
        PixmapButton {
            basePixmap: "home-power";
            focusedPixmap: "home-power-FO";
            onClicked:
                confluence.state = "showingExitDialog"
        }
    }

    Blade {
        id: mediaBlade
        bladePeek: 10
        bladeWidth: 200

        //Magical pixmap offset again
        anchors { left: mainBlade.right; leftMargin: -10 }

        bladePixmap: themeResourcePath + "/media/MediaBladeSub.png"

        Keys.onLeftPressed: {
            state = "closed"
            mainBladeList.forceActiveFocus()
        }
    }
}
