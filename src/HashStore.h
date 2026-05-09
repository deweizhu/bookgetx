//
// Created by DeweiZhu on 2026/5/9.
//

#ifndef BOOKGET_HASHSTORE_H
#define BOOKGET_HASHSTORE_H

#pragma once

#include <QtCore/QString>
#include <QtCore/QSet>

class HashStore
{
public:
    HashStore();

    QString recordPath() const;
    QSet<QString> load() const;
    bool append(const QString& hash) const;
    bool clear() const;

private:
    QString filePath_;
};
#endif //BOOKGET_HASHSTORE_H