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

#ifndef NOTETAKERMODEL_H
#define NOTETAKERMODEL_H

#include <Akonadi/EntityTreeModel>
#include <globaldefs.h>

namespace Akonadi
{
    class ChangeRecorder;
    class Session;
}
class QStringList;
using namespace Akonadi;

/**
 *
 * Defines which data is available in the model, and sets the headers
 */
class NotetakerModel : public EntityTreeModel
{
    Q_OBJECT
public:
    explicit NotetakerModel( ChangeRecorder *monitor, QObject *parent = 0 );
    virtual ~NotetakerModel();

    enum ItemColumn {
        Title=0,
        Date,
        Status,
        ColumnCount
    };

    //For QML
    enum CustomRoles {
        SortRole=Zanshin::UserRole,
        TitleRole,
        DateRole,
        ItemTypeRole,
        UserRole
    };

    /**
     * Implement to return the number of columns for a HeaderGroup.
     * If the HeaderGroup is CollectionTreeHeaders, return the number of columns to display for the Collection tree, and if it is ItemListHeaders,
     * return the number of columns to display for the item. In the case of addressee, this could be for example,
     * two (for given name and family name) or for emails it could be three (for subject, sender, date).
     * This is a decision of the subclass implementor.
     */
    int entityColumnCount( HeaderGroup headerGroup ) const;

    /**
     * Implement to return the data for each section for a HeaderGroup. For example, if the header group is CollectionTreeHeaders in a contacts model,
     * the string "Address books" might be returned for column 0, whereas if the headerGroup is ItemListHeaders,
     * the strings "Given Name", "Family Name", "Email Address" might be returned for the columns 0, 1, and 2.
     */
    QVariant entityHeaderData( int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup ) const;

    /**
     * Implement to return the data for a particular item and column. In the case of email for example, this would be the actual subject, sender and date of the email.
     */
    QVariant entityData( const Item &item, int column, int role = Qt::DisplayRole ) const;
    
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


private:
    QStringList *m_itemHeaders;
    QStringList *m_collectionHeaders;
};

#endif // NOTETAKERMODEL_H
