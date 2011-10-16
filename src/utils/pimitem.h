
#ifndef PIMITEM_H
#define PIMITEM_H

#include "note.h"
#include "incidenceitem.h"
//#include "abstractpimitem.h"

namespace PimItemUtils {

    //use together with QScopedPointer
    static AbstractPimItem* getItem(const Akonadi::Item &item, QObject *parent = 0)
    {
        if (!item.isValid()) {
            kWarning() << "invalid item";
            return 0;
        }
        AbstractPimItem::ItemType itemType = AbstractPimItem::itemType(item);
        if (itemType & AbstractPimItem::Note) {
            return  new Note(item, parent);
        } else if (itemType & AbstractPimItem::Incidence) {
            return new IncidenceItem(item, parent);
        }
        return 0;
    };

    /*typedef QSharedPointer<AbstractPimItem> AbstractPimItemPtr;
    AbstractPimItemPtr getItem(const Akonadi::Item &item, QObject *parent = 0)
    {
        //Q_ASSERT(item.isValid());
        if (!item.isValid()) {
            kWarning() << "invalid item";
            return AbstractPimItemPtr();
        }
        AbstractPimItem::ItemType itemType = AbstractPimItem::itemType(item);
        if (itemType & AbstractPimItem::Note) {
            return  AbstractPimItemPtr(new Note(item, parent));
        } else if (itemType & AbstractPimItem::Incidence) {
            return AbstractPimItemPtr(new IncidenceItem(item, parent));
        }
        //Q_ASSERT(0);
        return AbstractPimItemPtr();
    };*/
/*
    static Akonadi::Item getAkonadiItem(const QModelIndex &index)
    {
        const Akonadi::Item item = index0.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if (!item.isValid()) {
            kDebug() << "invalid item";
        }
        return item;
    };
*/

    static AbstractPimItem* fromUrl( const KUrl &url )
    {
        return getItem(Akonadi::Item::fromUrl(url));
    };

}

#endif
