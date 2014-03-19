/*
 * Copyright (C) 2010 Johannes Zellner <webmaster@nebulon.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

import QtQuick 2.0
import MediaModel 1.0

Rectangle {
    id: plattenbau

    height: 720
    width: 1280

    color: "black"

    property string themeResourcePath: runtime.skin.path

    function initialize() {
        if (typeof cursor  != 'undefined') {
            cursor.idleTimeout = 2
            cursor.defaultCursorPath = themeResourcePath + "/media/pointer-focus.png"
            cursor.clickedCursorPath = themeResourcePath + "/media/pointer-focus-click.png"
        }
    }

    ContentGrid {
        id: contentGrid
        anchors.left: parent.left
        anchors.leftMargin: 20
        height: parent.height
    }

    ApplicationList {
        id: applicationList
        anchors.right: parent.right
        anchors.rightMargin: 20
    }

    Component.onCompleted: {
        initialize();
    }
}
