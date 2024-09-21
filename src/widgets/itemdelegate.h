/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_ITEMDELEGATE_H
#define WIDGETS_ITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Widgets {

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private:
    enum class EditingState { NotEditing, JustCreatedEditor, Editing };
    mutable EditingState m_editingState = EditingState::NotEditing;

    struct Layout;
    Layout doLayout(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const;
};

}

#endif // WIDGETS_ITEMDELEGATE_H
