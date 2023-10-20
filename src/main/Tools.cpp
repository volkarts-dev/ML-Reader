// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "Tools.h"

Q_LOGGING_CATEGORY(MLR_LOG_CAT, "va.mlreader", QtMsgType::QtDebugMsg)

int indexClamp(int value, int max)
{
    return qMin(max, qMax(-1, value));
}
