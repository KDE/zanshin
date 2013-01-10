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

#include "itemviewer.h"

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

#include "itemcontext.h"
#include "pimitem.h"

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

#include "datestringbuilder.h"
#include <QInputDialog>
#include <QApplication>
#include <QLayout>
#include <KAction>

#include "toolbox.h"
#include "itemmonitor.h"
// #include "ui_tags.h"
#include "ui_properties.h"
#include <KConfigGroup>
#include <incidenceitem.h>
#include <kurl.h>
#include <krun.h>
#include <KTemporaryFile>
#include <kmimetype.h>
#include <kmessagebox.h>

using namespace Ui;

ItemViewer::ItemViewer(QWidget* parent, KXMLGUIClient *parentClient)
:   QFrame(parent),
    KXMLGUIClient(parentClient),
    m_currentItem(0),
    m_autosaveTimer(new QTimer(this)),
    m_autosaveTimeout(5000),
    ui_properties(new properties()),
    m_attachmentsList(new QListWidget(this))
{
    setupUi(this);

    setXMLFile("editorui.rc");

    //Title
    //TODO set focus to end of edit, instead of beginning
    connect(&title->lineEdit(), SIGNAL(returnPressed()), editor->editor(), SLOT(setFocus()));
    title->lineEdit().setPlaceholderText(i18n("Title..."));
    QFont font = QApplication::font();
    font.setBold(true);
    font.setPointSize(11);
    title->setDisplayFont(font);
    
    //Editor
    editor->setXmlGuiClient(this);
    connect(editor, SIGNAL(fullscreenToggled(bool)), SLOT(setFullscreenEditor()));
    
    KAction *action = actionCollection()->addAction( "fullscreen_editor" );
    action->setText( i18n( "Fullscreen &Editor" ) );
    action->setIcon( KIcon( "go-up" ) );
    action->setShortcut(QKeySequence(Qt::Key_F5));
    connect( action, SIGNAL(triggered()), SLOT(setFullscreenEditor()) );
    
    editor->addAction(action);
    
    
    static_cast<QVBoxLayout*>(layout())->setStretchFactor(editor, 5);

    //Toolbox
    QWidget *propertiesWidget = new QWidget(toolbox);
    ui_properties->setupUi(propertiesWidget);
    connect(ui_properties->editableDueDate, SIGNAL(dateChanged(KDateTime, bool)), this, SLOT(setDueDate(KDateTime, bool)));
    connect(ui_properties->editableEventDate, SIGNAL(dateChanged(KDateTime)), this, SLOT(setEventDate(KDateTime)));
    toolbox->addWidget(propertiesWidget, i18n("Properties"));

    QWidget *attachmentsWidget = new QWidget(toolbox);
    QVBoxLayout *attachmentsLayout = new QVBoxLayout;
    connect(m_attachmentsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(viewAttachment(QListWidgetItem*)));
    attachmentsLayout->addWidget(m_attachmentsList);
    attachmentsWidget->setLayout(attachmentsLayout);
    toolbox->addWidget(attachmentsWidget, i18n("Attachments"));
    
    setEnabled(false);

    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(autosave()));
    QTimer::singleShot(0, this, SLOT(restoreState())); //delayed so we can check if the toolbar is visible or not
}

ItemViewer::~ItemViewer()
{
    KConfigGroup config(KGlobal::config(), "ItemViewer");
    config.writeEntry("activeToolbox", toolbox->currentIndex());
    config.writeEntry("toolbarHidden", actionCollection()->action( "hide_toolbar" )->isChecked()); //The widget is already hidden, but the action still has the correct state

    saveItem();
    if (m_currentItem) {
        disconnect(m_currentItem, 0, this, 0);
        m_currentItem->deleteLater();
        m_currentItem = 0;
    }

    delete ui_properties;
    ui_properties = 0;
}

