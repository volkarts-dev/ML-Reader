// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QObject>

MlClient* createMlClient(int endpointIndex, const QString& apiKey);
void deleteSenderMlClient(QObject* sender);

template<typename Callback>
inline void mlClientLoadPatientData(MlClient* mlClient, const QStringList& pidList, const QStringList& fieldList,
                                    const typename QtPrivate::FunctionPointer<Callback>::Object* target,
                                    Callback callback)
{
    QObject::connect(mlClient, &MlClient::patientDataLoadingDone, target, callback);

    mlClient->loadPatientData(pidList, fieldList);
}

template<typename Callback>
inline void mlClientQueryPatientData(MlClient* mlClient, const QHash<QString, QString>& patientData, bool sureness,
                                     const typename QtPrivate::FunctionPointer<Callback>::Object* target,
                                     Callback callback)
{
    QObject::connect(mlClient, &MlClient::patientDataQueringDone, target, callback);

    mlClient->queryPatientData(patientData, sureness);
}

template<typename Callback>
inline void mlClientEditPatientData(MlClient* mlClient, const QString& pid, const QHash<QString, QString>& patientData,
                                    const typename QtPrivate::FunctionPointer<Callback>::Object* target,
                                    Callback callback)
{
    QObject::connect(mlClient, &MlClient::patientDataEditingDone, target, callback);

    mlClient->editPatientData(pid, patientData);
}
