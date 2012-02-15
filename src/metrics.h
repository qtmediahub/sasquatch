#include <QFile>

namespace Metrics
{
    static int swaplogFPS()
    {
        int fps;
        QFile logFile("/tmp/swaplog");
        if (!logFile.open(QFile::ReadOnly))
            return;

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
