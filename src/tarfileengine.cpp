/****************************************************************************
 * Copyright (c) 2010 Adriano Tinoco d'Oliveira Rezende
 * Copyright (c) 2011 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "tarfileengine.h"
#include "refcountcache.h"
#include <QtDebug>

static RefCountCache<TarFile> refCountTarCache;


/*******************************************************************************
 * StringListIterator
 ******************************************************************************/

class StringListIterator : public QAbstractFileEngineIterator
{
public:
    StringListIterator(const QStringList &values, QDir::Filters filters,
                       const QStringList &nameFilters);

    QString next();
    bool hasNext() const;

    QString currentFileName() const;

private:
    int m_index;
    QStringList m_values;
};

StringListIterator::StringListIterator(const QStringList &values, QDir::Filters filters,
                                       const QStringList &nameFilters)
    : QAbstractFileEngineIterator(filters, nameFilters),
      m_index(-1), m_values(values)
{

}

QString StringListIterator::next()
{
    if (!hasNext())
        return QString();

    m_index++;
    return currentFilePath();
}

bool StringListIterator::hasNext() const
{
    return (m_index < m_values.count() - 1);
}

QString StringListIterator::currentFileName() const
{
    return (m_index < 0) ? QString() : m_values.at(m_index);
}


/*******************************************************************************
 * TarFileEngineHandler
 ******************************************************************************/

QAbstractFileEngine *TarFileEngineHandler::create(const QString &fileName) const
{
    if (!fileName.endsWith(".qar", Qt::CaseInsensitive) &&
        fileName.indexOf(".qar/", 0, Qt::CaseInsensitive) <= 0)
        return 0;

    int extPos = fileName.lastIndexOf(".qar", -1, Qt::CaseInsensitive);
    return new TarFileEngine(fileName, fileName.left(extPos + 4));
}

/*******************************************************************************
 * TarFileEngine
 ******************************************************************************/

TarFileEngine::TarFileEngine(const QString &filePath,
                             const QString &archivePath)
    : QAbstractFileEngine(),
      m_handle(0), m_tarPath(archivePath), m_filePath(filePath), m_data(0)
{
    TarFile *tarFile = refCountTarCache.ref(m_tarPath);

    m_fileSuffix = filePath;
    m_fileSuffix.remove(0, m_tarPath.count() + 1);

    // remove trailing slash
    if (m_fileSuffix.endsWith('/'))
        m_fileSuffix.remove(m_fileSuffix.count() - 1, 1);

    // retrieve entry data
    if (m_fileSuffix.isEmpty())
        m_data = tarFile->entries.value(".", 0);
    else
        m_data = tarFile->entries.value(m_fileSuffix, 0);
}

TarFileEngine::~TarFileEngine()
{
    refCountTarCache.unref(m_tarPath);
}

QAbstractFileEngine::FileFlags TarFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    Q_UNUSED(type);

    if (!m_data)
        return 0;

    // file perms are ignored
    QAbstractFileEngine::FileFlags result = QAbstractFileEngine::ExistsFlag |
        QAbstractFileEngine::ReadOtherPerm;

    if (!m_data->isDir)
        result |= QAbstractFileEngine::FileType;
    else
        result |= QAbstractFileEngine::DirectoryType;

    return result;
}

bool TarFileEngine::open(QIODevice::OpenMode openMode)
{
    if (!m_data || !(openMode & QIODevice::ReadOnly))
        return false;

    m_handle = fopen(m_tarPath.toLatin1().data(), "rb");

    if (m_handle) {
        // seek file chunk offset
        fseek(m_handle, m_data->chunkOffset, SEEK_SET);
    }

    return bool(m_handle);
}

qint64 TarFileEngine::read(char *data, qint64 maxlen)
{
    if (!m_data || !m_handle)
        return 0;

    const qint64 end = m_data->fileSize + m_data->chunkOffset;
    return fread(data, 1, qMin<int>(end - ftell(m_handle), maxlen), m_handle);
}

bool TarFileEngine::close()
{
    if (!m_data || !m_handle)
        return false;

    bool result = (fclose(m_handle) == 0);
    m_handle = 0;
    return result;
}

qint64 TarFileEngine::pos() const
{
    if (!m_data || !m_handle)
        return -1;

    return (ftell(m_handle) - m_data->chunkOffset);
}

