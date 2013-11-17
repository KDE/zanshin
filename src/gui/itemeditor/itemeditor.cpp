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

#include "itemeditor.h"

#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <KRichTextWidget>
#include <KMime/KMimeMessage>
#include <boost/shared_ptr.hpp>
#include <kpimtextedit/textutils.h>

#include <KInputDialog>

#include <kxmlguiclient.h>
#include <kxmlguiwindow.h>
#include <kmenubar.h>
#include <KDE/KToolBar>
#include <QToolBar>
#include <KActionCollection>
#include <qtoolbox.h>

#include <kdatetimewidget.h>
#include <QCheckBox>
#include <QHeaderView>

#include <QTimer>

#include "utils/datestringbuilder.h"
#include <QInputDialog>
#include <QApplication>
#include <QLayout>
#include <KAction>

#include "toolbox.h"
#include "ui_itemview.h"
#include "ui_properties.h"
#include <KConfigGroup>
#include "core/akonadiincidenceitem.h"
#include "core/pimitemfactory.h"

using namespace Ui;

ItemEditor::ItemEditor(QWidget* parent, KXMLGUIClient *parentClient)
:   QFrame(parent),
    KXMLGUIClient(parentClient),
    m_itemMonitor(0),
    m_autosaveTimer(new QTimer(this)),
    m_autosaveTimeout(5000),
    ui_itemView(new itemView),
    ui_properties(new properties()),
    m_attachmentsViewer(new AttachmentsViewer(this))
{
    ui_itemView->setupUi(this);

    setXMLFile("editorui.rc");

    //Title
    //TODO set focus to end of edit, instead of beginning
    connect(&ui_itemView->title->lineEdit(), SIGNAL(returnPressed()), ui_itemView->editor->editor(), SLOT(setFocus()));
    ui_itemView->title->lineEdit().setPlaceholderText(i18n("Title..."));
    QFont font = QApplication::font();
    font.setBold(true);
    font.setPointSize(11);
    ui_itemView->title->setDisplayFont(font);
    
    //Editor
    ui_itemView->editor->setXmlGuiClient(this);
    connect(ui_itemView->editor, SIGNAL(fullscreenToggled(bool)), SLOT(setFullscreenEditor()));
    
    KAction *action = actionCollection()->addAction( "fullscreen_editor" );
    action->setText( i18n( "Fullscreen &Editor" ) );
    action->setIcon( KIcon( "go-up" ) );
    action->setShortcut(QKeySequence(Qt::Key_F5));
    connect( action, SIGNAL(triggered()), SLOT(setFullscreenEditor()) );
    
    ui_itemView->editor->addAction(action);
    
    
    static_cast<QVBoxLayout*>(layout())->setStretchFactor(ui_itemView->editor, 5);

    //Toolbox
    QWidget *propertiesWidget = new QWidget(ui_itemView->toolbox);
    ui_properties->setupUi(propertiesWidget);
    connect(ui_properties->editableDueDate, SIGNAL(dateChanged(KDateTime, bool)), this, SLOT(setDueDate(KDateTime)));
    connect(ui_properties->editableEventDate, SIGNAL(dateChanged(KDateTime)), this, SLOT(setEventDate(KDateTime)));
    ui_itemView->toolbox->addWidget(propertiesWidget, i18n("Properties"));

    ui_itemView->toolbox->addWidget(m_attachmentsViewer, i18n("Attachments"));
    
    setEnabled(false);

    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(autosave()));
    QTimer::singleShot(0, this, SLOT(restoreState())); //delayed so we can check if the toolbar is visible or not
}

ItemEditor::~ItemEditor()
{
    KConfigGroup config(KGlobal::config(), "ItemEditor");
    config.writeEntry("activeToolbox", ui_itemView->toolbox->currentIndex());
    config.writeEntry("toolbarHidden", actionCollection()->action( "hide_toolbar" )->isChecked()); //The widget is already hidden, but the action still has the correct state

    saveItem();
    if (m_currentItem) {
        disconnect(m_itemMonitor, 0, this, 0);
        m_currentItem.clear();
    }

    delete ui_properties;
    ui_properties = 0;
    delete ui_itemView;
    ui_itemView = 0;
}

void ItemEditor::restoreState()
{
    KConfigGroup config(KGlobal::config(), "ItemEditor");
    int activeToolbox= config.readEntry( "activeToolbox", 0);
    ui_itemView->toolbox->activateWidget(activeToolbox);
    bool toolbarHidden = config.readEntry( "toolbarHidden", false);
    if (toolbarHidden) {
        ui_itemView->editor->toggleToolbarVisibility();
    }
}


void ItemEditor::setFullscreenEditor()
{
    if (!m_currentItem) {
        return;
    }
    if (ui_itemView->editor->windowState() & Qt::WindowFullScreen) {
        ui_itemView->editor->setParent(this);
        static_cast<QVBoxLayout*>(layout())->insertWidget(1, ui_itemView->editor, 5);
    } else {
        ui_itemView->editor->setParent(0);
    }
    ui_itemView->editor->setWindowState(ui_itemView->editor->windowState() ^ Qt::WindowFullScreen);
    ui_itemView->editor->show();
}


