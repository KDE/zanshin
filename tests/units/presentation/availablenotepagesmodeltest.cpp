/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include <testlib/qtest_zanshin.h>

#include "utils/mockobject.h"

#include "presentation/availablenotepagesmodel.h"
#include "presentation/errorhandler.h"
#include "presentation/noteinboxpagemodel.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/tagpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
    {
        m_message = message;
    }

    QString m_message;
};

class AvailableNotePagesModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldDeclareOnlyProjectAndContextPages()
    {
        // GIVEN
        Presentation::AvailableNotePagesModel pages({}, {}, {}, {});

        // THEN
        QVERIFY(!pages.hasProjectPages());
        QVERIFY(!pages.hasContextPages());
        QVERIFY(pages.hasTagPages());
    }

    void shouldListAvailablePages()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("Tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("Tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // One note (used for dropping later on)
        auto noteToDrop = Domain::Note::Ptr::create();

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    tagRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);
        const QModelIndex tagsIndex = model->index(1, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);
        const QModelIndex tag2Index = model->index(1, 0, tagsIndex);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(inboxIndex), 0);
        QCOMPARE(model->rowCount(tagsIndex), 2);
        QCOMPARE(model->rowCount(tag1Index), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable;
        QCOMPARE(model->flags(inboxIndex), (defaultFlags & ~(Qt::ItemIsEditable)) | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(tagsIndex), Qt::NoItemFlags);
        QCOMPARE(model->flags(tag1Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(tag2Index), defaultFlags | Qt::ItemIsDropEnabled);

        QCOMPARE(model->data(inboxIndex).toString(), tr("Inbox"));
        QCOMPARE(model->data(tagsIndex).toString(), tr("Tags"));
        QCOMPARE(model->data(tag1Index).toString(), tag1->name());
        QCOMPARE(model->data(tag2Index).toString(), tag2->name());

        QVERIFY(!model->data(inboxIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(tagsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(tag1Index, Qt::EditRole).toString(), tag1->name());
        QCOMPARE(model->data(tag2Index, Qt::EditRole).toString(), tag2->name());

        QCOMPARE(model->data(inboxIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("mail-folder-inbox"));
        QCOMPARE(model->data(tagsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(tag1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));
        QCOMPARE(model->data(tag2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("view-pim-tasks"));

        QVERIFY(!model->data(inboxIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(tagsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(tag1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(tag2Index, Qt::CheckStateRole).isValid());

        QVERIFY(!model->setData(inboxIndex, "foo", Qt::EditRole));
        QVERIFY(!model->setData(tagsIndex, "foo", Qt::EditRole));
        QVERIFY(!model->setData(tag1Index, "foo", Qt::EditRole));
        QVERIFY(!model->setData(tag2Index, "foo", Qt::EditRole));

        // WHEN
        tagRepositoryMock(&Domain::TagRepository::associate).when(tag1, noteToDrop).thenReturn(new FakeJob(this));
        QMimeData *data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << noteToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, tag1Index);

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::associate).when(tag1, noteToDrop).exactly(1));

        // WHEN
        tagRepositoryMock(&Domain::TagRepository::dissociateAll).when(noteToDrop).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << noteToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, inboxIndex);
        QTest::qWait(150);

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::dissociateAll).when(noteToDrop).exactly(1));
    }

    void shouldCreateInboxPage()
    {
        // GIVEN

        // Empty tag provider
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    Domain::TagRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);

        QObject *inboxPage = pages.createPageForIndex(inboxIndex);
        QVERIFY(qobject_cast<Presentation::NoteInboxPageModel*>(inboxPage));
    }

    void shouldCreateTagsPage()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);


        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    Domain::TagRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex tagsIndex = model->index(1, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);
        const QModelIndex tag2Index = model->index(1, 0, tagsIndex);

        QObject *tagsPage = pages.createPageForIndex(tagsIndex);
        QObject *tag1Page = pages.createPageForIndex(tag1Index);
        QObject *tag2Page = pages.createPageForIndex(tag2Index);

        QVERIFY(!tagsPage);
        QVERIFY(qobject_cast<Presentation::TagPageModel*>(tag1Page));
        QCOMPARE(qobject_cast<Presentation::TagPageModel*>(tag1Page)->tag(), tag1);
        QVERIFY(qobject_cast<Presentation::TagPageModel*>(tag2Page));
        QCOMPARE(qobject_cast<Presentation::TagPageModel*>(tag2Page)->tag(), tag2);
    }

    void shouldAddTags()
    {
        // GIVEN

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                         .thenReturn(new FakeJob(this));

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    Domain::TagQueries::Ptr(),
                                                    tagRepositoryMock.getInstance());

        // WHEN
        pages.addTag("Foo");

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                                 .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddTagFailed()
    {
        // GIVEN

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::create).when(any<Domain::Tag::Ptr>())
                                                         .thenReturn(job);

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    Domain::TagQueries::Ptr(),
                                                    tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addTag("Foo");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add tag Foo: Foo"));
    }

    void shouldRemoveTag()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex tagsIndex = model->index(1, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);

        auto job = new FakeJob(this);
        tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).thenReturn(job);

        // WHEN
        pages.removeItem(tag1Index);

        // THEN
        QTest::qWait(150);
        QVERIFY(errorHandler.m_message.isEmpty());
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveTagFailed()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex tagsIndex = model->index(1, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::remove).when(tag1).thenReturn(job);

        // WHEN
        pages.removeItem(tag1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove tag tag 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateTagFailed()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("Tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("Tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // One note (used for dropping later on)
        auto noteToDrop = Domain::Note::Ptr::create();
        noteToDrop->setTitle("noteDropped");

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex tagsIndex = model->index(1, 0);
        const QModelIndex tag1Index = model->index(0, 0, tagsIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::associate).when(tag1, noteToDrop).thenReturn(job);
        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << noteToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, tag1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot tag noteDropped with Tag 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenDissociateTaskFailed()
    {
        // GIVEN

        // Two tags
        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("tag 1");
        auto tag2 = Domain::Tag::Ptr::create();
        tag2->setName("tag 2");
        auto tagProvider = Domain::QueryResultProvider<Domain::Tag::Ptr>::Ptr::create();
        auto tagResult = Domain::QueryResult<Domain::Tag::Ptr>::create(tagProvider);
        tagProvider->append(tag1);
        tagProvider->append(tag2);

        // One note (used for dropping later on)
        auto noteToDrop = Domain::Note::Ptr::create();
        noteToDrop->setTitle("noteDropped");

        // tags mocking
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findAll).when().thenReturn(tagResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::AvailableNotePagesModel pages(Domain::NoteQueries::Ptr(),
                                                    Domain::NoteRepository::Ptr(),
                                                    tagQueriesMock.getInstance(),
                                                    tagRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex inboxIndex = model->index(0, 0);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        tagRepositoryMock(&Domain::TagRepository::dissociateAll).when(noteToDrop).thenReturn(job);
        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << noteToDrop));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, inboxIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot move noteDropped to Inbox: Foo"));
    }
};

ZANSHIN_TEST_MAIN(AvailableNotePagesModelTest)

#include "availablenotepagesmodeltest.moc"
