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

class CollectionsFilterProxyModel;
class QComboBox;
class KLineEdit;
class KXMLGUIClient;
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
    friend class ActionListEditor;
public:
    ActionListEditorPage(QAbstractItemModel *model,
                         ModelStack *models,
                         Zanshin::ApplicationMode mode,
                         const QList<QAction*> &contextActions,
                         const QList<QAction*> &toolbarActions,
                         QWidget *parent, KXMLGUIClient *client);

    QItemSelectionModel *selectionModel() const;
    Akonadi::EntityTreeView *treeView() const;
    void saveColumnsState(KConfigGroup &config, const QString &key) const;
    void restoreColumnsState(const KConfigGroup &config, const QString &key);

    Zanshin::ApplicationMode mode();

    bool selectSiblingIndex(const QModelIndex &index);
    void selectFirstIndex();
    
    void setCollectionSelectorVisible(bool);
    
    void focusActionEdit();
    void clearActionEdit();
    void setActionEditEnabled(bool);

public slots:
    void addNewItem(const QString &summary);
    void dissociateTodo(const QModelIndex &current);
    void onAddActionRequested();

private slots:
    void onColumnsGeometryChanged();
    void onSelectFirstIndex();
    void onComboBoxChanged();
    void onRowInsertedInComboBox(const QModelIndex &index, int start, int end);
    void setDefaultCollection(const Akonadi::Collection &collection);
    void setDefaultNoteCollection(const Akonadi::Collection &collection);

private:
    bool selectDefaultCollection(QAbstractItemModel *model, const QModelIndex &parent, int begin, int end, Akonadi::Collection::Id defaultCol);
    void selectDefaultCollection(const Akonadi::Collection &collection);
    void setCurrentCollection(const Akonadi::Collection &collection);

    Akonadi::EntityTreeView *m_treeView;
    Zanshin::ApplicationMode m_mode;

    KLineEdit *m_addActionEdit;
    QComboBox *m_comboBox;
    
    QByteArray m_normalStateCache;
    
    qint64 m_defaultCollectionId;
    Akonadi::Collection m_currentCollection;
    
    CollectionsFilterProxyModel *m_todoColsModel;


};


#endif

