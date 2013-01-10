/* This file is part of Zanshin Todo.

   Copyright 2013 Augusto Destrero <a.destrero@gmail.com>

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

#include "attachmentsviewer.h"

#include <kurl.h>
#include <krun.h>
#include <KTemporaryFile>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <QLayout>

AttachmentsViewer::AttachmentsViewer(QWidget* parent)
:   QWidget(parent),
    m_listWidget(new QListWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    connect(m_listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(viewAttachment(QListWidgetItem*)));
    layout->addWidget(m_listWidget);
    setLayout(layout);
}

AttachmentsViewer::~AttachmentsViewer()
{
}

void AttachmentsViewer::updateAttachments(KCalCore::Attachment::List const& attachments)
{
    m_attachmentsList = attachments;
    m_listWidget->clear();
    KCalCore::Attachment::List::ConstIterator it;
    KCalCore::Attachment::List::ConstIterator end( attachments.constEnd() );
    for ( it = attachments.constBegin(); it != end; ++it ) {
        QListWidgetItem* attachmentItem = new QListWidgetItem(m_listWidget);
        attachmentItem->setText((*it)->label());
        m_listWidget->addItem(attachmentItem);
    }
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

void AttachmentsViewer::viewAttachment(QListWidgetItem* item)
{
    int index = item->listWidget()->currentRow();
    KCalCore::Attachment::Ptr attachment = m_attachmentsList.at(index);
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
