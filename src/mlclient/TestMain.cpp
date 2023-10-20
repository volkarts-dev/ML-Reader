// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

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

    QObject::connect(&client, &MlClient::patientDataLoaded, &client,
                     [] (bool result, const MlClient::PatientData& data) {
        qCInfo(MLC_LOG_CAT) << data;
        QCoreApplication::quit();
    });

    client.loadPatientData(pids, fields);

    return QCoreApplication::exec();
}
