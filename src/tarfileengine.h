/****************************************************************************
 * Copyright (c) 2010 Adriano Tinoco d'Oliveira Rezende
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ****************************************************************************/

#ifndef TARFILEENGINE_H
#define TARFILEENGINE_H

#include <QHash>
#include <QAbstractFileEngine>
#include <QAbstractFileEngineHandler>

struct TarFile
{
    struct Entry
    {
        QString fileName;
        int fileSize;
        bool isDir;
        int chunkOffset;
        QStringList entryList;
    };

    TarFile(const QString &filePath);
    ~TarFile();

    bool load();

    bool isLoaded;
    QString filePath;
    QHash<QString, Entry *> entries;
};


class TarFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QAbstractFileEngine *create(const QString &fileName) const;
};


class TarFileEngine : public QAbstractFileEngine
{
public:
    TarFileEngine(const QString &filePath, const QString &archivePath);
    ~TarFileEngine();

    QString fileName(FileName file = DefaultName) const;
    FileFlags fileFlags(FileFlags type = FileInfoAll) const;

    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    bool open(QIODevice::OpenMode openMode);
    qint64 read(char *data, qint64 maxlen);
    bool close();

    qint64 pos() const;
    bool seek(qint64 offset);
    qint64 size() const;

private:
    FILE *m_handle;
    QString m_tarPath;
    QString m_filePath;
    QString m_fileSuffix;
    TarFile::Entry *m_data;
};


#endif
