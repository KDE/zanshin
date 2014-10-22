/*
 * This file is part of libkldap.
 *
 * Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 * Author: Steffen Hansen <hansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ldapsearchdialog.h"

#include "ldapclient.h"
#include "ldapclientsearchconfig.h"
#include <QtCore/QPair>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QClipboard>

#include <akonadi/collection.h>
#include <akonadi/itemcreatejob.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcmultidialog.h>
#include <kdialogbuttonbox.h>
#include <kldap/ldapobject.h>
#include <kldap/ldapserver.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <KPushButton>

#include <KPIMUtils/ProgressIndicatorLabel>

using namespace KLDAP;

static QString asUtf8( const QByteArray &val )
{
    if ( val.isEmpty() ) {
        return QString();
    }

    const char *data = val.data();

    //QString::fromUtf8() bug workaround
    if ( data[ val.size() - 1 ] == '\0' ) {
        return QString::fromUtf8( data, val.size() - 1 );
    } else {
        return QString::fromUtf8( data, val.size() );
    }
}

static QString join( const KLDAP::LdapAttrValue &lst, const QString &sep )
{
    QString res;
    bool alredy = false;
    KLDAP::LdapAttrValue::ConstIterator end(lst.constEnd());
    for ( KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin(); it != end; ++it ) {
        if ( alredy ) {
            res += sep;
        }

        alredy = true;
        res += asUtf8( *it );
    }

    return res;
}

static QMap<QString, QString>& adrbookattr2ldap()
{
    static QMap<QString, QString> keys;

    if ( keys.isEmpty() ) {
        keys[ i18nc( "@item LDAP search key", "Title" ) ] = QLatin1String("title");
        keys[ i18n( "Full Name" ) ] = QLatin1String("cn");
        keys[ i18nc( "@item LDAP search key", "Email" ) ] = QLatin1String("mail");
        keys[ i18n( "Home Number" ) ] = QLatin1String("homePhone");
        keys[ i18n( "Work Number" ) ] = QLatin1String("telephoneNumber");
        keys[ i18n( "Mobile Number" ) ] = QLatin1String("mobile");
        keys[ i18n( "Fax Number" ) ] = QLatin1String("facsimileTelephoneNumber");
        keys[ i18n( "Pager" ) ] = QLatin1String("pager");
        keys[ i18n( "Street" ) ] = QLatin1String("street");
        keys[ i18nc( "@item LDAP search key", "State" ) ] = QLatin1String("st");
        keys[ i18n( "Country" ) ] = QLatin1String("co");
        keys[ i18n( "City" ) ] = QLatin1String("l"); //krazy:exclude=doublequote_chars
        keys[ i18n( "Organization" ) ] = QLatin1String("o"); //krazy:exclude=doublequote_chars
        keys[ i18n( "Company" ) ] = QLatin1String("Company");
        keys[ i18n( "Department" ) ] = QLatin1String("department");
        keys[ i18n( "Zip Code" ) ] = QLatin1String("postalCode");
        keys[ i18n( "Postal Address" ) ] = QLatin1String("postalAddress");
        keys[ i18n( "Description" ) ] = QLatin1String("description");
        keys[ i18n( "User ID" ) ] = QLatin1String("uid");
    }

    return keys;
}

static QString makeFilter( const QString &query, const QString &attr, bool startsWith )
{
    /* The reasoning behind this filter is:
   * If it's a person, or a distlist, show it, even if it doesn't have an email address.
   * If it's not a person, or a distlist, only show it if it has an email attribute.
   * This allows both resource accounts with an email address which are not a person and
   * person entries without an email address to show up, while still not showing things
   * like structural entries in the ldap tree. */
    QString result( QLatin1String("&(|(objectclass=person)(objectclass=groupofnames)(mail=*))(") );
    if ( query.isEmpty() ) {
        // Return a filter that matches everything
        return result + QLatin1String("|(cn=*)(sn=*)") + QLatin1Char(')');
    }

    if ( attr == i18nc( "Search attribute: Name of contact", "Name" ) ) {
        result += startsWith ? QLatin1String("|(cn=%1*)(sn=%2*)") : QLatin1String("|(cn=*%1*)(sn=*%2*)");
        result = result.arg( query ).arg( query );
    } else {
        result += startsWith ? QLatin1String("%1=%2*") : QLatin1String("%1=*%2*");
        if ( attr == i18nc( "Search attribute: Email of the contact", "Email" ) ) {
            result = result.arg( QLatin1String("mail") ).arg( query );
        } else if ( attr == i18n( "Home Number" ) ) {
            result = result.arg( QLatin1String("homePhone") ).arg( query );
        } else if ( attr == i18n( "Work Number" ) ) {
            result = result.arg( QLatin1String("telephoneNumber") ).arg( query );
        } else {
            // Error?
            result.clear();
            return result;
        }
    }
    result += QLatin1Char(')');
    return result;
}

