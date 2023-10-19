/*
 * SPDX-FileCopyrightText: 2014-2019 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef ZANSHINCONTEXT_H
#define ZANSHINCONTEXT_H

#include <QObject>
#include <QPersistentModelIndex>
#include <QSharedPointer>
#include <QPointer>
#include <QList>

#include "domain/task.h"
#include "domain/datasource.h"
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
        QList<QByteArray> roles;
        QList<QList<QVariant>> rows;
    };

    explicit ZanshinContext(QObject *parent = nullptr);

    // GIVEN
    [[nodiscard]] bool I_display_the_available_data_sources();
    [[nodiscard]] bool I_display_the_available_pages();
    [[nodiscard]] bool I_display_the_page(const QString &pageName);
    [[nodiscard]] bool
    there_is_an_item_in_the_central_list(const QString &taskName);
    [[nodiscard]] bool
    there_is_an_item_in_the_available_data_sources(const QString &sourceName);
    [[nodiscard]] bool
    the_central_list_contains_items_named(const QStringList &taskNames);

    // WHEN
    [[nodiscard]] bool I_look_at_the_central_list();
    [[nodiscard]] bool I_check_the_item();
    [[nodiscard]] bool I_uncheck_the_item();
    [[nodiscard]] bool I_remove_the_item();
    [[nodiscard]] bool I_promote_the_item();
    [[nodiscard]] bool I_add_a_project(const QString &projectName,
                                       const QString &parentSourceName);
    [[nodiscard]] bool I_add_a_context(const QString &contextName,
                                       const QString &parentSourceName);
    [[nodiscard]] bool I_add_a_task(const QString &taskName);
    [[nodiscard]] bool I_rename_a_page(const QString &path,
                                       const QString &oldName,
                                       const QString &newName);
    [[nodiscard]] bool I_remove_a_page(const QString &path,
                                       const QString &pageName);
    [[nodiscard]] bool I_add_a_task_child(const QString &childName,
                                          const QString &parentName);
    [[nodiscard]] bool I_list_the_items();
    [[nodiscard]] bool I_open_the_item_in_the_editor();
    [[nodiscard]] bool I_mark_the_item_done_in_the_editor();
    [[nodiscard]] bool I_change_the_editor_field(const QString &field,
                                                 const QVariant &value);
    [[nodiscard]] bool I_rename_the_item(const QString &taskName);
    [[nodiscard]] bool I_open_the_item_in_the_editor_again();
    [[nodiscard]] bool
    I_drop_the_item_on_the_central_list(const QString &dropSiteName);
    [[nodiscard]] bool I_drop_the_item_on_the_blank_area_of_the_central_list();
    [[nodiscard]] bool
    I_drop_items_on_the_central_list(const QString &dropSiteName);
    [[nodiscard]] bool
    I_drop_the_item_on_the_page_list(const QString &pageName);
    [[nodiscard]] bool I_drop_items_on_the_page_list(const QString &pageName);
    [[nodiscard]] bool I_change_the_setting(const QString &key, qint64 id);
    [[nodiscard]] bool
    I_change_the_default_data_source(const QString &sourceName);

    // THEN
    [[nodiscard]] bool the_list_is(const TableData &data);
    [[nodiscard]] bool the_list_contains(const QString &itemName);
    [[nodiscard]] bool the_list_does_not_contain(const QString &itemName);
    [[nodiscard]] bool the_task_corresponding_to_the_item_is_done();
    [[nodiscard]] bool the_editor_shows_the_task_as_done();
    [[nodiscard]] bool
    the_editor_shows_the_field(const QString &field,
                               const QVariant &expectedValue);
    [[nodiscard]] bool the_default_data_source_is(const QString &sourceName);
    [[nodiscard]] bool the_setting_is(const QString &key, qint64 expectedId);

  private:
    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *sourceModel() const;
    QAbstractItemModel *model() const;

    Domain::Task::Ptr currentTask() const;
    Domain::DataSource::Ptr dataSourceFromName(const QString &sourceName);

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
#endif
