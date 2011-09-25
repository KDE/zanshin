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


#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

class KToolBar;
class QToolButton;
class KRichTextWidget;
class KXMLGUIClient;

/**
 * An editorwidget, with the toolbox buttons
 */
class EditorWidget: public QWidget
{
    Q_OBJECT
public:
    EditorWidget(QWidget *parent = 0);

    KRichTextWidget *editor();
    void setXmlGuiClient(KXMLGUIClient *);

public slots:
    void toggleToolbarVisibility();

protected:
    virtual void changeEvent(QEvent* );
    virtual void keyPressEvent(QKeyEvent* );

signals:
    void fullscreenToggled(bool);
    void toolbarVisibilityToggled(bool);

private:
    KRichTextWidget *m_editor;
    QToolButton *m_fullscreenButton;
    KToolBar *m_toolbar;
    const QColor m_defaultColor;
};

#endif // EDITORWIDGET_H
