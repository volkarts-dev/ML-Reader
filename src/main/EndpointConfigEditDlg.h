// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>

namespace Ui {
class EndpointConfigEditDlg;
}

class EndpointConfigItemDelegate;
class EndpointConfigModel;
class QDataWidgetMapper;
class QItemSelection;

class EndpointConfigEditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EndpointConfigEditDlg(QWidget* parent = nullptr);
    ~EndpointConfigEditDlg() override;

public slots:
    void accept() override;

private slots:
    void onAddButtonClicked();
    void onRemoveButtonClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void setup();

private:
    Ui::EndpointConfigEditDlg* ui;
    QDataWidgetMapper* mapper_;
    EndpointConfigItemDelegate* itemDelegate_;

    Q_DISABLE_COPY_MOVE(EndpointConfigEditDlg)
};
