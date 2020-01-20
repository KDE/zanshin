/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2015 Franck Arrecot <franck.arrecot@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef WIDGETS_QUICKSELECTDIALOG_H
#define WIDGETS_QUICKSELECTDIALOG_H

#include <QDialog>
#include <QPersistentModelIndex>
#include <QSharedPointer>

#include "presentation/metatypes.h"

#include "widgets/quickselectdialoginterface.h"

class QAbstractItemModel;
class QLabel;
class QTreeView;

class QSortFilterProxyModel;

namespace Widgets {

class QuickSelectDialog : public QDialog, public QuickSelectDialogInterface
{
    Q_OBJECT
public:
    explicit QuickSelectDialog(QWidget *parent = nullptr);

    int exec() override;

    QPersistentModelIndex selectedIndex() const override;
    void setModel(QAbstractItemModel *model) override;

private slots:
    void applyFilterChanged(const QString &textFilter);
    bool eventFilter(QObject *object, QEvent *ev) override;

private:
    QString m_filter;
    QAbstractItemModel *m_model;
    QSortFilterProxyModel *m_filterProxyModel;

    QLabel *m_label;
    QTreeView *m_tree;
};

}

#endif // WIDGETS_QUICKSELECTDIALOG_H
