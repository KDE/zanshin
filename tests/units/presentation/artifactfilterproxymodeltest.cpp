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

#include <QStandardItemModel>

#include "domain/note.h"
#include "domain/task.h"

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/querytreemodelbase.h"

class ArtifactFilterProxyModelTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createTaskItem(const QString &title, const QString &text) const
    {
        auto task = Domain::Task::Ptr::create();
        task->setTitle(title);
        task->setText(text);

        auto item = new QStandardItem;
        item->setData(task->title(), Qt::DisplayRole);
        item->setData(QVariant::fromValue(Domain::Artifact::Ptr(task)),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

    QStandardItem *createNoteItem(const QString &title, const QString &text) const
    {
        auto note = Domain::Note::Ptr::create();
        note->setTitle(title);
        note->setText(text);

        auto item = new QStandardItem;
        item->setData(note->title(), Qt::DisplayRole);
        item->setData(QVariant::fromValue(Domain::Artifact::Ptr(note)),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

private slots:
    void shouldFilterByTextAndTitle()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem("1. foo", "find me"));
        input.appendRow(createTaskItem("2. Find Me", "bar"));
        input.appendRow(createTaskItem("3. baz", "baz"));
        input.appendRow(createNoteItem("4. foo", "find me"));
        input.appendRow(createNoteItem("5. find me", "bar"));
        input.appendRow(createNoteItem("6. baz", "baz"));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString("find me");

        // THEN
        QCOMPARE(output.rowCount(), 4);
        QCOMPARE(output.index(0, 0).data().toString(), QString("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QString("2. Find Me"));
        QCOMPARE(output.index(2, 0).data().toString(), QString("4. foo"));
        QCOMPARE(output.index(3, 0).data().toString(), QString("5. find me"));
    }

    void shouldKeepRowIfItHasAcceptableChildren()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem("1. foo", "find me"));
        QStandardItem *item = createTaskItem("2. baz", "baz");
        item->appendRow(createTaskItem("21. bar", "bar"));
        item->appendRow(createNoteItem("22. foo", "Find Me"));
        item->appendRow(createTaskItem("23. find me", "foo"));
        input.appendRow(item);
        input.appendRow(createTaskItem("3. baz", "baz"));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString("find me");

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QString("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QString("2. baz"));

        const QModelIndex parent = output.index(1, 0);
        QCOMPARE(output.rowCount(parent), 2);
        QCOMPARE(output.index(0, 0, parent).data().toString(), QString("22. foo"));
        QCOMPARE(output.index(1, 0, parent).data().toString(), QString("23. find me"));
    }
};

QTEST_MAIN(ArtifactFilterProxyModelTest)

#include "artifactfilterproxymodeltest.moc"
