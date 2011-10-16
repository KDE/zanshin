
#ifndef PIMITEM_H
#define PIMITEM_H

#include "abstractpimitem.h"
//#include "abstractpimitem.h"

namespace PimItemUtils {

    //use together with QScopedPointer
    AbstractPimItem* getItem(const Akonadi::Item &item, QObject *parent = 0);

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

    AbstractPimItem* fromUrl( const KUrl &url );
    /**
     * Returns the resource for an item
     * The Resource represents the data
     * The resource can be used for accessing data, tagging, relating , etc should be done on the thing
     *
     * The passed item needs to have a valid mimetype, othwise the resource cannot be created
     */
    Nepomuk::Resource getResource(const Akonadi::Item &item);
    /**
     * Returns the thing of and item
     * Each thing can have multiple resources as grounding occurences, (i.e. if the item is stored in several places),
     * but the thing is for each item unique.
     * This means, tagging, relating ,etc should be done on the Thing, as it should be global
     *
     * The passed item needs to have a valid mimetype, othwise the resource cannot be created
     */
    Nepomuk::Thing getThing(const Akonadi::Item &item);

    Akonadi::Item getItemFromResource(const Nepomuk::Resource &resource);

    Nepomuk::Thing getExistingThing(const Akonadi::Item &item);
}

#endif
