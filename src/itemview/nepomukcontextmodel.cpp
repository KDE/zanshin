/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Christian Mollekopf <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "nepomukcontextmodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>

#include <Nepomuk/Resource>

#include <kdebug.h>
#include <kurl.h>
#include <Nepomuk/Utils/SimpleResourceModel>
#include <Nepomuk/File>
#include <Nepomuk/Thing>


NepomukContextModel::NepomukContextModel( QObject* parent )
: Nepomuk::Utils::SimpleResourceModel( parent )
{
}


QVariant NepomukContextModel::data( const QModelIndex& index, int role ) const
{
  /*  Nepomuk::Resource res = resourceForIndex( index );
    if( !res.isValid() ) {
        return QVariant();
    }

    Nepomuk::Thing thing = res.pimoThing();

    if (thing.groundingOccurrences().size()!=1) {
        kWarning() << thing.groundingOccurrences().size() << thing.uri() << thing.genericLabel();
        kWarning() << res.uri() << res.genericLabel();
        foreach ( const Nepomuk::Resource &r, thing.groundingOccurrences()) {
            kWarning() << r.uri() << r.genericLabel();
        }
    }
    Q_ASSERT( thing.groundingOccurrences().size() == 1);
    res = thing.groundingOccurrences().first();*/

    //
    // Part 1: column specific data
    //
    /*
    switch( index.column() ) {
        case ResourceColumn:
            switch( role ) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return res.genericLabel();

                case Qt::DecorationRole: {
                    QString iconName = res.genericIcon();
                    if( !iconName.isEmpty() ) {
                        return KIcon( iconName );
                    }
                    else {
                        QIcon icon = Types::Class(res.resourceType()).icon();
                        if( !icon.isNull() )
                            return icon;
                        else
                            return QVariant();
                    }
                }

                case Qt::ToolTipRole:
                    return KUrl( res.resourceUri() ).prettyUrl();

            }

            case ResourceTypeColumn:
                switch( role ) {
                    case Qt::DisplayRole:
                    case Qt::EditRole:
                        return Types::Class( res.resourceType() ).label();

                    case Qt::DecorationRole: {
                        QIcon icon = Types::Class(res.resourceType()).icon();
                        if( !icon.isNull() )
                            return icon;
                        else
                            return QVariant();
                    }

                    case Qt::ToolTipRole:
                        return KUrl(res.resourceType()).prettyUrl();
                }

    }
    */

    return Nepomuk::Utils::SimpleResourceModel::data(index, role);

}