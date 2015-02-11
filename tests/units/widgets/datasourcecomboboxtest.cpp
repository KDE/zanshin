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

#include <QAbstractItemView>
#include <QComboBox>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTestEventList>

#include "presentation/querytreemodelbase.h"
#include "widgets/datasourcecombobox.h"

class DataSourceComboBoxTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createStandardItem(const QString &name)
    {
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);
        return createStandardItem(source);
    }

    QStandardItem *createStandardItem(const Domain::DataSource::Ptr &source)
    {
        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(source), Presentation::QueryTreeModelBase::ObjectRole);
        item->setData(source->name(), Qt::DisplayRole);
        item->setData(source->iconName(), Presentation::QueryTreeModelBase::IconNameRole);
        return item;
    }

private slots:
    void shouldSelectDefaultSourceSilently()
    {
        // GIVEN
        auto defaultSource = Domain::DataSource::Ptr::create();
        defaultSource->setName("Default");

        QObject stub;
        stub.setProperty("defaultSource", QVariant::fromValue(defaultSource));
        QStandardItemModel model;
        model.appendRow(createStandardItem("Foo"));
        model.appendRow(createStandardItem("Bar"));
        model.appendRow(createStandardItem(defaultSource));
        model.appendRow(createStandardItem("Baz"));

        Widgets::DataSourceComboBox combo;
        QSignalSpy spy(&combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)));

        // WHEN
        combo.setModel(&model);
        combo.setDefaultSourceProperty(&stub, "defaultSource");

        // THEN
        QVERIFY(spy.isValid());
        QVERIFY(spy.isEmpty());
        QCOMPARE(combo.count(), 4);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.itemSource(2), defaultSource);
    }

    void shouldSelectDefaultSourceSilentlyIfItAppearsLater()
    {
        // GIVEN
        auto defaultSource = Domain::DataSource::Ptr::create();
        defaultSource->setName("Default");

        QObject stub;
        QStandardItemModel model;
        model.appendRow(createStandardItem("Foo"));
        model.appendRow(createStandardItem("Bar"));
        model.appendRow(createStandardItem("Baz"));

        Widgets::DataSourceComboBox combo;
        QSignalSpy spy(&combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)));
        combo.setDefaultSourceProperty(&stub, "defaultSource");
        combo.setModel(&model);

        QVERIFY(spy.isValid());
        QVERIFY(spy.isEmpty());
        QCOMPARE(combo.count(), 3);
        QCOMPARE(combo.currentIndex(), 0);

        // WHEN
        stub.setProperty("defaultSource", QVariant::fromValue(defaultSource));
        model.insertRow(2, createStandardItem(defaultSource));

        // THEN
        QVERIFY(spy.isEmpty());
        QCOMPARE(combo.count(), 4);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.itemSource(2), defaultSource);
    }

    void shouldNotifyKeyboardUserChange()
    {
        // GIVEN
        auto defaultSource = Domain::DataSource::Ptr::create();
        defaultSource->setName("Default");

        auto userSelectedSource = Domain::DataSource::Ptr::create();
        userSelectedSource->setName("To Be Selected");

        QObject stub;
        stub.setProperty("defaultSource", QVariant::fromValue(defaultSource));
        QStandardItemModel model;
        model.appendRow(createStandardItem("Foo"));
        model.appendRow(createStandardItem("Bar"));
        model.appendRow(createStandardItem(defaultSource));
        model.appendRow(createStandardItem(userSelectedSource));
        model.appendRow(createStandardItem("Baz"));

        Widgets::DataSourceComboBox combo;
        QSignalSpy spy(&combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)));
        combo.setModel(&model);
        combo.setDefaultSourceProperty(&stub, "defaultSource");
        QVERIFY(spy.isValid());

        // WHEN
        QTest::keyClick(combo.focusProxy(), Qt::Key_Down);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst().value<Domain::DataSource::Ptr>(), userSelectedSource);
        QCOMPARE(combo.currentIndex(), 3);
    }

    void shouldNotifyMouseUserChange()
    {
        // GIVEN
        auto defaultSource = Domain::DataSource::Ptr::create();
        defaultSource->setName("Default");

        auto userSelectedSource = Domain::DataSource::Ptr::create();
        userSelectedSource->setName("To Be Selected");

        QObject stub;
        stub.setProperty("defaultSource", QVariant::fromValue(defaultSource));
        QStandardItemModel model;
        model.appendRow(createStandardItem("Foo"));
        model.appendRow(createStandardItem(userSelectedSource));
        model.appendRow(createStandardItem("Bar"));
        model.appendRow(createStandardItem(defaultSource));
        model.appendRow(createStandardItem("Baz"));

        Widgets::DataSourceComboBox combo;
        QSignalSpy spy(&combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)));
        combo.setModel(&model);
        combo.setDefaultSourceProperty(&stub, "defaultSource");
        QVERIFY(spy.isValid());

        combo.show();
        QTest::qWaitForWindowShown(&combo);

        // WHEN
        QComboBox *proxy = qobject_cast<QComboBox*>(combo.focusProxy());
        QVERIFY(proxy);
        QTest::mouseClick(proxy, Qt::LeftButton, Q_NULLPTR, QPoint());

        // Make sure the popup is on screen
        QTest::qWaitForWindowShown(proxy->view());
        QTest::qWait(500);

        // Scan to find coordinates of the second item
        const int x = proxy->view()->rect().center().x();
        for (int y = 0; y < proxy->view()->rect().height(); y += 4) {
            const QPoint point(x, y);
            if (proxy->view()->indexAt(point).row() == 1) {
                QTest::mouseClick(proxy->view()->viewport(), Qt::LeftButton, Q_NULLPTR, point);
                break;
            }
        }

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst().value<Domain::DataSource::Ptr>(), userSelectedSource);
        QCOMPARE(combo.currentIndex(), 1);
    }
};

QTEST_MAIN(DataSourceComboBoxTest)

#include "datasourcecomboboxtest.moc"
