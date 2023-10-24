// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include <QVariant>

class MainInterface
{
public:
    enum class Page
    {
        Loader,
        Query,
        Editor,
    };

public:
    virtual ~MainInterface() = default;

    virtual void showStatusMessage(const QString& message, int timeout = 0) = 0;

    virtual void openPage(Page page, const QVariant& openData = {}) = 0;
};