static KABC::Addressee convertLdapAttributesToAddressee( const KLDAP::LdapAttrMap &attrs )
{
    KABC::Addressee addr;

    // name
    if ( !attrs.value( QLatin1String("cn") ).isEmpty() ) {
        addr.setNameFromString( asUtf8( attrs[QLatin1String("cn")].first() ) );
    }

    // email
    KLDAP::LdapAttrValue lst = attrs[QLatin1String("mail")];
    KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin();
    bool pref = true;
    while ( it != lst.constEnd() ) {
        addr.insertEmail( asUtf8( *it ), pref );
        pref = false;
        ++it;
    }

    if ( !attrs.value( QLatin1String("o") ).isEmpty() ) {
        addr.setOrganization( asUtf8( attrs[ QLatin1String("o") ].first() ) );
    }
    if ( addr.organization().isEmpty() && !attrs.value( QLatin1String("Company") ).isEmpty() ) {
        addr.setOrganization( asUtf8( attrs[ QLatin1String("Company") ].first() ) );
    }

    // Address
    KABC::Address workAddr( KABC::Address::Work );

    if ( !attrs.value( QLatin1String("department") ).isEmpty() ) {
        addr.setDepartment( asUtf8( attrs[ QLatin1String("department") ].first() ) );
    }

    if ( !workAddr.isEmpty() ) {
        addr.insertAddress( workAddr );
    }

    // phone
    if ( !attrs.value( QLatin1String("homePhone") ).isEmpty() ) {
        KABC::PhoneNumber homeNr = asUtf8( attrs[  QLatin1String("homePhone") ].first() );
        homeNr.setType( KABC::PhoneNumber::Home );
        addr.insertPhoneNumber( homeNr );
    }

    if ( !attrs.value( QLatin1String("telephoneNumber") ).isEmpty() ) {
        KABC::PhoneNumber workNr = asUtf8( attrs[  QLatin1String("telephoneNumber") ].first() );
        workNr.setType( KABC::PhoneNumber::Work );
        addr.insertPhoneNumber( workNr );
    }

    if ( !attrs.value( QLatin1String("facsimileTelephoneNumber") ).isEmpty() ) {
        KABC::PhoneNumber faxNr = asUtf8( attrs[  QLatin1String("facsimileTelephoneNumber") ].first() );
        faxNr.setType( KABC::PhoneNumber::Fax );
        addr.insertPhoneNumber( faxNr );
    }

    if ( !attrs.value( QLatin1String("mobile") ).isEmpty() ) {
        KABC::PhoneNumber cellNr = asUtf8( attrs[  QLatin1String("mobile") ].first() );
        cellNr.setType( KABC::PhoneNumber::Cell );
        addr.insertPhoneNumber( cellNr );
    }

    if ( !attrs.value( QLatin1String("pager") ).isEmpty() ) {
        KABC::PhoneNumber pagerNr = asUtf8( attrs[  QLatin1String("pager") ].first() );
        pagerNr.setType( KABC::PhoneNumber::Pager );
        addr.insertPhoneNumber( pagerNr );
    }

    return addr;
}

class ContactListModel : public QAbstractTableModel
{
public:
    enum Role {
        ServerRole = Qt::UserRole + 1
    };

    ContactListModel( QObject *parent )
        : QAbstractTableModel( parent )
    {
    }

    void addContact( const KLDAP::LdapAttrMap &contact, const QString &server )
    {
        mContactList.append( contact );
        mServerList.append( server );
        reset();
    }

