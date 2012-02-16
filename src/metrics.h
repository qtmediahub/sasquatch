#include <qdebug.h>
#include <qdeclarative.h>
#include <QFile>
#include <QStringList>
#ifdef QT5
#include <QGuiApplication>
#else
#include <QApplication>
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
