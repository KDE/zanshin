/*
   Copyright 2013 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include <qtest_kde.h>

#include <core/incidenceitem.h>
#include <core/pimitemfactory.h>
#include <core/noteitem.h>

Q_DECLARE_METATYPE(QModelIndex)

class PimItemTest : public QObject
{
    Q_OBJECT
    
private slots:
    void testGetAndSet_data()
    {
        QTest::addColumn<PimItem::Ptr>("item");
        QTest::newRow("event") << PimItem::Ptr(new IncidenceItem(PimItem::Event));
        QTest::newRow("note") << PimItem::Ptr(new NoteItem());
    }
    
    //Ensure save and load from akonadi item works
    void testGetAndSet()
    {
        QFETCH(PimItem::Ptr, item);
        item->setTitle("title");
        item->setText("text");
        
        Akonadi::Item akonadiItem = item->getItem();
        akonadiItem.setId(1);
        
        PimItem::Ptr pimitem = PimItemFactory::getItem(akonadiItem);
        QCOMPARE(pimitem->getUid(), item->getUid());
        QCOMPARE(pimitem->getTitle(), item->getTitle());
        QCOMPARE(pimitem->getText(), item->getText());
    }
    
    void testUidConsistency_data()
    {
        QTest::addColumn<PimItem::Ptr>("item");
        QTest::newRow("event") << PimItem::Ptr(new IncidenceItem(PimItem::Event));
        QTest::newRow("note") << PimItem::Ptr(new NoteItem());
    }
    
    void testUidConsistency()
    {
        QFETCH(PimItem::Ptr, item);
        const QString &uid = item->getUid();
        QVERIFY(!uid.isEmpty());
        QCOMPARE(item->getUid(), uid);
    }
};

QTEST_KDEMAIN(PimItemTest, GUI)

#include "pimitemtest.moc"
