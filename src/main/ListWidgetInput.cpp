// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "ListWidgetInput.h"

#include <QAction>
#include <QMenu>

ListWidgetInput::ListWidgetInput(QWidget* parent) :
    QListWidget{parent},
    editable_{false},
    actionAddItem_{new QAction{tr("New Item"), this}},
    shortcutAddItem_{new QShortcut{this}},
    actionRemoveItem_{new QAction{tr("Remove Item"), this}},
    shortcutRemoveItem_{new QShortcut{this}}
{
    shortcutAddItem_->setKey(QKeySequence(Qt::Key_Insert));
    shortcutRemoveItem_->setKey(QKeySequence(Qt::Key_Delete));

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &ListWidgetInput::customContextMenuRequested, this, &ListWidgetInput::onCustomContextMenuRequested);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ListWidgetInput::onSelectionChanged);
    connect(actionAddItem_, &QAction::triggered, this, &ListWidgetInput::onActionAddItemTriggered);
    connect(shortcutAddItem_, &QShortcut::activated, actionAddItem_, &QAction::trigger);
    connect(actionRemoveItem_, &QAction::triggered, this, &ListWidgetInput::onActionRemoveItemTriggered);
    connect(shortcutRemoveItem_, &QShortcut::activated, actionRemoveItem_, &QAction::trigger);
}

QStringList ListWidgetInput::items() const
{
    QSet<QString> dupCheck;
    QStringList output;
    for (int i = 0; i < count(); ++i)
    {
        const auto text = item(i)->text();
        if (dupCheck.find(text) != dupCheck.end() ||
            text.compare(tr("<Field>"), Qt::CaseInsensitive) == 0)
            continue;
        output << text;
    }
    return output;
}

void ListWidgetInput::setEditable(bool editable)
{
    if (editable == editable_)
        return;

    editable_ = editable;
    fixItemEditability();

    emit editableChanged();
}

void ListWidgetInput::setItems(const QStringList& items)
{
    clear();
    for (int i = 0; i < items.count(); ++i)
    {
        addItem(makeItem(items[i]));
    }
    emit itemsChanged();
}

void ListWidgetInput::onSelectionChanged(const QItemSelection& selected, const QItemSelection& unselected)
{
    Q_UNUSED(unselected)

    actionRemoveItem_->setEnabled(!selected.isEmpty());
}

void ListWidgetInput::onCustomContextMenuRequested(const QPoint& point)
{
    if (editable_)
    {
        QMenu ctxMenu;
        ctxMenu.addAction(actionAddItem_);
        ctxMenu.addAction(actionRemoveItem_);
        ctxMenu.exec(mapToGlobal(point));
    }
}

void ListWidgetInput::onActionAddItemTriggered()
{
    if (editable_)
    {
        int insertIndex = count();
        addItem(makeItem(tr("<Field>")));
        editItem(item(insertIndex));
    }
}

void ListWidgetInput::onActionRemoveItemTriggered()
{
    if (editable_)
    {
        const auto indexes = selectionModel()->selectedIndexes();
        if (indexes.empty())
            return;
        delete takeItem(indexes[0].row());
    }
}

QListWidgetItem* ListWidgetInput::makeItem(const QString& text)
{
    auto* item = new QListWidgetItem{text};
    if (editable_)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    return item;
}

void ListWidgetInput::fixItemEditability()
{
    for (int i = 0; i < count(); ++i)
    {
        auto* itm = item(i);

        if (editable_)
        {
            itm->setFlags(itm->flags() | Qt::ItemIsEditable);
        }
        else
        {
            auto newFlags = static_cast<unsigned int>(itm->flags()) & ~static_cast<unsigned int>(Qt::ItemIsEditable);
            itm->setFlags(static_cast<Qt::ItemFlags>(newFlags));
        }
    }
}
