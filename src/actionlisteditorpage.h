/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_ACTIONLISTEDITORPAGE_H
#define ZANSHIN_ACTIONLISTEDITORPAGE_H

#include <KDE/Akonadi/Collection>

#include <QtGui/QWidget>

#include "globaldefs.h"

class KConfigGroup;
class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;
class ModelStack;

namespace Akonadi
{
    class EntityTreeView;
}

class ActionListEditorPage : public QWidget
{
    Q_OBJECT

public:
    ActionListEditorPage(QAbstractItemModel *model,
                         ModelStack *models,
                         Zanshin::ApplicationMode mode,
                         QWidget *parent=0);

    QItemSelectionModel *selectionModel() const;

    void saveColumnsState(KConfigGroup &config, const QString &key) const;
    void restoreColumnsState(const KConfigGroup &config, const QString &key);

    Zanshin::ApplicationMode mode();

    void setCollectionColumnHidden(bool hidden);

    void setDefaultCollection(const Akonadi::Collection &collection);

    bool selectSiblingIndex(const QModelIndex &index);
    void selectFirstIndex();

public slots:
    void addNewTodo(const QString &summary);
    void removeCurrentTodo();

private slots:
    void onAutoHideColumns();
    void onColumnsGeometryChanged();
    void onSelectFirstIndex();

private:
    Akonadi::EntityTreeView *m_treeView;
    Zanshin::ApplicationMode m_mode;

    QByteArray m_normalStateCache;
    QByteArray m_noCollectionStateCache;

    Akonadi::Collection m_defaultCollection;
};

#endif

