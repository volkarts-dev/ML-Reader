// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include <QStringList>
#include <QVersionNumber>

class QJsonObject;

QJsonObject makeReadPatientToken(const QVersionNumber& apiVersion, QStringList pids, QStringList fields);
