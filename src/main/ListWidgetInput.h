#pragma once

#include <QListWidget>
#include <QShortcut>

class ListWidgetInput : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(QStringList items READ items WRITE setItems NOTIFY itemsChanged)

public:
    ListWidgetInput(QWidget* parent = nullptr);

    bool editable() const { return editable_; }
    QStringList items() const;

public slots:
    void setEditable(bool editable);
    void setItems(const QStringList& items);

signals:
    void editableChanged();
    void itemsChanged();

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& unselected);
    void onCustomContextMenuRequested(const QPoint& point);
    void onActionAddItemTriggered();
    void onActionRemoveItemTriggered();

private:
    QListWidgetItem* makeItem(const QString& text);
    void fixItemEditability();

private:
    bool editable_;
    QAction* actionAddItem_;
    QShortcut* shortcutAddItem_;
    QAction* actionRemoveItem_;
    QShortcut* shortcutRemoveItem_;
};
