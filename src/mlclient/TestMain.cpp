// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MlClient.h"
#include "Tools.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSslSocket>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qCInfo(MLC_LOG_CAT) <<
        "TLS library built against:" << QSslSocket::sslLibraryBuildVersionString() << "\n" <<
        "TLS library version available:" << QSslSocket::sslLibraryVersionString();

    MlClient client{
        QStringLiteral("http://localhost:8080/mainzelliste.muko"),
        QVersionNumber(2, 2),
        QStringLiteral("mainzelliste"),
    };

    QStringList pids{QStringLiteral("ZPU3P1W3"), QStringLiteral("EL8F3DNC")};
    QStringList fields{QStringLiteral("vorname"), QStringLiteral("nachname")};

    QObject::connect(&client, &MlClient::patientDataLoadingDone, &client,
                     [] (const MlClient::Error& error, const MlClient::PatientData& data) {
        if (error)
        {
            qCInfo(MLC_LOG_CAT) << "ERROR:" << error.message;
        }
        else
        {
            qCInfo(MLC_LOG_CAT) << data;
        }
        QCoreApplication::quit();
    });

    client.loadPatientData(pids, fields);

    return QCoreApplication::exec();
}
