//
// Created by DeweiZhu on 2026/5/9.
//

#include "HashStore.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>

HashStore::HashStore()
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty())
    {
        baseDir = QDir::currentPath();
    }

    QDir dir(baseDir);
    dir.mkpath(".");
    filePath_ = dir.filePath("downloaded_url_hashes.txt");
}

QString HashStore::recordPath() const
{
    return filePath_;
}

QSet<QString> HashStore::load() const
{
    QSet<QString> hashes;
    QFile f(filePath_);
    if (!f.exists())
    {
        return hashes;
    }
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return hashes;
    }

    QTextStream in(&f);
    while (!in.atEnd())
    {
        const QString line = in.readLine().trimmed();
        if (!line.isEmpty())
        {
            hashes.insert(line);
        }
    }
    return hashes;
}

bool HashStore::append(const QString& hash) const
{
    QFile f(filePath_);
    if (!f.open(QIODevice::Append | QIODevice::Text))
    {
        return false;
    }

    QTextStream out(&f);
    out << hash << "\n";
    return true;
}

bool HashStore::clear() const
{
    return !QFile::exists(filePath_) || QFile::remove(filePath_);
}