// Copyright 2022 - 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "PasswordStore.h"

#include "Tools.h"
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <qt5keychain/keychain.h>

namespace {

const QString SERVICE_NAME = QStringLiteral("ML-Reader");

} // namespace

PasswordStore::PasswordStore(QObject *parent) :
    QObject(parent)
{
}

bool PasswordStore::loadPassword(const QUuid& uuid, void* context)
{
    auto* job = new QKeychain::ReadPasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));

    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, uuid, context](QKeychain::Job* j) {
        auto rj = dynamic_cast<QKeychain::ReadPasswordJob*>(j);
        Q_ASSERT(rj);

        auto ok = rj->error() == QKeychain::NoError;
        if (!ok && rj->error() != QKeychain::EntryNotFound)
            qCWarning(MLR_LOG_CAT) << "Read password exited with error:" << rj->error();

        emit passwordLoaded(ok, uuid, QString::fromUtf8(rj->binaryData()), context);
    });

    job->start();

    return true;
}

bool PasswordStore::savePassword(const QUuid& uuid, const QString& passwd, void* context)
{
    auto* job = new QKeychain::WritePasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));
    job->setBinaryData(passwd.toUtf8());

    connect(job, &QKeychain::WritePasswordJob::finished, this, [this, uuid, context](QKeychain::Job* j) {
        auto ok = j->error() == QKeychain::NoError;
        if (!ok)
            qCWarning(MLR_LOG_CAT) << "Write password exited with error:" << j->error();

        emit passwordSaved(ok, uuid, context);
    });

    job->start();

    return true;
}

bool PasswordStore::removePassword(const QUuid& uuid, void* context)
{
    auto* job = new QKeychain::DeletePasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));
    connect(job, &QKeychain::DeletePasswordJob::finished, this, [this, uuid, context](QKeychain::Job* j) {
        auto ok = j->error() == QKeychain::NoError;
        if (!ok)
            qCWarning(MLR_LOG_CAT) << "Remove password exited with error:" << j->error();

        emit passwordRemoved(ok, uuid, context);
    });

    job->start();

    return true;
}
