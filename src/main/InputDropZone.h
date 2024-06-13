// Copyright 2024, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include <QFrame>

class QMimeData;

class InputDropZone : public QFrame
{
    Q_OBJECT

public:
    explicit InputDropZone(QWidget *parent = nullptr);
    ~InputDropZone();

    Qt::DropActions allowedDropActions() const { return allowedDropActions_; }
    void setAllowedDropActions(Qt::DropActions dropActions) { allowedDropActions_ = dropActions; }

    QVector<QString> allowedMimeTypes() const { return allowedMimeTypes_; }
    void setAllowedMimeTypes(const QVector<QString>& newAllowedMimeTypes) { allowedMimeTypes_ = newAllowedMimeTypes; }

signals:
    void dataDropped(const QMimeData* mimeData);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void setup();
    bool checkActionAllowed(QDropEvent* event) const;

private:
    Qt::DropActions allowedDropActions_;
    QVector<QString> allowedMimeTypes_;
};