    QPair<KLDAP::LdapAttrMap, QString> contact( const QModelIndex &index ) const
    {
        if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
            return qMakePair( KLDAP::LdapAttrMap(), QString() );
        }

        return qMakePair( mContactList.at( index.row() ), mServerList.at( index.row() ) );
    }

    QString email( const QModelIndex &index ) const
    {
        if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
            return QString();
        }

        return asUtf8( mContactList.at( index.row() ).value( QLatin1String("mail") ).first() ).trimmed();
    }

    QString fullName( const QModelIndex &index ) const
    {
        if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
            return QString();
        }

        return asUtf8( mContactList.at( index.row() ).value( QLatin1String("cn") ).first() ).trimmed();
    }

    void clear()
    {
        mContactList.clear();
        mServerList.clear();
        reset();
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
        if ( !parent.isValid() ) {
            return mContactList.count();
        } else {
            return 0;
        }
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
        if ( !parent.isValid() ) {
            return 18;
        } else {
            return 0;
        }
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const
    {
        if ( orientation == Qt::Vertical || role != Qt::DisplayRole || section < 0 || section > 17 ) {
            return QVariant();
        }

        switch ( section ) {
        case 0:
            return i18n( "Full Name" );
            break;
        case 1:
            return i18nc( "@title:column Column containing email addresses", "Email" );
            break;
        case 2:
            return i18n( "Home Number" );
            break;
        case 3:
            return i18n( "Work Number" );
            break;
        case 4:
            return i18n( "Mobile Number" );
            break;
        case 5:
            return i18n( "Fax Number" );
            break;
        case 6:
            return i18n( "Company" );
            break;
        case 7:
            return i18n( "Organization" );
            break;
        case 8:
            return i18n( "Street" );
            break;
        case 9:
            return i18nc( "@title:column Column containing the residential state of the address",
                          "State" );
            break;
        case 10:
            return i18n( "Country" );
            break;
        case 11:
            return i18n( "Zip Code" );
            break;
        case 12:
            return i18n( "Postal Address" );
            break;
        case 13:
            return i18n( "City" );
            break;
        case 14:
            return i18n( "Department" );
            break;
        case 15:
            return i18n( "Description" );
            break;
        case 16:
            return i18n( "User ID" );
            break;
        case 17:
            return i18nc( "@title:column Column containing title of the person", "Title" );
            break;
        default:
            return QVariant();
            break;
        };

        return QVariant();
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
        if ( !index.isValid() ) {
            return QVariant();
        }

        if ( index.row() < 0 || index.row() >= mContactList.count() ||
             index.column() < 0 || index.column() > 17 ) {
            return QVariant();
        }

        if ( role == ServerRole ) {
            return mServerList.at( index.row() );
        }

        if ( (role != Qt::DisplayRole) &&  (role != Qt::ToolTipRole) ) {
            return QVariant();
        }

        const KLDAP::LdapAttrMap map = mContactList.at( index.row() );

        switch ( index.column() ) {
        case 0:
            return join( map.value( QLatin1String("cn") ), QLatin1String(", ") );
            break;
        case 1:
            return join( map.value( QLatin1String("mail") ), QLatin1String(", ") );
            break;
        case 2:
            return join( map.value( QLatin1String("homePhone") ), QLatin1String(", ") );
            break;
        case 3:
            return join( map.value( QLatin1String("telephoneNumber") ), QLatin1String(", ") );
            break;
        case 4:
            return join( map.value( QLatin1String("mobile") ), QLatin1String(", ") );
            break;
        case 5:
            return join( map.value( QLatin1String("facsimileTelephoneNumber") ), QLatin1String(", ") );
            break;
        case 6:
            return join( map.value( QLatin1String("Company") ), QLatin1String(", ") );
            break;
        case 7:
            return join( map.value( QLatin1String("o") ), QLatin1String(", " ));
            break;
        case 8:
            return join( map.value( QLatin1String("street") ), QLatin1String(", ") );
            break;
        case 9:
            return join( map.value( QLatin1String("st") ), QLatin1String(", " ));
            break;
        case 10:
            return join( map.value( QLatin1String("co") ), QLatin1String(", ") );
            break;
        case 11:
            return join( map.value( QLatin1String("postalCode") ), QLatin1String(", ") );
            break;
        case 12:
            return join( map.value( QLatin1String("postalAddress") ), QLatin1String(", ") );
            break;
        case 13:
            return join( map.value( QLatin1String("l") ), QLatin1String(", ") );
            break;
        case 14:
            return join( map.value( QLatin1String("department") ),QLatin1String( ", " ));
            break;
        case 15:
            return join( map.value( QLatin1String("description") ), QLatin1String(", ") );
            break;
        case 16:
            return join( map.value( QLatin1String("uid") ), QLatin1String(", ") );
            break;
        case 17:
            return join( map.value( QLatin1String("title") ), QLatin1String(", ") );
            break;
        default:
            return QVariant();
            break;
        }

        return QVariant();
    }

private:
    QList<KLDAP::LdapAttrMap> mContactList;
    QStringList mServerList;
};