void ItemViewer::restoreState()
{
    KConfigGroup config(KGlobal::config(), "ItemViewer");
    int activeToolbox= config.readEntry( "activeToolbox", 0);
    toolbox->activateWidget(activeToolbox);
    bool toolbarHidden = config.readEntry( "toolbarHidden", false);
    if (toolbarHidden) {
        editor->toggleToolbarVisibility();
    }
}


void ItemViewer::setFullscreenEditor()
{
    if (!m_currentItem) {
        return;
    }
    if (editor->windowState() & Qt::WindowFullScreen) {
        editor->setParent(this);
        static_cast<QVBoxLayout*>(layout())->insertWidget(1, editor, 5);
    } else {
        editor->setParent(0);
    }
    editor->setWindowState(editor->windowState() ^ Qt::WindowFullScreen);
    editor->show();
}


void ItemViewer::autosave()
{
    //kDebug();
    saveItem();
}


void ItemViewer::saveItem()
{
    //kDebug();
    if (!m_currentItem) {
        kDebug() << "no item set";
        return;
    }
    bool modified = false;
    
    if (editor->editor()->document()->isModified()) {
        bool isRichText = KPIMTextEdit::TextUtils::containsFormatting( editor->editor()->document() );
        if (isRichText) {
            m_currentItem->setText(editor->editor()->toHtml(), true);
        } else {
            m_currentItem->setText(editor->editor()->toPlainText(), false);
        }
        editor->editor()->document()->setModified(false);
        modified = true;
    }

    if (title->lineEdit().isModified()) {
        m_currentItem->setTitle(title->text());
        title->lineEdit().setModified(false);
        modified = true;
    }
    if (modified) {
        kDebug() << "save item";
        m_currentItem->saveItem();
    }
}
/*
 TODO this widget never has the focus, check the focus of the editor->editor() instead
void ItemViewer::focusInEvent(QFocusEvent* event)
{
    connect(title, SIGNAL(editingFinished()), this, SLOT(saveItem()));
    QWidget::focusInEvent(event);
}


void ItemViewer::focusOutEvent(QFocusEvent* event)
{
    kDebug();
    //So the item is saved only once, if we loose focus after editing the title
    disconnect(title, SIGNAL(editingFinished()), this, SLOT(saveItem()));
    saveItem();
    QWidget::focusOutEvent(event);
}*/

void ItemViewer::clearView()
{
    m_autosaveTimer->stop();
    editor->editor()->clear();
    //Reset action from last text (i.e. if bold text was enabled)
    /*foreach (QAction *action, guiWindow->actionCollection()->actions()) {
        kDebug() << "reset: " << action->text();
        action->setChecked(false);
    }*/
    //Reset formatting actions from last text (i.e. if bold text was enabled)
    //maybe it would be cleaner to set the default values in the QDocument (i.e. setDefaultFont())
    editor->editor()->switchToPlainText();
    editor->editor()->enableRichTextMode();

    title->clear();
    title->lineEdit().setModified(false);
    editor->editor()->document()->setModified(false);
    //we're not editing anymore, so clear focus. Otherwise a conflict will be detected
    editor->editor()->clearFocus();
    title->lineEdit().clearFocus();

    if (m_currentItem) {
        disconnect(m_currentItem, 0, this, 0);
        m_currentItem->deleteLater();
        m_currentItem = 0;
    }
}

void ItemViewer::setItem(const Nepomuk2::Resource &res)
{
    Akonadi::Item item = PimItemUtils::getItemFromResource(res);    
    if (!item.isValid()) {
        kWarning() << "invalid item passed" << res.uri().toString();
        return;
    }
    setItem(item);
}

void ItemViewer::setItem(const KUrl &url)
{
    Akonadi::Item item = Akonadi::Item::fromUrl(url);
    if (!item.isValid()) {
        kWarning() << "invalid item passed" << url;
        return;
    }
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item, this);
    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(itemsReceived(Akonadi::Item::List)));
}

