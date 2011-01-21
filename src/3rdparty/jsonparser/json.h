/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QVariant>

namespace Json {
    QVariant parse(const QByteArray &data, QString *error = 0);
    QByteArray stringify(const QVariant &variant);
};

#endif // JSON_H