class LdapSearchDialog::Private
{
public:
    Private( LdapSearchDialog *qq )
        : q( qq ),
          mNumHosts( 0 ),
          mIsConfigured( false ),
          mModel( 0 )
    {
    }

    QList< QPair<KLDAP::LdapAttrMap, QString> > selectedItems()
    {
        QList< QPair<KLDAP::LdapAttrMap, QString> > contacts;

        const QModelIndexList selected = mResultView->selectionModel()->selectedRows();
        for ( int i = 0; i < selected.count(); ++i ) {
            contacts.append( mModel->contact( sortproxy->mapToSource(selected.at( i )) ) );
        }

        return contacts;
    }


    void saveSettings();
    void restoreSettings();
    void cancelQuery();

    void slotAddResult( const KLDAP::LdapClient&, const KLDAP::LdapObject& );
    void slotSetScope( bool );
    void slotStartSearch();
    void slotStopSearch();
    void slotSearchDone();
    void slotError( const QString& );
    void slotSelectAll();
    void slotUnselectAll();
    void slotSelectionChanged();

    LdapSearchDialog *q;
    KGuiItem startSearchGuiItem;
    KGuiItem stopSearchGuiItem;
    int mNumHosts;
    QList<KLDAP::LdapClient*> mLdapClientList;
    bool mIsConfigured;
    KABC::Addressee::List mSelectedContacts;

    KComboBox *mFilterCombo;
    KComboBox *mSearchType;
    KLineEdit *mSearchEdit;

    QCheckBox *mRecursiveCheckbox;
    QTableView *mResultView;
    KPushButton *mSearchButton;
    ContactListModel *mModel;
    KPIMUtils::ProgressIndicatorLabel *progressIndication;
    QSortFilterProxyModel *sortproxy;
};

