// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

class QString;

class MessageView
{
public:
    virtual ~MessageView() = default;

    virtual void showStatusMessage(const QString& message, int timeout = 0) = 0;
};
