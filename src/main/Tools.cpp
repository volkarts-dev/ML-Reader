// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "Tools.h"

#include <QDataStream>
#include <QSplitter>

Q_LOGGING_CATEGORY(MLR_LOG_CAT, "va.mlreader", QtMsgType::QtDebugMsg)

QByteArray saveSplitterState(const QSplitter* splitter)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << splitter->sizes();

    return data;
}

bool restoreSplitterState(QSplitter* splitter, const QByteArray& data)
{
    QByteArray wd = data;
    QDataStream stream(&wd, QIODevice::ReadOnly);

    QList<int> sizes;
    stream >> sizes;

    if (splitter->count() > sizes.count())
        return false;

    splitter->setSizes(sizes);

    return true;
}