LdapSearchDialog::LdapSearchDialog( QWidget *parent )
    : KDialog( parent ), d( new Private( this ) )
{
    setCaption( i18n( "Import Contacts from LDAP" ) );
    setButtons( /*Help |*/ User1 | User2 | Cancel );
    setDefaultButton( User1 );
    setModal( false );
    showButtonSeparator( true );
    setButtonGuiItem( KDialog::Cancel, KStandardGuiItem::close() );
    QFrame *page = new QFrame( this );
    setMainWidget( page );
    QVBoxLayout *topLayout = new QVBoxLayout( page );
    topLayout->setSpacing( spacingHint() );
    topLayout->setMargin( marginHint() );

    QGroupBox *groupBox = new QGroupBox( i18n( "Search for Addresses in Directory" ),
                                         page );
    QGridLayout *boxLayout = new QGridLayout();
    groupBox->setLayout( boxLayout );
    boxLayout->setSpacing( spacingHint() );
    boxLayout->setColumnStretch( 1, 1 );

    QLabel *label = new QLabel( i18n( "Search for:" ), groupBox );
    boxLayout->addWidget( label, 0, 0 );

    d->mSearchEdit = new KLineEdit( groupBox );
    d->mSearchEdit->setClearButtonShown(true);
    boxLayout->addWidget( d->mSearchEdit, 0, 1 );
    label->setBuddy( d->mSearchEdit );

    label = new QLabel( i18nc( "In LDAP attribute", "in" ), groupBox );
    boxLayout->addWidget( label, 0, 2 );

    d->mFilterCombo = new KComboBox( groupBox );
    d->mFilterCombo->addItem( i18nc( "@item:inlistbox Name of the contact", "Name" ) );
    d->mFilterCombo->addItem( i18nc( "@item:inlistbox email address of the contact", "Email" ) );
    d->mFilterCombo->addItem( i18nc( "@item:inlistbox", "Home Number" ) );
    d->mFilterCombo->addItem( i18nc( "@item:inlistbox", "Work Number" ) );
    boxLayout->addWidget( d->mFilterCombo, 0, 3 );
    d->startSearchGuiItem = KGuiItem(  i18nc( "@action:button Start searching", "&Search" ), QLatin1String("edit-find") );
    d->stopSearchGuiItem = KStandardGuiItem::stop();

    QSize buttonSize;
    d->mSearchButton = new KPushButton( groupBox );
    d->mSearchButton->setGuiItem(d->startSearchGuiItem);

    buttonSize = d->mSearchButton->sizeHint();
    if ( buttonSize.width() < d->mSearchButton->sizeHint().width() ) {
        buttonSize = d->mSearchButton->sizeHint();
    }
    d->mSearchButton->setFixedWidth( buttonSize.width() );

    d->mSearchButton->setDefault( true );
    boxLayout->addWidget( d->mSearchButton, 0, 4 );

    d->mRecursiveCheckbox = new QCheckBox( i18n( "Recursive search" ), groupBox );
    d->mRecursiveCheckbox->setChecked( true );
    boxLayout->addWidget( d->mRecursiveCheckbox, 1, 0, 1, 5 );

    d->mSearchType = new KComboBox( groupBox );
    d->mSearchType->addItem( i18n( "Contains" ) );
    d->mSearchType->addItem( i18n( "Starts With" ) );
    boxLayout->addWidget( d->mSearchType, 1, 3, 1, 2 );

    topLayout->addWidget( groupBox );

    d->mResultView = new QTableView( page );
    d->mResultView->setSelectionMode( QTableView::MultiSelection );
    d->mResultView->setSelectionBehavior( QTableView::SelectRows );
    d->mModel = new ContactListModel( d->mResultView );

    d->sortproxy = new QSortFilterProxyModel( this );
    d->sortproxy->setSourceModel( d->mModel );

    d->mResultView->setModel( d->sortproxy );
    d->mResultView->verticalHeader()->hide();
    d->mResultView->setSortingEnabled(true);
    d->mResultView->horizontalHeader()->setSortIndicatorShown(true);
    connect( d->mResultView, SIGNAL(clicked(QModelIndex)),
             SLOT(slotSelectionChanged()) );
    topLayout->addWidget( d->mResultView );

    d->mResultView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->mResultView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));


    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    topLayout->addLayout(buttonLayout);

    d->progressIndication = new KPIMUtils::ProgressIndicatorLabel(i18n("Searching..."));
    buttonLayout->addWidget(d->progressIndication);

    KDialogButtonBox *buttons = new KDialogButtonBox( page, Qt::Horizontal );
    buttons->addButton( i18n( "Select All" ),
                        QDialogButtonBox::ActionRole, this, SLOT(slotSelectAll()) );
    buttons->addButton( i18n( "Unselect All" ),
                        QDialogButtonBox::ActionRole, this, SLOT(slotUnselectAll()) );

    buttonLayout->addWidget( buttons );



    setButtonText( User1, i18n( "Add Selected" ) );
    setButtonText( User2, i18n( "Configure LDAP Servers..." ) );

    connect( d->mRecursiveCheckbox, SIGNAL(toggled(bool)),
             this, SLOT(slotSetScope(bool)) );
    connect( d->mSearchButton, SIGNAL(clicked()),
             this, SLOT(slotStartSearch()) );

    setTabOrder( d->mSearchEdit, d->mFilterCombo );
    setTabOrder( d->mFilterCombo, d->mSearchButton );
    d->mSearchEdit->setFocus();

    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
    connect( this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()) );
    connect( this, SIGNAL(cancelClicked()), this, SLOT(slotCancelClicked()));
    d->slotSelectionChanged();
    d->restoreSettings();
}

