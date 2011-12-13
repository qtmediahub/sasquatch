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

    property real wave: 0.3
    property real waveOriginX: 0.5
    property real waveOriginY: 0.5
    property real waveWidth: 0.01
    property real aspectRatio: width/height
    property variant source: 0

    fragmentShader:
        "
        varying mediump vec2 qt_TexCoord0;
        uniform sampler2D source;
        uniform highp float wave;
        uniform highp float waveWidth;
        uniform highp float waveOriginX;
        uniform highp float waveOriginY;
        uniform highp float aspectRatio;

        void main(void)
        {
        mediump vec2 texCoord2 = qt_TexCoord0;
        mediump vec2 origin = vec2(waveOriginX, (1.0 - waveOriginY) / aspectRatio);

        highp float fragmentDistance = distance(vec2(texCoord2.s, texCoord2.t / aspectRatio), origin);
        highp float waveLength = waveWidth + fragmentDistance * 0.25;

        if ( fragmentDistance > wave && fragmentDistance < wave + waveLength) {
            highp float distanceFromWaveEdge = min(abs(wave - fragmentDistance), abs(wave + waveLength - fragmentDistance));
            texCoord2 += sin(1.57075 * distanceFromWaveEdge / waveLength) * distanceFromWaveEdge * 0.08 / fragmentDistance;
        }

        gl_FragColor = texture2D(source, texCoord2.st);
        }
        "
}
