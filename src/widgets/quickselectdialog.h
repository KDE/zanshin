/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2015 Franck Arrecot <franck.arrecot@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

private Q_SLOTS:
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
