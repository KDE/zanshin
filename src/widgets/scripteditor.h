/* This file is part of Zanshin

   Copyright 2015 Theo Vaucher <theo.vaucher@gmail.com>

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


#ifndef WIDGETS_SCRIPTEDITOR_H
#define WIDGETS_SCRIPTEDITOR_H

#include <QMainWindow>

#include "presentation/metatypes.h"

class ScriptHandler;
class QTextEdit;

namespace Widgets {

class ScriptEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScriptEditor(QWidget *parent = Q_NULLPTR);
    virtual ~ScriptEditor();

    QObjectPtr scriptHandler() const;

public slots:
    void setScriptHandler(const QObjectPtr &scriptHandler);

private:
    QObjectPtr m_scriptHandler;

    QTextEdit *m_textEdit;
};

}

#endif // WIDGETS_SCRIPTEDITOR_H
