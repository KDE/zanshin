/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef WIDGETS_APPLICATIONCOMPONENTS_H
#define WIDGETS_APPLICATIONCOMPONENTS_H

#include <QObject>

#include "domain/artifact.h"

#include "presentation/metatypes.h"

class QWidget;

namespace Widgets {

class AvailablePagesView;
class DataSourceComboBox;
class EditorView;
class PageView;

class ApplicationComponents : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationComponents(QWidget *parent = 0);

    QObject *model() const;

    AvailablePagesView *availablePagesView() const;
    PageView *pageView() const;
    EditorView *editorView() const;

    DataSourceComboBox *defaultNoteSourceCombo() const;
    DataSourceComboBox *defaultTaskSourceCombo() const;

public slots:
    void setModel(QObject *model);

private slots:
    void onCurrentPageChanged(QObject *page);
    void onCurrentArtifactChanged(const Domain::Artifact::Ptr &artifact);

private:
    QObject *m_model;

    QWidget *m_parent;
    AvailablePagesView *m_availablePagesView;
    PageView *m_pageView;
    EditorView *m_editorView;
    DataSourceComboBox *m_noteCombo;
    DataSourceComboBox *m_taskCombo;
};

}

#endif // WIDGETS_APPLICATIONCOMPONENTS_H
