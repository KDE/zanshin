/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  Copyright (c) 2001-2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#include "recentaddresses.h"
#include <kpimutils/email.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KLineEdit>
#include <KPushButton>

#include <QCoreApplication>
#include <QLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QKeyEvent>

using namespace KPIM;

RecentAddresses *s_self = 0;

void deleteGlobalRecentAddresses()
{
    delete s_self;
    s_self = 0;
}

RecentAddresses *RecentAddresses::self( KConfig *config )
{
    if ( !s_self ) {
        s_self = new RecentAddresses( config );
        qAddPostRoutine( deleteGlobalRecentAddresses );
    }
    return s_self;
}

bool RecentAddresses::exists()
{
    return s_self != 0;
}

RecentAddresses::RecentAddresses( KConfig *config )
{
    if ( !config ) {
        load( KGlobal::config().data() );
    } else {
        load( config );
    }
}

RecentAddresses::~RecentAddresses()
{
    // if you want this destructor to get called, use K_GLOBAL_STATIC
    // on s_self
}

void RecentAddresses::load( KConfig *config )
{
    QStringList addresses;
    QString name;
    QString email;

    m_addresseeList.clear();
    KConfigGroup cg( config, "General" );
    m_maxCount = cg.readEntry( "Maximum Recent Addresses", 40 );
    addresses = cg.readEntry( "Recent Addresses", QStringList() );
    QStringList::ConstIterator end( addresses.constEnd() );
    for ( QStringList::ConstIterator it = addresses.constBegin(); it != end; ++it ) {
        KABC::Addressee::parseEmailAddress( *it, name, email );
        if ( !email.isEmpty() ) {
            KABC::Addressee addr;
            addr.setNameFromString( name );
            addr.insertEmail( email, true );
            m_addresseeList.append( addr );
        }
    }

    adjustSize();
}

void RecentAddresses::save( KConfig *config )
{
    KConfigGroup cg( config, "General" );
    cg.writeEntry( "Recent Addresses", addresses() );
}

void RecentAddresses::add( const QString &entry )
{
    if ( !entry.isEmpty() && m_maxCount > 0 ) {
        const QStringList list = KPIMUtils::splitAddressList( entry );
        QStringList::const_iterator e_itEnd( list.constEnd() );
        for ( QStringList::const_iterator e_it = list.constBegin(); e_it != e_itEnd; ++e_it ) {
            KPIMUtils::EmailParseResult errorCode = KPIMUtils::isValidAddress( *e_it );
            if ( errorCode != KPIMUtils::AddressOk ) {
                continue;
            }
            QString email;
            QString fullName;
            KABC::Addressee addr;

            KABC::Addressee::parseEmailAddress( *e_it, fullName, email );

            KABC::Addressee::List::Iterator end( m_addresseeList.end() );
            for ( KABC::Addressee::List::Iterator it = m_addresseeList.begin();
                  it != end; ++it ) {
                if ( email == (*it).preferredEmail() ) {
                    //already inside, remove it here and add it later at pos==1
                    m_addresseeList.erase( it );
                    break;
                }
            }
            addr.setNameFromString( fullName );
            addr.insertEmail( email, true );
            m_addresseeList.prepend( addr );
            adjustSize();
        }
    }
}

void RecentAddresses::setMaxCount( int count )
{
    if (count != m_maxCount) {
        m_maxCount = count;
        adjustSize();
    }
}

void RecentAddresses::adjustSize()
{
    while ( m_addresseeList.count() > m_maxCount ) {
        m_addresseeList.takeLast();
    }
}

void RecentAddresses::clear()
{
    m_addresseeList.clear();
    adjustSize();
}

QStringList RecentAddresses::addresses() const
{
    QStringList addresses;
    KABC::Addressee::List::ConstIterator end = m_addresseeList.constEnd();
    for ( KABC::Addressee::List::ConstIterator it = m_addresseeList.constBegin();
          it != end; ++it ) {
        addresses.append( (*it).fullEmail() );
    }
    return addresses;
}

