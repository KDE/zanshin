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


#include <testlib/qtest_gui_zanshin.h>

#include "widgets/scripteditor.h"

class ScriptEditorTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldNotHaveScriptHandler()
    {
        // GIVEN
        Widgets::ScriptEditor scriptEditor;

        // THEN
        QVERIFY(scriptEditor.scriptHandler().isNull());
    }

    void shouldHaveScriptHandler()
    {
        // GIVEN
        Widgets::ScriptEditor scriptEditor;
        auto scriptHandler = QObjectPtr::create();

        // WHEN
        scriptEditor.setScriptHandler(scriptHandler);

        // THEN
        QCOMPARE(scriptEditor.scriptHandler(), scriptHandler);
    }
};

ZANSHIN_TEST_MAIN(ScriptEditorTest)

#include "scripteditortest.moc"
