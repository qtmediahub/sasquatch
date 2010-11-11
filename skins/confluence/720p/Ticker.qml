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

BorderImage {
    id: ticker
    border.left: 100

    source: themeResourcePath + "/media/Rss_Back.png"

    width: 850; //height: list.height

    property string currentFeed: "rss.news.yahoo.com/rss/topstories"

    XmlListModel {
        id: feedModel
        source: "http://" + ticker.currentFeed
        query: "/rss/channel/item"

        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "link"; query: "link/string()" }
        XmlRole { name: "description"; query: "description/string()" }
    }

    ListView {
        id: list
        clip: true
        anchors.right : parent.right
        orientation: ListView.Horizontal
        width: parent.width - 40; height: parent.height
        model: feedModel
        delegate: Item {
            id: tickerItem
            width: childrenRect.width; height: parent.height
            ConfluenceText {
                id: tickerTitle;
                //FIXME: offset madness
                y: 5
                height: tickerItem.height
                text: title.replace("\n", "")
                //                verticalAlignment: Text.AlignVCenter
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered:  tickerTitle.color = "blue"
                onExited:  tickerTitle.color = "white"
                onClicked: {
                    webDialog.loadPage(link)
                }
            }
            ConfluenceText { anchors.left: tickerTitle.right; text: " - "; color: "blue" }
        }
    }

    BorderImage {
        source: themeResourcePath + "/media/Rss_Back_Overlay.png"
        border.left: 100
    }
}
