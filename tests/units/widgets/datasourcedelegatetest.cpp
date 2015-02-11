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

#include <QtTestGui>

#include <QHeaderView>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTreeView>

#include "domain/datasource.h"

#include "presentation/querytreemodelbase.h"

#include "widgets/datasourcedelegate.h"

Q_DECLARE_METATYPE(QList<int>)

class DataSourceDelegateTest : public QObject
{
    Q_OBJECT
public:
    explicit DataSourceDelegateTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<Domain::DataSource::Ptr>();
    }

    QStandardItem *itemFromSource(const Domain::DataSource::Ptr &source)
    {
        auto item = new QStandardItem;
        item->setText(source->name());
        item->setIcon(QIcon::fromTheme(source->iconName()));
        item->setData(QVariant::fromValue(source),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

private slots:
    void shouldHandleClickOnButtons_data()
    {
        QTest::addColumn<Domain::DataSource::Ptr>("source");
        QTest::addColumn<QList<int>>("expectedActions");

        QList<int> actions;
        auto source = Domain::DataSource::Ptr::create();
        source->setName("No Content");
        source->setIconName("folder");
        source->setListStatus(Domain::DataSource::Bookmarked);
        QTest::newRow("no content") << source << actions;

        actions.clear();
        actions << Widgets::DataSourceDelegate::AddToList;
        source = Domain::DataSource::Ptr::create();
        source->setName("Not listed");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setListStatus(Domain::DataSource::Unlisted);
        QTest::newRow("not listed") << source << actions;

        actions.clear();
        actions << Widgets::DataSourceDelegate::Bookmark << Widgets::DataSourceDelegate::RemoveFromList;
        source = Domain::DataSource::Ptr::create();
        source->setName("Listed");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setListStatus(Domain::DataSource::Listed);
        QTest::newRow("listed") << source << actions;

        actions.clear();
        actions << Widgets::DataSourceDelegate::Bookmark << Widgets::DataSourceDelegate::RemoveFromList;
        source = Domain::DataSource::Ptr::create();
        source->setName("Bookmarked");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setListStatus(Domain::DataSource::Bookmarked);
        QTest::newRow("bookmarked") << source << actions;
    }

    void shouldHandleClickOnButtons()
    {
        // GIVEN
        QFETCH(Domain::DataSource::Ptr, source);
        QFETCH(QList<int>, expectedActions);

        Widgets::DataSourceDelegate delegate;
        QSignalSpy spy(&delegate, SIGNAL(actionTriggered(Domain::DataSource::Ptr,int)));

        QStandardItemModel model;
        model.appendRow(itemFromSource(source));

        QTreeView view;
        view.header()->hide();
        view.setItemDelegate(&delegate);
        view.setModel(&model);
        view.show();
        QTest::qWaitForWindowShown(&view);

        // WHEN
        QPoint clickPoint = view.rect().topRight() + QPoint(-30, 10);
        for (int i = 0; i < 5; i++) {
            QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, clickPoint);
            clickPoint += QPoint(-20, 0);
        }

        // THEN
        QList<int> actions;
        while (!spy.isEmpty()) {
            auto parameters = spy.takeFirst();
            QCOMPARE(parameters.first().value<Domain::DataSource::Ptr>(), source);
            actions << parameters.last().toInt();
        }
        QCOMPARE(actions, expectedActions);
    }
};

QTEST_MAIN(DataSourceDelegateTest)

#include "datasourcedelegatetest.moc"
