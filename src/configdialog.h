/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef ZANSHIN_CONFIGDIALOG_H
#define ZANSHIN_CONFIGDIALOG_H

#include <akonadi/collection.h>

#include <KDE/KConfigDialog>

namespace Akonadi
{
    class CollectionView;
}

class GlobalSettings;

class ConfigDialog : public KConfigDialog
{
    Q_OBJECT

public:
    ConfigDialog(QWidget *parent, const QString &name, GlobalSettings *settings);

protected slots:
    virtual void updateSettings();
    virtual void updateWidgets();
    virtual void updateWidgetsDefault();

protected:
    virtual bool hasChanged();
    virtual bool isDefault();
private:
    Akonadi::Collection::Id selectedCollection();
    Akonadi::CollectionView *m_collectionList;
    GlobalSettings *m_settings;
};

#endif

