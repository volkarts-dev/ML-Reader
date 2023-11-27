// Copyright 2022 - 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QUuid>

class PasswordStore : public QObject
{
    Q_OBJECT

public:
    explicit PasswordStore(QObject* parent = {});

    bool loadPassword(const QUuid& uuid, void* context = {});
    bool savePassword(const QUuid& uuid, const QString& passwd, void* context = {});
    bool removePassword(const QUuid& uuid, void* context = {});

signals:
    void passwordLoaded(bool result, const QUuid& uuid, const QString& passwd, void* context = {});
    void passwordSaved(bool result, const QUuid& uuid, void* context = {});
    void passwordRemoved(bool result, const QUuid& uuid, void* context = {});
};
