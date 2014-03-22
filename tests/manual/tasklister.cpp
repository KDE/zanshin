/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include <QListView>
#include <QAbstractListModel>

#include "akonadi/akonaditaskqueries.h"

class SimpleTaskListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SimpleTaskListModel(QObject *parent = 0)
        : QAbstractListModel(parent),
          m_queries(new Akonadi::TaskQueries)
    {
        m_taskList = m_queries->findAll();
        m_taskList->addPreInsertHandler([this](const Domain::Task::Ptr &, int index) {
                                            beginInsertRows(QModelIndex(), index, index);
                                        });
        m_taskList->addPostInsertHandler([this](const Domain::Task::Ptr &, int) {
                                             endInsertRows();
                                         });
        m_taskList->addPreRemoveHandler([this](const Domain::Task::Ptr &, int index) {
                                            beginRemoveRows(QModelIndex(), index, index);
                                        });
        m_taskList->addPostRemoveHandler([this](const Domain::Task::Ptr &, int) {
                                             endRemoveRows();
                                         });
        m_taskList->addPostReplaceHandler([this](const Domain::Task::Ptr &, int idx) {
                                             emit dataChanged(index(idx), index(idx));
                                         });
    }

    ~SimpleTaskListModel()
    {
        delete m_queries;
    }

    int rowCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return m_taskList->data().size();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.column() != 0
         || index.row() < 0 || index.row() >= m_taskList->data().size()) {
            return QVariant();
        }

        if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
            return QVariant();
        }

        Domain::Task::Ptr task = m_taskList->data().at(index.row());
        if (role == Qt::DisplayRole)
            return task->title();
        else
            return task->isDone() ? Qt::Checked : Qt::Unchecked;
    }

private:
    Akonadi::TaskQueries *m_queries;
    Domain::QueryResult<Domain::Task::Ptr>::Ptr m_taskList;
};

int main(int argc, char **argv)
{
    KAboutData about("tasklister", "tasklister",
                     ki18n("Lists all the tasks"), "1.0");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    QListView view;
    view.setModel(new SimpleTaskListModel);
    view.resize(640, 480);
    view.show();

    return app.exec();
}

#include "tasklister.moc"