LdapSearchDialog::~LdapSearchDialog()
{
    d->saveSettings();
    delete d;
}

void LdapSearchDialog::setSearchText( const QString &text )
{
    d->mSearchEdit->setText( text );
}

KABC::Addressee::List LdapSearchDialog::selectedContacts() const
{
    return d->mSelectedContacts;
}

void LdapSearchDialog::slotCustomContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = d->mResultView->indexAt(pos);
    if (index.isValid()) {
        QMenu menu;
        QAction *act = menu.addAction(i18n("Copy"));
        if (menu.exec(QCursor::pos()) == act) {
            QClipboard *cb = QApplication::clipboard();
            cb->setText(index.data().toString(), QClipboard::Clipboard);
        }
    }
}


void LdapSearchDialog::Private::slotSelectionChanged()
{
    q->enableButton( KDialog::User1, mResultView->selectionModel()->hasSelection() );
}

void LdapSearchDialog::Private::restoreSettings()
{
    // Create one KLDAP::LdapClient per selected server and configure it.

    // First clean the list to make sure it is empty at
    // the beginning of the process
    qDeleteAll( mLdapClientList ) ;
    mLdapClientList.clear();

    KConfig *config = KLDAP::LdapClientSearchConfig::config();

    KConfigGroup searchGroup( config, "LDAPSearch" );
    mSearchType->setCurrentIndex( searchGroup.readEntry( "SearchType", 0 ) );

    // then read the config file and register all selected
    // server in the list
    KConfigGroup group( config, "LDAP" );
    mNumHosts = group.readEntry( "NumSelectedHosts", 0 );
    if ( !mNumHosts ) {
        mIsConfigured = false;
    } else {
        mIsConfigured = true;
        KLDAP::LdapClientSearchConfig *clientSearchConfig = new KLDAP::LdapClientSearchConfig;
        for ( int j = 0; j < mNumHosts; ++j ) {
            KLDAP::LdapServer ldapServer;
            KLDAP::LdapClient *ldapClient = new KLDAP::LdapClient( 0, q );
            clientSearchConfig->readConfig( ldapServer, group, j, true );
            ldapClient->setServer( ldapServer );
            QStringList attrs;

            QMap<QString, QString>::ConstIterator end(adrbookattr2ldap().constEnd());
            for ( QMap<QString, QString>::ConstIterator it = adrbookattr2ldap().constBegin();
                  it != end; ++it ) {
                attrs << *it;
            }

            ldapClient->setAttributes( attrs );

            q->connect( ldapClient, SIGNAL(result(KLDAP::LdapClient,KLDAP::LdapObject)),
                        q, SLOT(slotAddResult(KLDAP::LdapClient,KLDAP::LdapObject)) );
            q->connect( ldapClient, SIGNAL(done()),
                        q, SLOT(slotSearchDone()) );
            q->connect( ldapClient, SIGNAL(error(QString)),
                        q, SLOT(slotError(QString)) );

            mLdapClientList.append( ldapClient );
        }
        delete clientSearchConfig;

        mModel->clear();
    }
    KConfigGroup groupHeader( config, "Headers" );
    mResultView->horizontalHeader()->restoreState(groupHeader.readEntry("HeaderState",QByteArray()));

    KConfigGroup groupSize( config, "Size" );
    const QSize dialogSize = groupSize.readEntry( "Size", QSize() );
    if ( dialogSize.isValid() ) {
        q->resize( dialogSize );
    } else {
        q->resize( QSize( 600, 400 ).expandedTo( q->minimumSizeHint() ) );
    }
}

void LdapSearchDialog::Private::saveSettings()
{
    KConfig *config = KLDAP::LdapClientSearchConfig::config();
    KConfigGroup group( config, "LDAPSearch" );
    group.writeEntry( "SearchType", mSearchType->currentIndex() );

    KConfigGroup groupHeader( config, "Headers" );
    groupHeader.writeEntry( "HeaderState", mResultView->horizontalHeader()->saveState());
    groupHeader.sync();

    KConfigGroup size( config, "Size" );
    size.writeEntry( "Size", q->size());
    size.sync();

    group.sync();
}

