/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#include "editorwidget.h"
#include <KRichTextWidget>
#include <KXmlGuiWindow>
#include <KXMLGUIClient>
#include <KActionCollection>
#include <QAction>
#include <KMenuBar>

#include <QVBoxLayout>
#include <KXMLGUIFactory>
#include <ktoolbar.h>
#include <QToolButton>
#include <QEvent>

#include <KDebug>
#include <QKeyEvent>
#include <KAction>
#include <KLocalizedString>

EditorWidget::EditorWidget(QWidget *parent)
:   QWidget(parent),
    m_editor(new KRichTextWidget(this)),
    m_fullscreenButton(0),
    m_toolbar(0),
    m_defaultColor(palette().color(QPalette::Window))
{
    m_editor->setWordWrapMode(QTextOption::WordWrap);
    m_editor->setRichTextSupport( KRichTextWidget::FullTextFormattingSupport
    | KRichTextWidget::FullListSupport
    | KRichTextWidget::SupportAlignment
    | KRichTextWidget::SupportRuleLine
    | KRichTextWidget::SupportFormatPainting
    | KRichTextWidget::SupportHyperlinks
    );

    /*QPalette pe = m_editor->palette();
    pe.setColor(QPalette::Window, Qt::blue);
    m_editor->setPalette(pe);
    setAutoFillBackground(true);*/

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addStretch();
    h->addWidget(m_editor, 1);
    h->addStretch();

    l->addLayout(h);
    setLayout(l);

    setAutoFillBackground(true);
    
}


void EditorWidget::setXmlGuiClient(KXMLGUIClient *xmlGuiClient)
{
   // KXMLGUIBuilder *builder = new KXMLGUIBuilder(this);
   // KXMLGUIFactory *factory = new KXMLGUIFactory(builder, this);
    //xmlGuiClient->setClientBuilder(builder);
    //xmlGuiClient->setFactory(factory);
    
    //factory->addClient(xmlGuiClient);
    
 
    m_editor->createActions(xmlGuiClient->actionCollection()); //for the toolbar in this window

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    m_toolbar = new KToolBar("TextEditorToolbar", this);
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_bold"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_italic"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_underline"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_strikeout"));
    
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_foreground_color"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_text_background_color"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_list_style"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_list_indent_more"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_list_indent_less"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_font_family"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_font_size"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("format_painter"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("manage_link"));
    m_toolbar->addAction(xmlGuiClient->actionCollection()->action("insert_horizontal_rule"));

    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    l->addWidget(m_toolbar);

    KAction *action = xmlGuiClient->actionCollection()->addAction( "hide_toolbar" );
    action->setText( i18n( "Hide &Toolbar" ) );
    action->setCheckable(true);
    action->setShortcut( QKeySequence( Qt::Key_F6) );
    connect( action, SIGNAL(triggered()), this, SLOT(toggleToolbarVisibility()) );
    connect (this, SIGNAL(toolbarVisibilityToggled(bool)), action, SLOT(setChecked(bool))); 

    m_fullscreenButton = new QToolButton(this);
    m_fullscreenButton->setArrowType(Qt::UpArrow);
    connect(m_fullscreenButton, SIGNAL(clicked(bool)), this, SIGNAL(fullscreenToggled(bool)));
    l->addWidget(m_fullscreenButton);
    static_cast<QVBoxLayout*>(layout())->insertLayout(0,l);
}


void EditorWidget::changeEvent(QEvent *event)
{
    if (!m_fullscreenButton) {
        kWarning() << "not yet initialized";
        QWidget::changeEvent(event);
        return;
    }
    if (event->type() & QEvent::WindowStateChange) {
        QPalette p = palette();
        if (windowState() & Qt::WindowFullScreen) {
            m_fullscreenButton->setArrowType(Qt::DownArrow);
            p.setColor(QPalette::Window, Qt::white);
            //The editor should use all available space, except in fullscreen mode where it should be like a piece of paper (to avoid having all the text on the left edge of the screen
            m_editor->setMaximumWidth(1000); //FIXME calculate based on fontmetrics
        } else {
            m_fullscreenButton->setArrowType(Qt::UpArrow);
            p.setColor(QPalette::Window, m_defaultColor);
            m_editor->setMaximumWidth(QWIDGETSIZE_MAX);
        }
        setPalette(p);
    }
    QWidget::changeEvent(event);
}

void EditorWidget::toggleToolbarVisibility()
{
    m_fullscreenButton->setVisible(!m_toolbar->isVisible());
    m_toolbar->setVisible(!m_toolbar->isVisible());
    emit toolbarVisibilityToggled(!m_toolbar->isVisible());
   // KConfigGroup config(KGlobal::config(), "EditorWidget");
   // config.writeEntry("toolbarHidden", m_toolbar->isVisible());
}


KRichTextWidget* EditorWidget::editor()
{
    return m_editor;
}

void EditorWidget::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) && (windowState() & Qt::WindowFullScreen)) {
        emit fullscreenToggled(false);
        event->accept();
    }
    QWidget::keyPressEvent(event);
}