qint64 TarFileEngine::size() const
{
    return (!m_data) ? 0 : m_data->fileSize;
}

bool TarFileEngine::seek(qint64 offset)
{
    if (!m_data || !m_handle)
        return false;

    qint64 localOffset = qMin<qint64>(m_data->chunkOffset + offset,
                                      m_data->chunkOffset + m_data->fileSize);

    return fseek(m_handle, localOffset, SEEK_SET) == 0;
}

QStringList TarFileEngine::entryList(QDir::Filters filters,
                                     const QStringList &filterNames) const
{
    Q_UNUSED(filters);
    Q_UNUSED(filterNames);
    return m_data->entryList;
}

QAbstractFileEngine::Iterator *TarFileEngine::beginEntryList(QDir::Filters filters,
                                                             const QStringList &filterNames)
{
    return new StringListIterator(m_data->entryList, filters, filterNames);
}

QString TarFileEngine::fileName(FileName file) const
{
    int count = m_filePath.count();
    int index = m_filePath.lastIndexOf('/');

    // TODO

    switch(file) {
    case QAbstractFileEngine::BaseName:
        if (index < 0)
            return m_filePath;
        else
            return m_filePath.right(count - index - 1);
        break;
    case QAbstractFileEngine::DefaultName:
        return m_filePath;
        break;

    default:
        return QString();
    }
}

/*******************************************************************************
 * TarFile
 ******************************************************************************/


TarFile::TarFile(const QString &filePath)
    : isLoaded(false),
      filePath(filePath)
{
    load();
}

TarFile::~TarFile()
{
    qDeleteAll(entries);
}

bool TarFile::load()
{
    if (isLoaded || filePath.isEmpty())
        return false;

    FILE *fp = fopen(filePath.toLatin1().data(), "rb");

    if (!fp)
        return false;

    const char FILE_NODE = '0';
    const char DIR_NODE = '5';
    const int BLOCK_SIZE = 512;

    bool ok;
    int chunkSize;
    QString fileName;
    char buffer[BLOCK_SIZE];
    QList<Entry *> entryList;

    Entry *rootEntry = new Entry;
    rootEntry->isDir = true;
    rootEntry->fileName = filePath;
    entries.insert(".", rootEntry);

    for (;;) {
        int read = fread(buffer, 1, BLOCK_SIZE, fp);

        if (read == 0 || *buffer == 0)
            break;

        const char type = buffer[156];
        const int fileSize = QString(&buffer[124]).toInt(&ok, 8);

        if (fileSize % BLOCK_SIZE == 0)
            chunkSize = fileSize;
        else
            chunkSize = int(fileSize / BLOCK_SIZE) * BLOCK_SIZE + BLOCK_SIZE;

        if (type != DIR_NODE && type != FILE_NODE) {
            // ignore entries that are not dirs or files
            fseek(fp, chunkSize, SEEK_CUR);
            continue;
        }

        fileName = QString::fromLatin1(buffer);

        // remove trailing slash
        if (fileName.endsWith('/'))
            fileName.remove(fileName.count() - 1, 1);

        // Drop the root-dir
        fileName = fileName.mid(fileName.indexOf('/')+1);
        fileName.replace("//", "/");

        // avoid duplicate entries
        if (!entries.contains(fileName)) {
            Entry *data = new Entry;
            data->fileName = fileName;
            data->fileSize = fileSize;
            data->chunkOffset = ftell(fp);
            data->isDir = (type == DIR_NODE);

            entryList.append(data);
            entries.insert(fileName, data);
        }

        fseek(fp, chunkSize, SEEK_CUR);
    }

    // setup dirs entry lists
    foreach (Entry *data, entryList) {
        const QStringList &parts = data->fileName.split('/');

        QString path = parts[0];

        if (parts.count() == 1) {
            // root dirs/files
            rootEntry->entryList.append(path);
            continue;
        }

        for (int i = 1; i < parts.count(); i++) {
            Entry *entry = entries.value(path, 0);

            if (!entry)
                continue;

            const QString &subpath = parts.at(i);

            if (!entry->entryList.contains(subpath))
                entry->entryList.append(subpath);

            path.append("/" + subpath);
        }
    }

    fclose(fp);

    isLoaded = true;
    return true;
}
