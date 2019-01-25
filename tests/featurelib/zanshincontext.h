/* This file is part of Zanshin

   Copyright 2014-2019 Kevin Ottens <ervin@kde.org>

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

#include <QObject>
#include <QPersistentModelIndex>
#include <QSharedPointer>
#include <QPointer>
#include <QVector>

#include "domain/task.h"
#include "presentation/errorhandler.h"

#include "testlib/akonadifakedata.h"
#include "testlib/fakejob.h"

class QAbstractItemModel;
class QSortFilterProxyModel;
class MonitorSpy;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &) override;
};

#define Given(a) QVERIFY(a)
#define When(a) QVERIFY(a)
#define Then(a) QVERIFY(a)
#define And(a) QVERIFY(a)

class ZanshinContext : public QObject
{
    Q_OBJECT
public:
    struct TableData
    {
        QVector<QByteArray> roles;
        QVector<QVector<QVariant>> rows;
    };

    explicit ZanshinContext(QObject *parent = nullptr);

    // GIVEN
    Q_REQUIRED_RESULT bool I_display_the_available_data_sources();
    Q_REQUIRED_RESULT bool I_display_the_available_pages();
    Q_REQUIRED_RESULT bool I_display_the_page(const QString &pageName);
    Q_REQUIRED_RESULT bool there_is_an_item_in_the_central_list(const QString &taskName);
    Q_REQUIRED_RESULT bool there_is_an_item_in_the_available_data_sources(const QString &sourceName);
    Q_REQUIRED_RESULT bool the_central_list_contains_items_named(const QStringList &taskNames);

    // WHEN
    Q_REQUIRED_RESULT bool I_look_at_the_central_list();
    Q_REQUIRED_RESULT bool I_check_the_item();
    Q_REQUIRED_RESULT bool I_uncheck_the_item();
    Q_REQUIRED_RESULT bool I_remove_the_item();
    Q_REQUIRED_RESULT bool I_promote_the_item();
    Q_REQUIRED_RESULT bool I_add_a_project(const QString &projectName, const QString &parentSourceName);
    Q_REQUIRED_RESULT bool I_add_a_context(const QString &contextName);
    Q_REQUIRED_RESULT bool I_add_a_task(const QString &taskName);
    Q_REQUIRED_RESULT bool I_rename_a_page(const QString &path, const QString &oldName, const QString &newName);
    Q_REQUIRED_RESULT bool I_remove_a_page(const QString &path, const QString &pageName);
    Q_REQUIRED_RESULT bool I_add_a_task_child(const QString &childName, const QString &parentName);
    Q_REQUIRED_RESULT bool I_list_the_items();
    Q_REQUIRED_RESULT bool I_open_the_item_in_the_editor();
    Q_REQUIRED_RESULT bool I_mark_the_item_done_in_the_editor();
    Q_REQUIRED_RESULT bool I_change_the_editor_field(const QString &field, const QVariant &value);
    Q_REQUIRED_RESULT bool I_rename_the_item(const QString &taskName);
    Q_REQUIRED_RESULT bool I_open_the_item_in_the_editor_again();
    Q_REQUIRED_RESULT bool I_drop_the_item_on_the_central_list(const QString &dropSiteName);
    Q_REQUIRED_RESULT bool I_drop_the_item_on_the_blank_area_of_the_central_list();
    Q_REQUIRED_RESULT bool I_drop_items_on_the_central_list(const QString &dropSiteName);
    Q_REQUIRED_RESULT bool I_drop_the_item_on_the_page_list(const QString &pageName);
    Q_REQUIRED_RESULT bool I_drop_items_on_the_page_list(const QString &pageName);
    Q_REQUIRED_RESULT bool I_change_the_setting(const QString &key, qint64 id);
    Q_REQUIRED_RESULT bool I_change_the_default_data_source(const QString &sourceName);

    // THEN
    Q_REQUIRED_RESULT bool the_list_is(const TableData &data);
    Q_REQUIRED_RESULT bool the_list_contains(const QString &itemName);
    Q_REQUIRED_RESULT bool the_list_does_not_contain(const QString &itemName);
    Q_REQUIRED_RESULT bool the_task_corresponding_to_the_item_is_done();
    Q_REQUIRED_RESULT bool the_editor_shows_the_task_as_done();
    Q_REQUIRED_RESULT bool the_editor_shows_the_field(const QString &field, const QVariant &expectedValue);
    Q_REQUIRED_RESULT bool the_default_data_source_is(const QString &sourceName);
    Q_REQUIRED_RESULT bool the_setting_is(const QString &key, qint64 expectedId);

private:
    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *sourceModel() const;
    QAbstractItemModel *model() const;

    Domain::Task::Ptr currentTask() const;

    void waitForEmptyJobQueue();
    void waitForStableState();

    void collectIndicesImpl(const QModelIndex &root = QModelIndex());
    void collectIndices();

    QSharedPointer<QObject> m_appModel;

    QList<QPersistentModelIndex> m_indices;
    QPersistentModelIndex m_index;
    QObject *m_presentation;
    QObject *m_editor;
    QList<QPersistentModelIndex> m_dragIndices;

    Testlib::AkonadiFakeData m_data;

    QSortFilterProxyModel *m_proxyModel;
    QAbstractItemModel *m_model;
    QPointer<QAbstractItemModel> m_sourceModel;
    MonitorSpy *m_monitorSpy;
    FakeErrorHandler m_errorHandler;
};