void ItemEditor::autosave()
{
    //kDebug();
    saveItem();
}


void ItemEditor::saveItem()
{
    //kDebug();
    if (!m_currentItem) {
        kDebug() << "no item set";
        return;
    }
    bool modified = false;
    
    if (ui_itemView->editor->editor()->document()->isModified()) {
        bool isRichText = KPIMTextEdit::TextUtils::containsFormatting( ui_itemView->editor->editor()->document() );
        if (isRichText) {
            m_currentItem->setText(ui_itemView->editor->editor()->toHtml(), true);
        } else {
            m_currentItem->setText(ui_itemView->editor->editor()->toPlainText(), false);
        }
        ui_itemView->editor->editor()->document()->setModified(false);
        modified = true;
    }

    if (ui_itemView->title->lineEdit().isModified()) {
        m_currentItem->setTitle(ui_itemView->title->text());
        ui_itemView->title->lineEdit().setModified(false);
        modified = true;
    }
    if (modified) {
        kDebug() << "save item";
        m_itemMonitor->saveItem();
    }
}
/*
 TODO this widget never has the focus, check the focus of the editor->editor() instead
void ItemEditor::focusInEvent(QFocusEvent* event)
{
    connect(title, SIGNAL(editingFinished()), this, SLOT(saveItem()));
    QWidget::focusInEvent(event);
}


void ItemEditor::focusOutEvent(QFocusEvent* event)
{
    kDebug();
    //So the item is saved only once, if we loose focus after editing the title
    disconnect(title, SIGNAL(editingFinished()), this, SLOT(saveItem()));
    saveItem();
    QWidget::focusOutEvent(event);
}*/

void ItemEditor::clearView()
{
    m_autosaveTimer->stop();
    ui_itemView->editor->editor()->clear();
    //Reset action from last text (i.e. if bold text was enabled)
    /*foreach (QAction *action, guiWindow->actionCollection()->actions()) {
        kDebug() << "reset: " << action->text();
        action->setChecked(false);
    }*/
    //Reset formatting actions from last text (i.e. if bold text was enabled)
    //maybe it would be cleaner to set the default values in the QDocument (i.e. setDefaultFont())
    ui_itemView->editor->editor()->switchToPlainText();
    ui_itemView->editor->editor()->enableRichTextMode();

    ui_itemView->title->clear();
    ui_itemView->title->lineEdit().setModified(false);
    ui_itemView->editor->editor()->document()->setModified(false);
    //we're not editing anymore, so clear focus. Otherwise a conflict will be detected
    ui_itemView->editor->editor()->clearFocus();
    ui_itemView->title->lineEdit().clearFocus();

    if (m_itemMonitor) {
        disconnect(m_itemMonitor, 0, this, 0);
        m_itemMonitor->deleteLater();
        m_itemMonitor = 0;
    }
    if (m_currentItem) {
        m_currentItem.clear();
    }
}

void ItemEditor::setItem(const KUrl &url)
{
    Akonadi::Item item = Akonadi::Item::fromUrl(url);
    if (!item.isValid()) {
        kWarning() << "invalid item passed" << url;
        return;
    }
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item, this);
    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(itemsReceived(Akonadi::Item::List)));
}

void ItemEditor::itemsReceived( const Akonadi::Item::List &list )
{
    if (list.isEmpty()) {
        kWarning() << "no items retrieved";
        return;
    }
    Q_ASSERT(list.size() == 1);
    setItem(list.first());
}


void ItemEditor::setItem(const Akonadi::Item& item)
{
    kDebug();
    //reset pending signals from last item
    disconnect(&ui_itemView->title->lineEdit(), SIGNAL(editingFinished()), this, SLOT(saveItem()));
    emit itemChanged();
    saveItem();

    clearView();

    connect(&ui_itemView->title->lineEdit(), SIGNAL(editingFinished()), this, SLOT(saveItem())); //update title also in listview as soon as it is set
    if (!item.isValid()) {
        setEnabled(false);
        kWarning() << "invalid item";
        return;
    }

    m_currentItem = PimItemFactory::getItem(item);
    m_itemMonitor = new PimItemMonitor(m_currentItem, this);
    setEnabled(true);
    connect(m_itemMonitor, SIGNAL(payloadFetchComplete()), this, SLOT(updateContent()));
    connect(m_itemMonitor, SIGNAL(changed(PimItemMonitor::ChangedParts)), this, SLOT(updateContent(PimItemMonitor::ChangedParts)));
    connect(m_itemMonitor, SIGNAL(removed()), this, SLOT(itemRemoved()));
}

