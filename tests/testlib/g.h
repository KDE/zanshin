/* This file is part of Zanshin Todo.
 * 
 *   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
 * 
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *   USA.
 */

#ifndef G_H
#define G_H

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Zanshin
{
    namespace Test
    {
        
            struct G // Stands for Generic
            {
                public:
                    typedef QList<G> List;
                    
                    G();
                    explicit G(qint64 i, int role, const QVariant &n);
                    
                    bool operator==(const G &other) const;
                    
                    QHash<int, QVariant> data;
                    qint64 id;
            };
            
    } // namespace Test
} // namespace Zanshin
    
Q_DECLARE_METATYPE(Zanshin::Test::G)
// Q_DECLARE_METATYPE(Zanshin::Test::G::List)


#endif // G_H
