// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QLatin1String>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MLC_LOG_CAT)

inline QLatin1String operator""_l1(const char* string, size_t size)
{
    return QLatin1String{string, static_cast<int>(size)};
}
