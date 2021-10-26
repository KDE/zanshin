/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKEDATAXMLLOADER_H
#define TESTLIB_AKONADIFAKEDATAXMLLOADER_H

#include <QString>

namespace Testlib {

class AkonadiFakeData;

class AkonadiFakeDataXmlLoader
{
public:
    explicit AkonadiFakeDataXmlLoader(AkonadiFakeData *data);

    void load(const QString &fileName) const;

private:
    AkonadiFakeData *m_data;
};

}

#endif // TESTLIB_AKONADIFAKEDATAXMLLOADER_H
