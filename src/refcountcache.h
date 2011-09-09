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

#ifndef REFCOUNTCACHE_H
#define REFCOUNTCACHE_H

#include <QMutexLocker>


template <class T>
class RefCountCache
{
public:
    RefCountCache() { }

    T *ref(const QString &key) {
        QMutexLocker locker(&mutex);

        if (!values.contains(key)) {
            T *result = new T(key);
            values.insert(key, qMakePair(1, result));
            return result;
        }

        const QPair<int, T*> &p = values.value(key);
        values.insert(key, qMakePair(p.first + 1, p.second));
        T *result = p.second;

        return result;
    }

    bool unref(const QString &key) {
        QMutexLocker locker(&mutex);

        if (!values.contains(key))
            return false;

        const QPair<int, T*> &p = values.value(key);

        if (p.first > 1)
            values.insert(key, qMakePair(p.first - 1, p.second));
        else {
            delete p.second;
            values.remove(key);
        }

        return true;
    }

private:
    mutable QMutex mutex;
    QHash<QString, QPair<int, T*> > values;
};

#endif
