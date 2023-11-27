// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QObject>

MlClient* createMlClient(int endpointIndex, const QString& apiKey);
void deleteSenderMlClient(QObject* sender);

template<typename SuccessFunc, typename FailedFunc>
inline void mlClientLoadPatientData(MlClient* mlClient, const QStringList& pidList, const QStringList& fieldList,
                                    const typename QtPrivate::FunctionPointer<SuccessFunc>::Object* target,
                                    SuccessFunc successFunc, FailedFunc failedFunc)
{
    QObject::connect(mlClient, &MlClient::patientDataLoaded, target, successFunc);
    QObject::connect(mlClient, &MlClient::patientDataLoadingFailed, target, failedFunc);

    mlClient->loadPatientData(pidList, fieldList);
}

template<typename SuccessFunc, typename FailedFunc>
inline void mlClientQueryPatientData(MlClient* mlClient, const QHash<QString, QString>& patientData, bool sureness,
                                 const typename QtPrivate::FunctionPointer<SuccessFunc>::Object* target,
                                 SuccessFunc successFunc, FailedFunc failedFunc)
{
    QObject::connect(mlClient, &MlClient::patientDataQueried, target, successFunc);
    QObject::connect(mlClient, &MlClient::patientDataQueringFailed, target, failedFunc);

    mlClient->queryPatientData(patientData, sureness);
}

template<typename SuccessFunc, typename FailedFunc>
inline void mlClientEditPatientData(MlClient* mlClient, const QString& pid, const QHash<QString, QString>& patientData,
                                     const typename QtPrivate::FunctionPointer<SuccessFunc>::Object* target,
                                     SuccessFunc successFunc, FailedFunc failedFunc)
{
    QObject::connect(mlClient, &MlClient::patientDataEdited, target, successFunc);
    QObject::connect(mlClient, &MlClient::patientDataEditingFailed, target, failedFunc);

    mlClient->editPatientData(pid, patientData);
}