RecentAddressDialog::RecentAddressDialog( QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n( "Edit Recent Addresses" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *layout = new QVBoxLayout( page );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    mLineEdit = new KLineEdit(this);
    layout->addWidget(mLineEdit);

    mLineEdit->setTrapReturnKey(true);
    mLineEdit->installEventFilter(this);

    connect(mLineEdit,SIGNAL(textChanged(QString)),SLOT(slotTypedSomething(QString)));
    connect(mLineEdit,SIGNAL(returnPressed()),SLOT(slotAddItem()));


    QHBoxLayout* hboxLayout = new QHBoxLayout;

    QVBoxLayout* btnsLayout = new QVBoxLayout;
    btnsLayout->addStretch();
    mNewButton = new KPushButton(KIcon(QLatin1String("list-add")), i18n("&Add"), this);
    connect(mNewButton, SIGNAL(clicked()), SLOT(slotAddItem()));
    btnsLayout->insertWidget(0 ,mNewButton);

    mRemoveButton = new KPushButton(KIcon(QLatin1String("list-remove")), i18n("&Remove"), this);
    mRemoveButton->setEnabled(false);
    connect(mRemoveButton, SIGNAL(clicked()), SLOT(slotRemoveItem()));
    btnsLayout->insertWidget(1, mRemoveButton);


    mListView = new QListWidget(this);
    mListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mListView->setSortingEnabled(true);
    hboxLayout->addWidget(mListView);
    hboxLayout->addLayout(btnsLayout);
    layout->addLayout(hboxLayout);
    connect(mListView, SIGNAL(itemSelectionChanged()),
            SLOT(slotSelectionChanged()));
    // maybe supplied lineedit has some text already
    slotTypedSomething( mLineEdit->text() );
    readConfig();
}

RecentAddressDialog::~RecentAddressDialog()
{
    writeConfig();
}

void RecentAddressDialog::slotTypedSomething(const QString& text)
{
    if (mListView->currentItem()) {
        if (mListView->currentItem()->text() != mLineEdit->text() && !mLineEdit->text().isEmpty()) {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = mListView->signalsBlocked();
            mListView->blockSignals( true );
            QListWidgetItem *currentIndex = mListView->currentItem();
            if ( currentIndex ) {
                currentIndex->setText(text);
            }
            mListView->blockSignals( block );
        }
    }
}

void RecentAddressDialog::slotAddItem()
{
    mListView->blockSignals(true);
    mListView->insertItem(0, QString());
    mListView->blockSignals(false);
    mListView->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
    mLineEdit->setFocus();
    updateButtonState();
}

void RecentAddressDialog::slotRemoveItem()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    if (selectedItems.isEmpty())
        return;
    Q_FOREACH(QListWidgetItem *item, selectedItems) {
        delete mListView->takeItem(mListView->row(item));
    }
    updateButtonState();
}

void RecentAddressDialog::updateButtonState()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    const int numberOfElementSelected(selectedItems.count());
    mRemoveButton->setEnabled(numberOfElementSelected);
    mNewButton->setEnabled(numberOfElementSelected <= 1);
    mLineEdit->setEnabled(numberOfElementSelected <= 1);

    if (numberOfElementSelected == 1) {
        const QString text = mListView->currentItem()->text();
        if (text != mLineEdit->text())
            mLineEdit->setText(text);
    } else {
        mLineEdit->clear();
    }
}

void RecentAddressDialog::slotSelectionChanged()
{
    updateButtonState();
}

void RecentAddressDialog::setAddresses( const QStringList &addrs )
{
    mListView->clear();
    mListView->addItems( addrs );
}

QStringList RecentAddressDialog::addresses() const
{
    QStringList lst;
    const int numberOfItem(mListView->count());
    for(int i = 0; i < numberOfItem; ++i) {
        lst<<mListView->item(i)->text();
    }
    return lst;
}

bool RecentAddressDialog::eventFilter( QObject* o, QEvent* e )
{
    if (o == mLineEdit && e->type() == QEvent::KeyPress ) {
        QKeyEvent* keyEvent = (QKeyEvent*)e;
        if (keyEvent->key() == Qt::Key_Down ||
                keyEvent->key() == Qt::Key_Up) {
            return ((QObject*)mListView)->event(e);
        }
    }

    return false;
}

void RecentAddressDialog::addAddresses(KConfig *config)
{
    const int numberOfItem(mListView->count());
    for (int i = 0; i < numberOfItem; ++i) {
        KPIM::RecentAddresses::self( config )->add( mListView->item(i)->text() );
    }
}

void RecentAddressDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void RecentAddressDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}



