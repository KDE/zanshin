/* This file is part of Zanshin Todo.

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

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
#include <core/pimitemstructurecache.h>

Q_DECLARE_METATYPE(QModelIndex)

class VirtualRelationCacheTest : public QObject
{
    Q_OBJECT
private slots:
//     void initTestCase()
//     {
//         QTest::setEvaluatedItemRoles(roles);
//     }

    static Akonadi::Item getEventItem(const QList<PimItemRelation> &relations = QList<PimItemRelation>())
    {
        IncidenceItem inc(PimItemIndex::Event);
        inc.setRelations(relations);
        Akonadi::Item item = inc.getItem();
        item.setId(1);
        return item;
    }
    
    static Akonadi::Item getContextItem(Akonadi::Item::Id id = 1, const PimItemTreeNode &node = PimItemTreeNode("uid", "name"))
    {
        PimItemRelation rel(PimItemRelation::Context, QList<PimItemTreeNode>() << node);
        Akonadi::Item item = getEventItem(QList<PimItemRelation>() << rel);
        item.setId(id);
        return item;
    }

    static Akonadi::Item getProjectItem(Akonadi::Item::Id id = 1, const PimItemTreeNode &node = PimItemTreeNode("uid", "name"))
    {
        PimItemRelation rel(PimItemRelation::Project, QList<PimItemTreeNode>() << node);
        Akonadi::Item item = getEventItem(QList<PimItemRelation>() << rel);
        item.setId(id);
        return item;
    }

    static QByteArray getUid(const Akonadi::Item &item)
    {
        IncidenceItem inc(item);
        return inc.getUid().toLatin1();
    }

    PimItemStructureCache *getStructure()
    {
        return new PimItemStructureCache(PimItemRelation::Context);
    }

    void returnCorrectItemId()
    {
        Akonadi::Item item = getEventItem();
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item);
        QVERIFY(id >= 0);
        QCOMPARE(relation->getItemId(item), id);
    }
    
    void returnEmptyParents()
    {
        Akonadi::Item item = getEventItem();
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item);
        qDebug() << relation->getParents(id);
        QCOMPARE(relation->getParents(id), IdList());
    }

    void getParents()
    {
        Akonadi::Item item = getContextItem(1, PimItemTreeNode("uid", "name"));
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getItemId(item), id);
        QCOMPARE(relation->getParents(id).size(), 1);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
    }
    
    void checkParentHierarchy()
    {
        PimItemTreeNode parent2("uid2", "name2");
        PimItemTreeNode parent1("uid", "name", QList<PimItemTreeNode>() << parent2);
        Akonadi::Item item = getContextItem(1, parent1);
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
        QCOMPARE(relation->getParents(firstParent).size(), 1);
        Id secondParent = relation->getParents(firstParent).first();
        QCOMPARE(relation->getName(secondParent), QLatin1String("name2"));
    }
    
    void mergeAndOverwriteName()
    {
        PimItemTreeNode parent2("uid2", "name2");
        PimItemTreeNode parent1("uid", "name", QList<PimItemTreeNode>() << parent2);
        Akonadi::Item item1 = getContextItem(1, parent1);
        
        PimItemTreeNode parent4("uid2", "name3");
        PimItemTreeNode parent3("uid", "name", QList<PimItemTreeNode>() << parent4);
        Akonadi::Item item2 = getContextItem(2, parent3);
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item1);
        relation->addItem(item2);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
        QCOMPARE(relation->getParents(firstParent).size(), 1);
        Id secondParent = relation->getParents(firstParent).first();
        QCOMPARE(relation->getName(secondParent), QLatin1String("name3"));
    }
    
    /*
     * Each child contains the information about all parents.
     * Or in other words: with virtual parents the child defines the parent structure.
     */
    void cleanupParentsAfterChildInsert()
    {
        Akonadi::Item item = getContextItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getContextItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<VirtualRelationCache> relation(getStructure());
        Id id = relation->addItem(item);
        relation->addItem(item2);
        QCOMPARE(relation->getParents(id).size(), 0);
    }
    


    //Project tests
    void getParentProject()
    {
        Akonadi::Item item = getProjectItem();
        
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getItemId(item), id);
        QCOMPARE(relation->getParents(id).size(), 1);
        relation->getParents(id).first();
    }
    
    void matchProjectByUid()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        Id id = relation->addItem(item);
        Id id2 = relation->addItem(item2);
        QCOMPARE(relation->getParents(id2).first(), id);
    }
  
    void matchProjectByUidReverse()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        Id id2 = relation->addItem(item2);
        Id id = relation->addItem(item);
        QCOMPARE(relation->getParents(id2).first(), id);
    }
    
    /*
     * Children only hold their parent project, but projects are themselves already in a structure.
     * Therfore we don't want to loose the parent projects structure when inserting a new item.
     */
    void dontLooseParentInformationOnChildInsert()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        Id id = relation->addItem(item);
        relation->addItem(item2);
        QCOMPARE(relation->getParents(id).size(), 1);
    }
    
    void getIdFromUid()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getId(getUid(item)), id);
    }
    
    void relatedToSelf()
    {
        IncidenceItem inc(PimItemIndex::Event);
        PimItemRelation rel(PimItemRelation::Project, QList<PimItemTreeNode>() << PimItemTreeNode(inc.getUid().toLatin1(), "name") << PimItemTreeNode("uid", "name"));
        inc.setRelations(QList<PimItemRelation>() << rel);
        Akonadi::Item item = inc.getItem();
        item.setId(1);
        
        QScopedPointer<ProjectStructureCache> relation(new ProjectStructureCache());
        const Id id = relation->addItem(item);
        QCOMPARE(relation->getId(getUid(item)), id);
        const IdList parents = relation->getParents(id);
        //TODO once we support multiple parent relations, make sure that the second relation is still available
        QVERIFY(parents.isEmpty());
//         QCOMPARE(parents.size(), 1);
//         QCOMPARE(relation->getUid(parents.at(0)), QByteArray("uid"));
    }

};

QTEST_KDEMAIN(VirtualRelationCacheTest, GUI)

#include "pimitemrelationstest.moc"