void LdapSearchDialog::Private::cancelQuery()
{
    Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
        client->cancelQuery();
    }
}

void LdapSearchDialog::Private::slotAddResult( const KLDAP::LdapClient &client,
                                               const KLDAP::LdapObject &obj )
{
    mModel->addContact( obj.attributes(), client.server().host() );
}

void LdapSearchDialog::Private::slotSetScope( bool rec )
{
    Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
        if ( rec ) {
            client->setScope( QLatin1String("sub") );
        } else {
            client->setScope( QLatin1String("one") );
        }
    }
}

void LdapSearchDialog::Private::slotStartSearch()
{
    cancelQuery();

    if ( !mIsConfigured ) {
        KMessageBox::error( q, i18n( "You must select an LDAP server before searching." ) );
        q->slotUser2();
        return;
    }

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( Qt::WaitCursor );
#endif
    mSearchButton->setGuiItem(stopSearchGuiItem);
    progressIndication->start();

    q->disconnect( mSearchButton, SIGNAL(clicked()),
                   q, SLOT(slotStartSearch()) );
    q->connect( mSearchButton, SIGNAL(clicked()),
                q, SLOT(slotStopSearch()) );

    const bool startsWith = (mSearchType->currentIndex() == 1);

    const QString filter = makeFilter( mSearchEdit->text().trimmed(),
                                       mFilterCombo->currentText(), startsWith );

    // loop in the list and run the KLDAP::LdapClients
    mModel->clear();
    Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
        client->startQuery( filter );
    }

    saveSettings();
}

void LdapSearchDialog::Private::slotStopSearch()
{
    cancelQuery();
    slotSearchDone();
}

void LdapSearchDialog::Private::slotSearchDone()
{
    // If there are no more active clients, we are done.
    Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
        if ( client->isActive() ) {
            return;
        }
    }

    q->disconnect( mSearchButton, SIGNAL(clicked()),
                   q, SLOT(slotStopSearch()) );
    q->connect( mSearchButton, SIGNAL(clicked()),
                q, SLOT(slotStartSearch()) );

    mSearchButton->setGuiItem(startSearchGuiItem);
    progressIndication->stop();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void LdapSearchDialog::Private::slotError( const QString &error )
{
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    KMessageBox::error( q, error );
}

void LdapSearchDialog::closeEvent( QCloseEvent *e )
{
    d->slotStopSearch();
    e->accept();
}

void LdapSearchDialog::Private::slotUnselectAll()
{
    mResultView->clearSelection();
    slotSelectionChanged();
}

void LdapSearchDialog::Private::slotSelectAll()
{
    mResultView->selectAll();
    slotSelectionChanged();
}

void LdapSearchDialog::slotUser1()
{
    // Import selected items

    d->mSelectedContacts.clear();

    const QList< QPair<KLDAP::LdapAttrMap, QString> >& items = d->selectedItems();

    if ( !items.isEmpty() ) {
        const QDateTime now = QDateTime::currentDateTime();

        for ( int i = 0; i < items.count(); ++i ) {
            KABC::Addressee contact = convertLdapAttributesToAddressee( items.at( i ).first );

            // set a comment where the contact came from
            contact.setNote( i18nc( "arguments are host name, datetime",
                                    "Imported from LDAP directory %1 on %2",
                                    items.at( i ).second, KGlobal::locale()->formatDateTime( now ) ) );

            d->mSelectedContacts.append( contact );
        }
    }

    d->slotStopSearch();
    emit contactsAdded();

    accept();
}

void LdapSearchDialog::slotUser2()
{
    // Configure LDAP servers

    KCMultiDialog dialog( this );
    dialog.setCaption( i18n( "Configure the Address Book LDAP Settings" ) );
    dialog.addModule( QLatin1String("kcmldap.desktop") );

    if ( dialog.exec() ) { //krazy:exclude=crashy
        d->restoreSettings();
    }
}

void LdapSearchDialog::slotCancelClicked()
{
    d->slotStopSearch();
    reject();
}

#include "moc_ldapsearchdialog.cpp"
