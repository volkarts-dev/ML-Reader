// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

class HttpUserDelegate
{
public:
    virtual ~HttpUserDelegate() = default;

    virtual bool askRecoverableError(const QString& title, const QString& message) = 0;
};
