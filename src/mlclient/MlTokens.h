// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QStringList>
#include <QVersionNumber>

class QJsonObject;

QJsonObject makeReadPatientToken(const QVersionNumber& apiVersion, const QStringList& pids, const QStringList& fields);
QJsonObject makeCreatePatientToken(const QVersionNumber& apiVersion);
QJsonObject makeEditPatientToken(const QVersionNumber& apiVersion, const QString& pid);
