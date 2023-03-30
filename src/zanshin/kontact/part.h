/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHIN_PART_H
#define ZANSHIN_PART_H

#include <KParts/ReadOnlyPart>

class Part : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Part(QWidget *parentWidget, QObject *parent, const QVariantList &);
#else
    Part(QWidget *parentWidget, QObject *parent, const KPluginMetaData &data, const QVariantList &);
#endif
    ~Part();

protected:
    virtual bool openFile() override;
};

#endif