void ItemEditor::updateContent(PimItemMonitor::ChangedParts parts)
{
    kDebug();
    Q_ASSERT(ui_properties);
    /*
     * TODO check for changed content, if there is changed content we have a conflict,
     * and the user should be allowed to save the current content
     */
    if ((ui_itemView->editor->editor()->hasFocus() && (parts & PimItemMonitor::Text)) || (ui_itemView->title->lineEdit().hasFocus() && (parts & PimItemMonitor::Title))) { //were currently editing, and the item has changed in the background, so there is probably a conflict
        kWarning() << "conflict";
        KDialog *dialog = new KDialog( this );
        dialog->setCaption( "Conflict" );
        dialog->setMainWidget(new QLabel("Discard current changes and use update data?", dialog)); //TODO show text of changed item
        dialog->setButtons( KDialog::Yes | KDialog::No);
        int result = dialog->exec();
        if (result != KDialog::Yes) {
            return;
        }
    }

    if (parts & PimItemMonitor::Text) {
        kDebug() << "text changed";
        ui_itemView->editor->editor()->setTextOrHtml(m_currentItem->text());
        ui_itemView->editor->editor()->document()->setModified(false);
    }

    if (parts & PimItemMonitor::Title) {
        kDebug() << "title changed";
        ui_itemView->title->setText(m_currentItem->title());
        ui_itemView->title->lineEdit().setModified(false);
    }


    //Properties
    ui_properties->creationTime->setText(DateStringBuilder::getFullDate(m_currentItem->date(PimItem::CreationDate)));
    ui_properties->lastModifiedTime->setText(DateStringBuilder::getFullDate(m_currentItem->date(PimItem::LastModifiedDate)));

    if (m_currentItem->itemType() == PimItem::Todo) {
        AkonadiIncidenceItem::Ptr inc = m_currentItem.staticCast<AkonadiIncidenceItem>();
        //Due Date
        bool hasDue = inc->supportedDateRoles() & PimItem::DueDate;

        ui_properties->editableDueDate->show();
        ui_properties->lb_dueDate->show();
        if (hasDue) {
            ui_properties->editableDueDate->setDate(inc->date(PimItem::DueDate));
        } else {
            ui_properties->editableDueDate->clear();
        }
        ui_properties->editableDueDate->enable(inc->date(PimItem::DueDate).isValid()); //set checked status

    } else { //not a todo
        ui_properties->editableDueDate->hide();
        ui_properties->lb_dueDate->hide();
    }

    if (m_currentItem->itemType() == PimItem::Event) {
        AkonadiIncidenceItem::Ptr inc = m_currentItem.staticCast<AkonadiIncidenceItem>();
        //Event Start
        ui_properties->editableEventDate->show();
        ui_properties->lb_eventDate->show();
        ui_properties->editableEventDate->setDate(inc->date(PimItem::StartDate));
    } else { //not a todo
        ui_properties->lb_eventDate->hide();
        ui_properties->editableEventDate->hide();
    }
    
    m_attachmentsViewer->updateAttachments(m_currentItem->attachments());
    
    //Set Focus for new items to title bar
    //TODO If it should be possible to have notes without titles this is not a good idea, but otherwise it works very well
    //kDebug() << m_currentItem->getLastModifiedDate() << KDateTime(QDateTime::currentDateTime().addSecs(-1));
    //FIXME this is broken if we going trough the list with the keyboard, looking at each note, because the focus doesn't stay on the list but goes to the linedit instead
    if (m_currentItem->title().isEmpty() && m_currentItem->date(PimItem::CreationDate) >= KDateTime(QDateTime::currentDateTime().addSecs(-30))) {
            ui_itemView->title->edit();
            ui_itemView->title->lineEdit().setFocus();
            //TODO work with a focus proxy instead?
            //only set the focusproxy, and wait until this widget gets focus, it will then forward the focus to either the titel or textedit
    }
    if (m_autosaveTimeout) {
        m_autosaveTimer->start(m_autosaveTimeout);
    }

  /*properties->setAutoFillBackground(true);
    QPalette p;
    p.setColor(QPalette::Window, QPalette::Dark);
    properties->setPalette(p);


   properties->adjustSize();
    properties->updateGeometry();
    toolBox->adjustSize();
    toolBox->updateGeometry();
    toolBox->layout()->update();
    kDebug() << properties->sizeHint() << toolBox->sizeHint();
*/
}

void ItemEditor::setEventDate(KDateTime dateTime)
{
    if (!m_currentItem) {
        return;
    }
    Q_ASSERT(m_currentItem);
    if (m_currentItem->itemType() == PimItem::Event) {
        AkonadiIncidenceItem::Ptr inc = m_currentItem.staticCast<AkonadiIncidenceItem>();
        inc->setDate(PimItem::StartDate, dateTime);
        m_itemMonitor->saveItem();
    }
}


void ItemEditor::setDueDate(KDateTime dateTime)
{
    if (!m_currentItem) {
        return;
    }
    Q_ASSERT(m_currentItem);
    if (m_currentItem->itemType() == PimItem::Todo) {
        AkonadiIncidenceItem::Ptr inc = m_currentItem.staticCast<AkonadiIncidenceItem>();
        inc->setDate(PimItem::DueDate, dateTime);
        m_itemMonitor->saveItem();
    }
}

void ItemEditor::itemRemoved()
{
    clearView();
    setEnabled(false);
}