void ItemViewer::itemsReceived( const Akonadi::Item::List &list )
{
    if (list.isEmpty()) {
        kWarning() << "no items retrieved";
        return;
    }
    Q_ASSERT(list.size() == 1);
    setItem(list.first());
}


void ItemViewer::setItem(const Akonadi::Item& item)
{
    kDebug();
    //reset pending signals from last item
    disconnect(&title->lineEdit(), SIGNAL(editingFinished()), this, SLOT(saveItem()));
    emit itemChanged();
    saveItem();

    clearView();

    connect(&title->lineEdit(), SIGNAL(editingFinished()), this, SLOT(saveItem())); //update title also in listview as soon as it is set

    m_currentItem = PimItemUtils::getItem(item, this);
    if (!m_currentItem) {
        setEnabled(false);
        kWarning() << "invalid item";
        return;
    }
    setEnabled(true);

    Q_ASSERT(m_currentItem);
    connect(m_currentItem, SIGNAL(payloadFetchComplete()), this, SLOT(updateContent()));
    connect(m_currentItem, SIGNAL(changed(AbstractPimItem::ChangedParts)), this, SLOT(updateContent(AbstractPimItem::ChangedParts)));
    connect(m_currentItem, SIGNAL(removed()), this, SLOT(itemRemoved()));

    m_currentItem->fetchPayload(); //in case the payload is not yet fetched (model does not automatically fetch
    m_currentItem->enableMonitor();

}

