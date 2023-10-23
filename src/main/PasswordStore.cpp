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
    QObject(parent),
    jobCount_{0}
{
}

bool PasswordStore::loadPassword(const QUuid& uuid)
{
    auto* job = new QKeychain::ReadPasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));
    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, uuid](QKeychain::Job* j) {
        auto rj = dynamic_cast<QKeychain::ReadPasswordJob*>(j);
        Q_ASSERT(rj);
        auto ok = rj->error() == QKeychain::NoError;
        if (!ok)
            qCWarning(MLR_LOG_CAT) << "Read password exited with error: " << j->error();
        emit passwordLoaded(ok, uuid, QString::fromUtf8(rj->binaryData()));
        --jobCount_;
        if (jobCount_ == 0)
            emit allJobsDone();
    });
    ++jobCount_;
    job->start();
    return true;
}

bool PasswordStore::savePassword(const QUuid& uuid, const QString& passwd)
{
    auto* job = new QKeychain::WritePasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));
    job->setBinaryData(passwd.toUtf8());
    connect(job, &QKeychain::WritePasswordJob::finished, this, [this, uuid](QKeychain::Job* j) {
        auto ok = j->error() == QKeychain::NoError;
        if (!ok)
            qCWarning(MLR_LOG_CAT) << "Write password exited with error: " << j->error();
        emit passwordSaved(ok, uuid);
        --jobCount_;
        if (jobCount_ == 0)
            emit allJobsDone();
    });
    ++jobCount_;
    job->start();
    return true;
}

bool PasswordStore::removePassword(const QUuid& uuid)
{
    auto* job = new QKeychain::DeletePasswordJob(SERVICE_NAME, this);
    job->setAutoDelete(true);
    job->setKey(uuid.toString(QUuid::WithoutBraces));
    connect(job, &QKeychain::DeletePasswordJob::finished, this, [this, uuid](QKeychain::Job* j) {
        auto ok = j->error() == QKeychain::NoError;
        if (!ok)
            qCWarning(MLR_LOG_CAT) << "Remove password exited with error: " << j->error();
        emit passwordRemoved(ok, uuid);
        --jobCount_;
        if (jobCount_ == 0)
            emit allJobsDone();
    });
    ++jobCount_;
    job->start();
    return true;
}

void PasswordStore::waitAllJobsDone(int timeout)
{
    if (jobCount_ == 0)
        return;

    QEventLoop loop;
    QTimer timer;

    if (timeout > 0)
    {
        timer.setSingleShot(true);
        timer.setInterval(timeout);
        timer.start();
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    }

    connect(this, &PasswordStore::allJobsDone, &loop, &QEventLoop::quit);

    loop.exec();
}
