/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include <memory>

#include <QMimeData>

#include "utils/mockobject.h"

#include "domain/noterepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"

#include "presentation/tagpagemodel.h"
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

class TagPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListTagNotesInCentralListModel()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle(QStringLiteral("note1"));
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle(QStringLiteral("note2"));
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findNotes).when(tag).thenReturn(noteResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex note1Index = model->index(0, 0);
        const QModelIndex note2Index = model->index(1, 0);

        QCOMPARE(page.tag(), tag);

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

        QCOMPARE(note1->title(), QStringLiteral("newNote1"));
        QCOMPARE(note2->title(), QStringLiteral("newNote2"));

        // WHEN
        auto data = std::unique_ptr<QMimeData>(model->mimeData(QModelIndexList() << note2Index));

        // THEN
        QVERIFY(data->hasFormat(QStringLiteral("application/x-zanshin-object")));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << note2);
    }

    void shouldAddNotes()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        // We'll gladly create a note though
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::createInTag).when(any<Domain::Note::Ptr>(),
                                                                      any<Domain::Tag::Ptr>())
                                                                .thenReturn(new FakeJob(this));

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        auto title = QStringLiteral("New note");
        auto note = page.addItem(title).objectCast<Domain::Note>();

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::createInTag).when(any<Domain::Note::Ptr>(),
                                                                              any<Domain::Tag::Ptr>())
                                                                        .exactly(1));
        QVERIFY(note);
        QCOMPARE(note->title(), title);
    }

    void shouldRemoveItem()
    {
        // GIVEN

        // One domain tag
        auto tag = Domain::Tag::Ptr::create();

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        auto note2 = Domain::Note::Ptr::create();

        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findNotes).when(tag).thenReturn(noteResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, note2).thenReturn(new FakeJob(this));

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex indexNote2 = page.centralListModel()->index(1, 0);
        page.removeItem(indexNote2);

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, note2).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveItemFailed()
    {
        // GIVEN

        // One domain tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName(QStringLiteral("Tag1"));

        // Two notes
        auto note1 = Domain::Note::Ptr::create();
        note1->setTitle(QStringLiteral("Note 1"));
        auto note2 = Domain::Note::Ptr::create();
        note2->setTitle(QStringLiteral("Note 2"));

        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(note1);
        noteProvider->append(note2);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findNotes).when(tag).thenReturn(noteResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, note2).thenReturn(job);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        const QModelIndex indexNote2 = page.centralListModel()->index(1, 0);
        page.removeItem(indexNote2);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot remove note Note 2 from tag Tag1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAddNoteFailed()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName(QStringLiteral("Tag1"));

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        // We'll gladly create a note though
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        noteRepositoryMock(&Domain::NoteRepository::createInTag).when(any<Domain::Note::Ptr>(),
                                                                      any<Domain::Tag::Ptr>())
                                                                .thenReturn(job);

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        page.addItem(QStringLiteral("New note"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add note New note in tag Tag1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateNoteFailed()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName(QStringLiteral("Tag1"));

        // One note and one task
        auto rootNote = Domain::Note::Ptr::create();
        rootNote->setTitle(QStringLiteral("rootNote"));
        auto noteProvider = Domain::QueryResultProvider<Domain::Note::Ptr>::Ptr::create();
        auto noteResult = Domain::QueryResult<Domain::Note::Ptr>::create(noteProvider);
        noteProvider->append(rootNote);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findNotes).when(tag).thenReturn(noteResult);

        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex rootNoteIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        noteRepositoryMock(&Domain::NoteRepository::update).when(rootNote).thenReturn(job);

        QVERIFY(model->setData(rootNoteIndex, "newRootNote"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify note rootNote in tag Tag1: Foo"));
    }
};

ZANSHIN_TEST_MAIN(TagPageModelTest)

#include "tagpagemodeltest.moc"
