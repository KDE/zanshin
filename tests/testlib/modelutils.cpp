/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "modelutils.h"

// Needed for roles
#include "modelbuilder.h"

using namespace Zanshin::Test;

QModelIndex ModelUtils::locateItem(QAbstractItemModel *model, const ModelPath &root)
{
    QVariantList path = root.m_path;
    QModelIndex index;

    foreach (const QVariant &pathPart, path) {
        bool found = false;

        for (int row=0; row<model->rowCount(index); row++) {
            QModelIndex childIndex = model->index(row, 0, index);
            QVariant variant = model->data(childIndex, TestDslRole);

            if (variant.userType()!=pathPart.userType()) {
                continue;
            }

            if (variant.canConvert<T>()) {
                T t1 = variant.value<T>();
                T t2 = pathPart.value<T>();
                if (t1==t2) {
                    found = true;
                }
            } else {
                Q_ASSERT(variant.canConvert<C>());
                C c1 = variant.value<C>();
                C c2 = pathPart.value<C>();
                if (c1==c2) {
                    found = true;
                }
            }

            if (found) {
                index = childIndex;
                break;
            }
        }
        Q_ASSERT(found);
    }

    return index;
}


