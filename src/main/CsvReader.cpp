// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "CsvReader.h"

#include "CsvRawData.h"
#include "DataModel.h"
#include "UserSettings.h"
#include <QIODevice>
#include <QTextStream>
#include <qtcsv/reader.h>
#include <qtcsv/writer.h>

CsvReader::CsvReader(QObject* parent) :
    QObject{parent}
{
}

bool CsvReader::read(QIODevice& input, DataModel* model)
{
    UserSettings s;

    CsvRawData dataAdapter{};

    auto codec = QStringConverter::encodingForName(s.stringValue(CfgCsvCodec).toUtf8());
    if (!codec.has_value())
        codec = QStringConverter::encodingForName(CfgCsvDefaultCodec.toUtf8());
    Q_ASSERT(codec.has_value());

    auto result = QtCSV::Reader::readToData(
                input, dataAdapter, s.stringValue(CfgCsvColumnSeparator),
                s.stringValue(CfgCsvQuotingCharacter), codec.value());

    if (result)
        model->setModelData(dataAdapter.data());
    else
        model->setModelData({});

    return result;
}

bool CsvReader::write(QIODevice& output, bool withHeader, DataModel* model)
{
    UserSettings s;

    CsvRawData dataAdapter{model->modelData(), !withHeader};

    auto codec = QStringConverter::encodingForName(s.stringValue(CfgCsvColumnSeparator).toUtf8());
    if (!codec.has_value())
        codec = QStringConverter::encodingForName(CfgCsvDefaultCodec.toUtf8());
    Q_ASSERT(codec.has_value());

    return QtCSV::Writer::write(
                output, dataAdapter, s.stringValue(CfgCsvColumnSeparator),
                s.stringValue(CfgCsvQuotingCharacter), {}, {}, codec.value());
}