void ItemViewer::updateContent(AbstractPimItem::ChangedParts parts)
{
    kDebug();
    Q_ASSERT(ui_properties);
    /*
     * TODO check for changed content, if there is changed content we have a conflict,
     * and the user should be allowed to save the current content
     */
    if ((editor->editor()->hasFocus() && (parts & AbstractPimItem::Text)) || (title->lineEdit().hasFocus() && (parts & AbstractPimItem::Title))) { //were currently editing, and the item has changed in the background, so there is probably a conflict
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

    Q_ASSERT(m_currentItem);
    //kDebug() << m_currentItem->hasValidPayload() << m_currentItem->getText();

    if (parts & AbstractPimItem::Text) {
        kDebug() << "text changed";
        editor->editor()->setTextOrHtml(m_currentItem->getText());
        editor->editor()->document()->setModified(false);
    }

    if (parts & AbstractPimItem::Title) {
        kDebug() << "title changed";
        title->setText(m_currentItem->getTitle());
        title->lineEdit().setModified(false);
    }


    //Properties
    ui_properties->creationTime->setText(DateStringBuilder::getFullDate(m_currentItem->getCreationDate()));
    ui_properties->lastModifiedTime->setText(DateStringBuilder::getFullDate(m_currentItem->getLastModifiedDate()));

    if (m_currentItem->itemType() & AbstractPimItem::Todo) {
        IncidenceItem *inc = static_cast<IncidenceItem*>(m_currentItem);
        //Due Date
        bool hasDue = inc->hasDueDate();

        ui_properties->editableDueDate->show();
        ui_properties->lb_dueDate->show();
        if (hasDue) {
            ui_properties->editableDueDate->setDate(inc->getDueDate());
        } else {
            ui_properties->editableDueDate->clear();
        }
        ui_properties->editableDueDate->enable(hasDue); //set checked status

    } else { //not a todo
        ui_properties->editableDueDate->hide();
        ui_properties->lb_dueDate->hide();
    }

    if (m_currentItem->itemType() & AbstractPimItem::Event) {
        IncidenceItem *inc = static_cast<IncidenceItem*>(m_currentItem);
        //Event Start
        ui_properties->editableEventDate->show();
        ui_properties->lb_eventDate->show();
        ui_properties->editableEventDate->setDate(inc->getEventStart());
    } else { //not a todo
        ui_properties->lb_eventDate->hide();
        ui_properties->editableEventDate->hide();
    }
    
    m_attachmentsList->clear();
    const KCalCore::Attachment::List attachments = m_currentItem->getAttachments();
    KCalCore::Attachment::List::ConstIterator it;
    KCalCore::Attachment::List::ConstIterator end( attachments.constEnd() );
    for ( it = attachments.constBegin(); it != end; ++it ) {
        QListWidgetItem* attachmentItem = new QListWidgetItem(m_attachmentsList);
        attachmentItem->setText((*it)->label());
        attachmentItem->setData(Qt::UserRole, (*it)->uri());
        m_attachmentsList->addItem(attachmentItem);
    }
    
    //Set Focus for new items to title bar
    //TODO If it should be possible to have notes without titles this is not a good idea, but otherwise it works very well
    //kDebug() << m_currentItem->getLastModifiedDate() << KDateTime(QDateTime::currentDateTime().addSecs(-1));
    //FIXME this is broken if we going trough the list with the keyboard, looking at each note, because the focus doesn't stay on the list but goes to the linedit instead
    if (m_currentItem->getTitle().isEmpty() && m_currentItem->getCreationDate() >= KDateTime(QDateTime::currentDateTime().addSecs(-30))) {
            title->edit();
            title->lineEdit().setFocus();
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

void ItemViewer::setEventDate(KDateTime dateTime)
{
    if (!m_currentItem) {
        return;
    }
    Q_ASSERT(m_currentItem);
    if (m_currentItem->itemType() & AbstractPimItem::Event) {
        IncidenceItem *inc = static_cast<IncidenceItem*>(m_currentItem);
        inc->setEventStart(dateTime);
        m_currentItem->saveItem();
    }
}


void ItemViewer::setDueDate(KDateTime dateTime, bool enabled)
{
    if (!m_currentItem) {
        return;
    }
    Q_ASSERT(m_currentItem);
    if (m_currentItem->itemType() & AbstractPimItem::Todo) {
        IncidenceItem *inc = static_cast<IncidenceItem*>(m_currentItem);
        inc->setDueDate(dateTime, enabled);
        m_currentItem->saveItem();
    }
}

void ItemViewer::itemRemoved()
{
    clearView();
    setEnabled(false);
}

// code taken from kdepim/calendarsupport/attachmenthandler.cpp
static KTemporaryFile *s_tempFile = 0;
static KUrl tempFileForAttachment( const KCalCore::Attachment::Ptr &attachment )
{
    KUrl url;

    s_tempFile = new KTemporaryFile();
    s_tempFile->setAutoRemove( false );
    QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();
    if ( !patterns.empty() ) {
        s_tempFile->setSuffix( QString( patterns.first() ).remove( '*' ) );
    }
    s_tempFile->open();
    s_tempFile->setPermissions( QFile::ReadUser );
    s_tempFile->write( QByteArray::fromBase64( attachment->data() ) );
    s_tempFile->close();
    QFile tf( s_tempFile->fileName() );
    if ( tf.size() != attachment->size() ) {
        //whoops. failed to write the entire attachment. return an invalid URL.
        delete s_tempFile;
        s_tempFile = 0;
        return url;
    }

    url.setPath( s_tempFile->fileName() );
    return url;
}

void ItemViewer::viewAttachment(QListWidgetItem* item)
{
    int index = item->listWidget()->currentRow();
    const KCalCore::Attachment::List attachments = m_currentItem->getAttachments();
    KCalCore::Attachment::Ptr attachment = attachments.at(index);
    if (attachment->isUri()) {
        const KUrl url = KUrl(attachment->uri());
        new KRun(url, this);
    } else {
        // code taken from kdepim/calendarsupport/attachmenthandler.cpp
        // put the attachment in a temporary file and launch it
        KUrl tempUrl = tempFileForAttachment( attachment );
        if ( tempUrl.isValid() ) {
            KRun::runUrl( tempUrl, attachment->mimeType(), 0, true );
        } else {
            KMessageBox::error(
                this,
                i18n( "Cannot open the attachment." ) );
        }
        delete s_tempFile;
        s_tempFile = 0;
    }
}
