/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Christian Mollekopf <chrigi_1@fastmail.fm>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtGui/QWidget>

/**
 * A fully collapsible toolbox
 * 
 */
class Toolbox : public QWidget
{
    Q_OBJECT
public:
    explicit Toolbox(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~Toolbox();
    /**
     * Adds the widget to the end of the toolbox
     * The contents margins of the widget are modified.
     */
    void addWidget(QWidget *widget, const QString &title);
    int count();
    int currentIndex();
    /// -1 collapses all widgets
    void activateWidget(int index);

public slots:
    void collapseAll();
};




#endif // TOOLBOX_H
