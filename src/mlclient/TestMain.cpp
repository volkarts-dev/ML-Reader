// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MlClient.h"
#include "Tools.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    MlClient client{
        QStringLiteral("http://localhost:8080/mainzelliste.muko"),
        QVersionNumber(2, 2),
        QStringLiteral("mainzelliste"),
    };

    QStringList pids{"ZPU3P1W3", "EL8F3DNC"};
    QStringList fields{"vorname", "nachname"};

    QObject::connect(&client, &MlClient::patientDataLoadingFailed, &client,
                     [] (const QString& error) {
        qCInfo(MLC_LOG_CAT) << "ERROR:" << error;
        QCoreApplication::quit();
    });

    QObject::connect(&client, &MlClient::patientDataLoaded, &client,
                     [] (const MlClient::PatientData& data) {
        qCInfo(MLC_LOG_CAT) << data;
        QCoreApplication::quit();
    });

    client.loadPatientData(pids, fields);

    return QCoreApplication::exec();
}
