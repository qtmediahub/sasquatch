/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
**
****************************************************************************/

import QtQuick 2.0

ShaderEffect {
    id: effect
    property variant image: 0
    property variant mask: 0

    fragmentShader:
        "
        varying highp vec2 qt_TexCoord0;
        uniform sampler2D image;
        uniform sampler2D mask;
        void main(void)
        {
            gl_FragColor = texture2D(image, qt_TexCoord0.st) * (texture2D(mask, qt_TexCoord0.st).a);
        }
        "
}
