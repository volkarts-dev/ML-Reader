// Copyright 2024, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "InputDropZone.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

InputDropZone::InputDropZone(QWidget *parent) :
    QFrame(parent)
{
    setup();
}

InputDropZone::~InputDropZone() = default;

void InputDropZone::setup()
{
    setAcceptDrops(true);
}

bool InputDropZone::checkActionAllowed(QDropEvent* event) const
{
    bool foundMimeType = false;
    for (const auto& amt : allowedMimeTypes_)
    {
        if (event->mimeData()->hasFormat(amt))
        {
            foundMimeType = true;
            break;
        }
    }
    if (!foundMimeType)
        return false;

    if (!(allowedDropActions_ & event->proposedAction()))
        return false;

    event->acceptProposedAction();
    return true;
}

void InputDropZone::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void InputDropZone::dropEvent(QDropEvent* event)
{
    if (checkActionAllowed(event))
    {
        emit dataDropped(event->mimeData());
    }
}
