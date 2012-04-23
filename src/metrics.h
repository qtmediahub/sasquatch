/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include <qdebug.h>
#include <QFile>
#include <QStringList>
#ifdef QT5
#include <qqml.h>
#include <QGuiApplication>
#include <qqml.h>
#else
#include <QApplication>
#include <qdeclarative.h>
#endif

class Metrics : public QObject
{
    Q_OBJECT

private:
    enum ProcStatMetric {
        Utime = 13,
        Stime = 14,
        ThreadCount = 19,
        Vsize = 22,
        Rss = 23,
        StatCount
    };

    static int procPIDstat(ProcStatMetric metric)
    {
        int ret = -1;
        QFile logFile("/proc/" + QVariant(qApp->applicationPid()).toString() + "/stat");
        if (!logFile.open(QFile::ReadOnly))
            return -1;

        QStringList entries = QString(logFile.readLine()).split(' ');

        if (metric < entries.length())
            ret = entries.at(metric).toInt();

        return ret;
    }
public:
    Q_INVOKABLE static int threadCount() {
        return procPIDstat(ThreadCount);
    }

    Q_INVOKABLE static int swaplogFPS()
    {
        int fps;
        QFile logFile("/tmp/swaplog");
        if (!logFile.open(QFile::ReadOnly))
            return -1;

        int lineCount = 0;

        char line[1000];
        int offset = sizeof(line);
        uchar* map = logFile.map(0, logFile.size());
        for (int i = logFile.size() - 1; i >= 0; i--) {
            if (map[i] == '\n') {
                ++lineCount;
                if (lineCount == 3) {
                    break;
                }
                offset = sizeof(line);
            } else if (offset) {
                line[--offset] = map[i];
            }
        }

        QByteArray rawData = QByteArray::fromRawData(line + offset, sizeof(line) - offset);

        offset = rawData.indexOf("apfs_64:");
        if (offset != -1) {
            fps = atoi(rawData.constData() + offset + sizeof("apfs_64:") - 1);
        }

        logFile.unmap(map);
        logFile.close();
        return fps;
    }
};

QML_DECLARE_TYPE(Metrics)
