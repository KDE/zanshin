/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#include "nepomukcontextmodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>

#include <Nepomuk2/Resource>

#include <kdebug.h>
#include <kurl.h>
// #include <Nepomuk2/Utils/SimpleResourceModel>
#include <Nepomuk2/File>
#include <Nepomuk2/Thing>


NepomukContextModel::NepomukContextModel( QObject* parent )
: /*Nepomuk2::Utils::SimpleResourceModel( parent )*/ QObject(parent)
{
}


QVariant NepomukContextModel::data( const QModelIndex& index, int role ) const
{
  /*  Nepomuk2::Resource res = resourceForIndex( index );
    if( !res.isValid() ) {
        return QVariant();
    }

    Nepomuk2::Thing thing = res.pimoThing();

    if (thing.groundingOccurrences().size()!=1) {
        kWarning() << thing.groundingOccurrences().size() << thing.uri() << thing.genericLabel();
        kWarning() << res.uri() << res.genericLabel();
        foreach ( const Nepomuk2::Resource &r, thing.groundingOccurrences()) {
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

//     return Nepomuk2::Utils::SimpleResourceModel::data(index, role);
    return QVariant();
}