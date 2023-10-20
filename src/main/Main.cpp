// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "Application.h"

int main(int argc, char** argv)
{
    Application app(argc, argv);
    if (!app.initialize())
        return 1;
    return app.exec();
}
