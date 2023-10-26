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

    bool loadPassword(const QUuid& uuid);
    bool savePassword(const QUuid& uuid, const QString& passwd);
    bool removePassword(const QUuid& uuid);

signals:
    void passwordLoaded(bool result, const QUuid& uuid, const QString& passwd);
    void passwordSaved(bool result, const QUuid& uuid);
    void passwordRemoved(bool result, const QUuid& uuid);
};
