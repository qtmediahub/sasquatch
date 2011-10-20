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

import QtQuick 1.0
import Qt.labs.shaders 1.0

ShaderEffectItem {
    id: effect
    property real ratio: 1.0
    property variant source: 0

    fragmentShader:
        "
        varying highp vec2 qt_TexCoord0;
        uniform sampler2D source;
        uniform highp float ratio;
        void main(void)
        {
            lowp vec4 textureColor = texture2D(source, qt_TexCoord0.st);
            lowp float gray = dot(textureColor, vec4(0.299, 0.587, 0.114, 0.0));
            gl_FragColor = vec4(gray * ratio + textureColor.r * (1.0 - ratio), gray * ratio + textureColor.g * (1.0 - ratio), gray * ratio + textureColor.b * (1.0 - ratio), textureColor.a);
        }
        "
}
