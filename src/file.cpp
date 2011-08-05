#include "file.h"

File::File(QObject *parent)
    : QObject(parent)
{
}

QStringList File::readAllLines(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textContent(&file);
    QStringList lines;
    int emptyLineCount = 0;
    while(!(file.isSequential() && textContent.atEnd())
          && (emptyLineCount < 5))
    {
        QString currentLine = textContent.readLine();
        if(currentLine.isEmpty())
            emptyLineCount++;
        else
            emptyLineCount = 0;
        lines << currentLine;
    }
    //Trim, especially important for virtual files
    for(int i = 0; i < emptyLineCount; i++)
        lines.removeLast();

    return lines;
}

QStringList File::findApplications() const
{
    QStringList apps;
    foreach(const QString &appSearchPath, LibraryInfo::applicationPaths()) {
        QStringList subdirs = QDir(appSearchPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach(const QString &subdir, subdirs)  {
            QString appPath(appSearchPath + '/' + subdir + '/');
            QString fileName(appPath + "qmhmanifest.qml"); // look for apps/x/qmhmanifest.qml
            QFile prospectiveFile(fileName);
            if (prospectiveFile.exists())
                apps << (QDir(appPath).absolutePath() + '/');
        }
    }
    return apps;
}

