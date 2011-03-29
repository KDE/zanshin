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

#include "qcompare.h"

#include <algorithm>

#include "modelbuilderbehavior.h"

#include <KDE/Akonadi/Item>
#include <KDE/Akonadi/Collection>
#include <QModelIndex>

Q_DECLARE_METATYPE(QModelIndexList)

static QList<int> s_itemRoles;

QList<int> QTest::getEvaluatedItemRoles()
{
    return s_itemRoles;
}

void QTest::setEvaluatedItemRoles(const QList<int> &roles)
{
    s_itemRoles = roles;
}

template<typename T>
QString toString(const T &t)
{
    return t.toString();
}

template<>
QString toString(const QString &s)
{
    return s;
}

QString toString(const char *s)
{
    return QString::fromUtf8(s);
}

template<>
QString toString(const Akonadi::Item &item)
{
    return QString("ID: %1, RemoteID: %2, UID: %3, \"%4\"")
           .arg(item.id())
           .arg(item.remoteId())
           .arg("uid?")
           .arg("summary?");
}

template<>
QString toString(const Akonadi::Collection &collection)
{
    return QString("ID: %1, RemoteID: %2, \"%3\"")
           .arg(collection.id())
           .arg(collection.remoteId())
           .arg(collection.name());
}

template<>
QString toString(const QModelIndexList &list)
{
    QString result = "(";

    foreach (const QModelIndex &index, list) {
        result+= "[0x" + QString::number(index.internalId(), 16) + ": "
               + QString::number(index.row()) + ", "
               + QString::number(index.column()) + ", "
               + "0x" + QString::number(index.parent().internalId(), 16) + "], ";
    }
    result.chop(2);

    result+= ')';
    return result;
}

template<typename T1, typename T2>
static bool dumpError(const QString &message,
                      const T1 &actualValue, const T2 &expectedValue,
                      const char *actual, const char *expected,
                      const char *file, int line)
{
    QString str1 = toString(actualValue);
    char *data1 = new char[str1.toLocal8Bit().size() + 1];
    strcpy(data1, str1.toLocal8Bit().data());

    QString str2 = toString(expectedValue);
    char *data2 = new char[str2.toLocal8Bit().size() + 1];
    strcpy(data2, str2.toLocal8Bit().data());

    return QTest::compare_helper(false, message.toLocal8Bit().constData(),
                                 data1, data2,
                                 actual, expected, file, line);
}

template<typename T>
bool compareVariantsAs(const QVariant &value1, const QVariant &value2, int depth, int role,
                       const char *actual, const char *expected,
                       const char *file, int line)
{
    if (!value1.canConvert<T>()) {
        return true;
    }

    T t1 = value1.value<T>();
    T t2 = value2.value<T>();

    if (t1==t2) {
        return true;
    }

    return dumpError(QString("Different values found at depth %1 for role %2").arg(depth).arg(role),
                     t1, t2, actual, expected, file, line);
}

template<>
bool compareVariantsAs<QVariant>(const QVariant &value1, const QVariant &value2, int depth, int role,
                     const char *actual, const char *expected,
                     const char *file, int line)
{
    if (value1.type()==QVariant::UserType) {
        return true;
    }

    if (value1==value2) {
        return true;
    }

    return dumpError(QString("Different values found at depth %1 for role %2").arg(depth).arg(role),
                     value1, value2, actual, expected, file, line);
}

static bool compareItems(const QModelIndex &index1, const QModelIndex &index2, int depth,
                         const char *actual, const char *expected,
                         const char *file, int line)
{
    foreach (int role, s_itemRoles) {
        QVariant value1 = index1.data(role);
        QVariant value2 = index2.data(role);

        if (value1.userType()!=value2.userType()) {
            return dumpError(QString("Different types found at depth %1 for role %2").arg(depth).arg(role),
                             value1.typeName(),
                             value2.typeName(),
                             actual, expected, file, line);
        }

        if (!compareVariantsAs<Akonadi::Item>(value1, value2, depth, role, actual, expected, file, line)
         || !compareVariantsAs<Akonadi::Collection>(value1, value2, depth, role, actual, expected, file, line)
         || !compareVariantsAs<QModelIndexList>(value1, value2, depth, role, actual, expected, file, line)
         || !compareVariantsAs<QVariant>(value1, value2, depth, role, actual, expected, file, line)) {
            return false;
        }
    }

    return true;
}

static bool compareBranches(const QAbstractItemModel *model1, const QAbstractItemModel *model2,
                            const QModelIndex &root1, const QModelIndex &root2, int depth,
                            const char *actual, const char *expected,
                            const char *file, int line)
{
    if (root1.isValid()==root2.isValid()) {
        if (root1.isValid() // No need to compare invalid indexes
         && !compareItems(root1, root2, depth, actual, expected, file, line)) {
            return false;
        }
    } else { // One valid, not the other!
        return dumpError(QString("One branch valid while the other is invalid at depth ")+QString::number(depth),
                                 root1.isValid() ? "true" : "false",
                                 root2.isValid() ? "true" : "false",
                                 actual, expected, file, line);
    }

    int rowCount1 = model1->rowCount(root1);
    int rowCount2 = model2->rowCount(root2);

    if (rowCount1!=rowCount2) {
        return dumpError(QString("Different row counts at depth ")+QString::number(depth),
                         QString::number(rowCount1),
                         QString::number(rowCount2),
                         actual, expected, file, line);
    }

    if (rowCount1==0) {
        // Nothing to see, we're done
        return true;
    }

    int columnCount1 = model1->columnCount(root1);
    int columnCount2 = model2->columnCount(root2);

    if (columnCount1!=columnCount2) {
        return dumpError(QString("Different column counts at depth ")+QString::number(depth),
                         QString::number(columnCount1),
                         QString::number(columnCount2),
                         actual, expected, file, line);
    }

    for (int row = 0; row<rowCount1; row++) {
        for (int col = 0; col<columnCount1; col++) {
            QModelIndex child1 = model1->index(row, col, root1);
            QModelIndex child2 = model2->index(row, col, root2);

            if (!compareBranches(model1, model2, child1, child2, depth+1,
                                 actual, expected, file, line)) {
                return false;
            }
        }
    }

    return true;
}

bool QTest::qCompare(const QAbstractItemModel &model1, const QAbstractItemModel &model2,
                     const char *actual, const char *expected,
                     const char *file, int line)
{
    bool result = compareBranches(&model1, &model2,
                                QModelIndex(), QModelIndex(), 0,
                                actual, expected, file, line);

    if (result) {
        return compare_helper(true, "COMPARE()", file, line);
    } else {
        return false;
    }
}

