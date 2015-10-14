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

#include <QtTest>

#include "utils/mockobject.h"

#include "presentation/noteinboxpagemodel.h"
#include "presentation/errorhandler.h"

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

class NoteInboxPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListInboxInCentralListModel()
    {
        // GIVEN

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle("note1");
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle("note2");
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;
        noteQueriesMock(&Domain::NoteQueries::findInbox).when().thenReturn(noteResult);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = inbox.centralListModel();

        // THEN
        const QModelIndex note1Index = model->index(0, 0);
        const QModelIndex note2Index = model->index(1, 0);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(note1Index), 0);
        QCOMPARE(model->rowCount(note2Index), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;
        QCOMPARE(model->flags(note1Index), defaultFlags);
        QCOMPARE(model->flags(note2Index), defaultFlags);

        QCOMPARE(model->data(note1Index).toString(), note1->title());
        QCOMPARE(model->data(note2Index).toString(), note2->title());

        QCOMPARE(model->data(note1Index, Qt::EditRole).toString(), note1->title());
        QCOMPARE(model->data(note2Index, Qt::EditRole).toString(), note2->title());

        QVERIFY(!model->data(note1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(note2Index, Qt::CheckStateRole).isValid());

        // WHEN
        noteRepositoryMock(&Domain::NoteRepository::update).when(note1).thenReturn(new FakeJob(this));
        noteRepositoryMock(&Domain::NoteRepository::update).when(note2).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(note1Index, "newNote1"));
        QVERIFY(model->setData(note2Index, "newNote2"));

        QVERIFY(!model->setData(note1Index, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(!model->setData(note2Index, Qt::Checked, Qt::CheckStateRole));

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::update).when(note1).exactly(1));
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::update).when(note2).exactly(1));

        QCOMPARE(note1->title(), QString("newNote1"));
        QCOMPARE(note2->title(), QString("newNote2"));

        // WHEN
        QMimeData *data = model->mimeData(QModelIndexList() << note2Index);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << note2);
    }

    void shouldAddNotes()
    {
        // GIVEN

        // ... in fact we won't list any model
        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;

        // We'll gladly create a note though
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::create).when(any<Domain::Note::Ptr>()).thenReturn(new FakeJob(this));


        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());

        // WHEN
        auto title = QString("New note");
        auto note = inbox.addItem(title).objectCast<Domain::Note>();

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::create).when(any<Domain::Note::Ptr>()).exactly(1));
        QVERIFY(note);
        QCOMPARE(note->title(), title);
    }

    void shouldGetAnErrorMessageWhenAddNoteFailed()
    {
        // GIVEN

        // ... in fact we won't list any model
        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;

        // We'll gladly create a note though
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        noteRepositoryMock(&Domain::NoteRepository::create).when(any<Domain::Note::Ptr>()).thenReturn(job);

        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());

        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        inbox.addItem("New note");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add note New note in Inbox: Foo"));
    }

    void shouldDeleteItems()
    {
        // GIVEN

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle("note1");
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle("note2");
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;
        noteQueriesMock(&Domain::NoteQueries::findInbox).when().thenReturn(noteResult);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::remove).when(note2).thenReturn(new FakeJob(this));

        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.removeItem(index);

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::remove).when(note2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenDeleteItemsFailed()
    {
        // GIVEN

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle("note1");
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle("note2");
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;
        noteQueriesMock(&Domain::NoteQueries::findInbox).when().thenReturn(noteResult);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        noteRepositoryMock(&Domain::NoteRepository::remove).when(note2).thenReturn(job);

        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.removeItem(index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot remove note note2 from Inbox: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateNoteFailed()
    {
        // GIVEN

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle("note1");
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle("note2");
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::NoteQueries> noteQueriesMock;
        noteQueriesMock(&Domain::NoteQueries::findInbox).when().thenReturn(noteResult);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::NoteInboxPageModel inbox(noteQueriesMock.getInstance(),
                                               noteRepositoryMock.getInstance());

        QAbstractItemModel *model = inbox.centralListModel();
        const QModelIndex note1Index = model->index(0, 0);
        FakeErrorHandler errorHandler;
        inbox.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        noteRepositoryMock(&Domain::NoteRepository::update).when(note1).thenReturn(job);

        QVERIFY(model->setData(note1Index, "newNote1"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify note note1 in Inbox: Foo"));
    }
};

QTEST_MAIN(NoteInboxPageModelTest)

#include "noteinboxpagemodeltest.moc"