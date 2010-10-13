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

class ActionListEditorPage;
class ActionListModel;
class KAction;
class KActionCollection;
class KConfigGroup;
class KLineEdit;
class QAbstractItemModel;
class QComboBox;
class QItemSelectionModel;
class QStackedWidget;
class ModelStack;

namespace Akonadi
{
    class EntityTreeView;
}

class ActionListEditor : public QWidget
{
    Q_OBJECT

public:
    ActionListEditor(ModelStack *models,
                     QItemSelectionModel *projectSelection,
                     QItemSelectionModel *categoriesSelection,
                     KActionCollection *ac, QWidget *parent=0);

    void setMode(Zanshin::ApplicationMode mode);

    void saveColumnsState(KConfigGroup &config) const;
    void restoreColumnsState(const KConfigGroup &config);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void updateActions(const QModelIndex &index);
    void onAddActionRequested();
    void onRemoveAction();
    void onMoveAction();
    void focusActionEdit();
    void onSideBarSelectionChanged(const QModelIndex &index);
    void onComboBoxChanged();

private:
    void createPage(QAbstractItemModel *model, ModelStack *models, Zanshin::ApplicationMode);
    void setupActions(KActionCollection *ac);

    ActionListEditorPage *currentPage() const;
    ActionListEditorPage *page(int idx) const;

    QStackedWidget *m_stack;
    QItemSelectionModel *m_projectSelection;
    QItemSelectionModel *m_categoriesSelection;

    KLineEdit *m_addActionEdit;
    QComboBox *m_comboBox;

    KAction *m_add;
    KAction *m_cancelAdd;
    KAction *m_remove;
    KAction *m_move;
};

#endif

