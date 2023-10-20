// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "Configuration.h"

#include <QAbstractTableModel>
#include <QDataWidgetMapper>
#include <QSettings>
#include <QTextCodec>

namespace {
/*
class Model : public QAbstractTableModel
{
public:
    static constexpr int PrefixRole = Qt::UserRole;

public:
    Model(Configuration* csvConfig, QObject* parent = nullptr) :
        QAbstractTableModel(parent),
        csvConfig_(csvConfig)
    {
    }

    int rowCount(const QModelIndex& parent) const override { return 1; }
    int columnCount(const QModelIndex& parent) const override { return static_cast<int>(Configuration::Key::_Count); }
    Qt::ItemFlags flags(const QModelIndex& index) const override { return QAbstractTableModel::flags(index) | Qt::ItemIsEditable; }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
    Configuration* csvConfig_;
};

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (static_cast<Configuration::Key>(section))
        {
            using enum Configuration::Key;

            case LastAccessedDirectory:
                if (role == PrefixRole)
                    return QStringLiteral("Main/");
                else if (role == Qt::DisplayRole)
                    return QStringLiteral("LastAccessedDirectory");
                break;

            case CsvColumnSeparator:
                if (role == PrefixRole)
                    return QStringLiteral("Main/CSV/");
                else if (role == Qt::DisplayRole)
                    return QStringLiteral("ColumnSeparator");
                break;

            case CsvQuotingCharacter:
                if (role == PrefixRole)
                    return QStringLiteral("Main/CSV/");
                else if (role == Qt::DisplayRole)
                    return QStringLiteral("QuotingCharacter");
                break;

            case CsvCodec:
                if (role == PrefixRole)
                    return QStringLiteral("Main/CSV/");
                else if (role == Qt::DisplayRole)
                    return QStringLiteral("Codec");
                break;

            case _Count:
                break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::EditRole)
        return {};

    switch (static_cast<Configuration::Key>(index.column()))
    {
        using enum Configuration::Key;

        case LastAccessedDirectory:
            return csvConfig_->lastAccessedDirectory_;
            break;

        case CsvColumnSeparator:
            return csvConfig_->csvColumnSeparator;
            break;

        case CsvQuotingCharacter:
            return csvConfig_->csvQuotingCharacter;
            break;

        case CsvCodec:
            return csvConfig_->csvCodec->name();
            break;

        case _Count:
            break;
    }

    return {};
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return {};

    switch (static_cast<Configuration::Key>(index.column()))
    {
        using enum Configuration::Key;

        case LastAccessedDirectory:
            csvConfig_->lastAccessedDirectory_ = value.toString();
            break;

        case CsvColumnSeparator:
            csvConfig_->csvColumnSeparator = value.toString();
            break;

        case CsvQuotingCharacter:
            csvConfig_->csvQuotingCharacter = value.toString();
            break;

        case CsvCodec:
            csvConfig_->csvCodec = QTextCodec::codecForName(value.toByteArray());
            break;

        case _Count:
            break;
    }

    emit dataChanged(index, index);

    return true;
}
*/
} // namespace

Configuration::Configuration()
{
}

//QDataWidgetMapper* Configuration::createWidgetMapper(QObject* parent)
//{
//    auto mo = new Model(this, parent);
//    auto dm = new QDataWidgetMapper(parent);
//    dm->setModel(mo);
//    return dm;
//}

void Configuration::setValue(Key key, const QVariant& value)
{
    QSettings s;

    switch (key)
    {
        using enum Configuration::Key;

        case LastAccessedDirectory:
            s.setValue(QStringLiteral("Main/LastAccessedDirectory"), value);
            break;

        case CsvColumnSeparator:
            s.setValue(QStringLiteral("Main/Csv/ColumnSeparator"), value);
            break;

        case CsvQuotingCharacter:
            s.setValue(QStringLiteral("Main/Csv/QuotingCharacter"), value);
            break;

        case CsvCodec:
            s.setValue(QStringLiteral("MainCsv/Codec"), value);
            break;

        case _Count:
            break;
    }
}

QVariant Configuration::value(Key key) const
{
    QSettings s;

    switch (key)
    {
        using enum Configuration::Key;

        case LastAccessedDirectory:
            return s.value(QStringLiteral("Main/LastAccessedDirectory"), QString{});

        case CsvColumnSeparator:
            return s.value(QStringLiteral("Main/Csv/ColumnSeparator"), QStringLiteral(";"));

        case CsvQuotingCharacter:
            return s.value(QStringLiteral("Main/Csv/QuotingCharacter"), QStringLiteral("\""));

        case CsvCodec:
            return s.value(QStringLiteral("MainCsv/Codec"), QStringLiteral("ISO 8859-15"));

        case _Count:
            break;
    }

    return {};
}

QString Configuration::stringValue(Key key) const
{
    return value(key).toString();
}

int Configuration::intValue(Key key) const
{
    return value(key).toInt();
}

bool Configuration::boolValue(Key key) const
{
    return value(key).toBool();
}
