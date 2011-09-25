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

#ifndef ZANSHIN_ACTIONLISTEDITOR_H
#define ZANSHIN_ACTIONLISTEDITOR_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>

#include "globaldefs.h"
#include <KXMLGUIClient>

class ItemViewer;
class ItemSelectorProxy;
class ActionListEditorPage;
class KAction;
class KActionCollection;
class KConfigGroup;
class KLineEdit;
class QAbstractItemModel;
class QComboBox;
class QItemSelectionModel;
class QStackedWidget;
class ModelStack;

class ActionListEditor : public QWidget
{
    Q_OBJECT

public:
    ActionListEditor(ModelStack *models,
                     QItemSelectionModel *projectSelection,
                     QItemSelectionModel *categoriesSelection,
                     KActionCollection *ac, QWidget *parent, KXMLGUIClient *client, ItemViewer *itemviewer);

    void setMode(Zanshin::ApplicationMode mode);

    void saveColumnsState(KConfigGroup &config) const;
    void restoreColumnsState(const KConfigGroup &config);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);
/*signals:
    void currentChanged(const Akonadi::Item &);*/

private slots:
    void updateActions();
    void onAddActionRequested();
    void onRemoveAction();
    void onMoveAction();
    void onPromoteAction();
    void onDissociateAction();
    void focusActionEdit();
    void onSideBarSelectionChanged(const QModelIndex &index);
    void onComboBoxChanged();
    void onRowInsertedInComboBox(const QModelIndex &index, int start, int end);

private:
    void createPage(QAbstractItemModel *model, ModelStack *models, Zanshin::ApplicationMode, KXMLGUIClient *guiClient);
    void setupActions(KActionCollection *ac);
    bool selectDefaultCollection(QAbstractItemModel *model, const QModelIndex &parent, int begin, int end);

    ActionListEditorPage *currentPage() const;
    ActionListEditorPage *page(int idx) const;

    QStackedWidget *m_stack;
    QItemSelectionModel *m_projectSelection;
    QItemSelectionModel *m_categoriesSelection;
    QItemSelectionModel *m_knowledgeSelection;

    KLineEdit *m_addActionEdit;
    QComboBox *m_comboBox;

    KAction *m_add;
    KAction *m_cancelAdd;
    KAction *m_remove;
    KAction *m_move;
    KAction *m_promote;
    KAction *m_dissociate;

    ModelStack *m_models;
    ItemSelectorProxy *m_selectorProxy;

    qint64 m_defaultCollectionId;
};

#endif

