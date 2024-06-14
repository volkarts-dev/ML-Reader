// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QLatin1String>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MLR_LOG_CAT)

class QSplitter;
class UserSettings;

inline int indexClamp(int value, int max, int min = -1)
{
    return qMin(max, qMax(min, value));
}

template<typename T>
inline int toInt(T value)
{
    return static_cast<int>(value);
}

inline QLatin1String operator""_l1(const char* string, size_t size)
{
    return QLatin1String{string, static_cast<int>(size)};
}

QByteArray saveSplitterState(const QSplitter* splitter);
bool restoreSplitterState(QSplitter* splitter, const QByteArray& data);
