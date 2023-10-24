// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "Tools.h"

Q_LOGGING_CATEGORY(MLR_LOG_CAT, "va.mlreader", QtMsgType::QtDebugMsg)

int indexClamp(int value, int max, int min)
{
    return qMin(max, qMax(min, value));
}
