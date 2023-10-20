// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "CsvReader.h"

#include "Application.h"
#include "CsvRawData.h"
#include "Configuration.h"
#include "DataModel.h"
#include <QIODevice>
#include <QTextStream>
#include <qtcsv/reader.h>
#include <qtcsv/writer.h>

CsvReader::CsvReader(QObject* parent) :
    QObject{parent}
{
}

CsvReader::~CsvReader()
{
}

bool CsvReader::read(QIODevice& input, DataModel* model)
{
    auto cfg = app()->configuration();

    CsvRawData dataAdapter{};

    auto codec = QTextCodec::codecForName(cfg->stringValue(Configuration::Key::CsvColumnSeparator).toLatin1());
    auto result = QtCSV::Reader::readToData(
                input, dataAdapter, cfg->stringValue(Configuration::Key::CsvColumnSeparator),
                cfg->stringValue(Configuration::Key::CsvQuotingCharacter), codec);

    if (result)
        model->setModelData(dataAdapter.data());
    else
        model->setModelData({});

    return result;
}

bool CsvReader::write(QIODevice& output, bool withHeader, DataModel* model)
{
    auto cfg = app()->configuration();

    CsvRawData dataAdapter{model->modelData(), !withHeader};

    auto codec = QTextCodec::codecForName(cfg->stringValue(Configuration::Key::CsvColumnSeparator).toLatin1());
    return QtCSV::Writer::write(
                output, dataAdapter, cfg->stringValue(Configuration::Key::CsvColumnSeparator),
                cfg->stringValue(Configuration::Key::CsvQuotingCharacter), {}, {}, codec);
}
