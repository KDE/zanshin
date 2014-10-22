/** -*- c++ -*-
 * completionordereditor.cpp
 *
 *  Copyright (c) 2004 David Faure <faure@kde.org>
 *                2010 Tobias Koenig <tokoe@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include "completionordereditor.h"
#include "completionordereditor_p.h"
#include <kdescendantsproxymodel.h>
#include "ldap/ldapclient.h"
#include "ldap/ldapclientsearch.h"
#include "ldap/ldapclientsearchconfig.h"

#include <akonadi/changerecorder.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/monitor.h>

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kldap/ldapserver.h>

#include <KDE/KConfigGroup>
#include <KDE/KHBox>
#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KPushButton>
#include <KDE/KVBox>

#include <QtDBus/QDBusConnection>
#include <QToolButton>
#include <QTreeWidget>

using namespace KPIM;

CompletionOrderEditorAdaptor::CompletionOrderEditorAdaptor( QObject *parent )
    : QDBusAbstractAdaptor( parent )
{
    setAutoRelaySignals( true );
}

class LDAPCompletionItem : public CompletionItem
{
public:
    LDAPCompletionItem( KLDAP::LdapClient *ldapClient )
        : mLdapClient( ldapClient )
    {
        mWeight = mLdapClient->completionWeight();
    }

    virtual QString label() const
    {
        return i18n( "LDAP server %1", mLdapClient->server().host() );
    }

    virtual QIcon icon() const
    {
        return KIcon( QLatin1String("view-ldap-resource") );
    }

    virtual int completionWeight() const
    {
        return mWeight;
    }

    virtual void save( CompletionOrderEditor* )
    {
        KConfig *config = KLDAP::LdapClientSearchConfig::config();
        KConfigGroup group( config, "LDAP" );
        group.writeEntry( QString::fromLatin1("SelectedCompletionWeight%1" ).arg( mLdapClient->clientNumber() ),
                          mWeight );
        group.sync();
    }

protected:
    virtual void setCompletionWeight( int weight )
    {
        mWeight = weight;
    }

private:
    KLDAP::LdapClient *mLdapClient;
    int mWeight;
};

class SimpleCompletionItem : public CompletionItem
{
public:
    SimpleCompletionItem( CompletionOrderEditor* editor, const QString& label, const QString& identifier, int weight )
        : mLabel( label ), mIdentifier( identifier )
    {
        KConfigGroup group( editor->configFile(), "CompletionWeights" );
        mWeight = group.readEntry( mIdentifier, weight );
    }

    void setIcon( const QIcon &icon )
    {
        mIcon = icon;
    }

    virtual QString label() const
    {
        return mLabel;
    }

    virtual QIcon icon() const
    {
        return mIcon;
    }

    virtual int completionWeight() const
    {
        return mWeight;
    }

    virtual void save( CompletionOrderEditor *editor )
    {
        KConfigGroup group( editor->configFile(), "CompletionWeights" );
        group.writeEntry( mIdentifier, mWeight );
    }

protected:
    virtual void setCompletionWeight( int weight )
    {
        mWeight = weight;
    }

private:
    QString mLabel;
    QString mIdentifier;
    int mWeight;
    QIcon mIcon;
};

/////////

class CompletionViewItem : public QTreeWidgetItem
{
public:
    CompletionViewItem( QTreeWidget *parent, CompletionItem* item, QTreeWidgetItem *preceding )
        : QTreeWidgetItem( parent, preceding )
    {
        setItem( item );
    }

    void setItem( CompletionItem *item )
    {
        mItem = item;
        setText( 0, mItem->label() );
        setIcon( 0, mItem->icon() );
    }

    CompletionItem* item() const
    {
        return mItem;
    }

    bool operator<( const QTreeWidgetItem &other ) const
    {
        const QTreeWidgetItem *otherItem = &other;
        const CompletionViewItem *completionItem = static_cast<const CompletionViewItem*>( otherItem );
        // item with weight 100 should be on the top -> reverse sorting
        return (mItem->completionWeight() > completionItem->item()->completionWeight());
    }

private:
    CompletionItem* mItem;
};

CompletionOrderEditor::CompletionOrderEditor( KLDAP::LdapClientSearch* ldapSearch,
                                              QWidget* parent )
    : KDialog( parent ), mConfig( QLatin1String("kpimcompletionorder") ), mLdapSearch( ldapSearch ), mDirty( false )
{
    setCaption( i18n( "Edit Completion Order" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    new CompletionOrderEditorAdaptor( this );
    QDBusConnection::sessionBus().registerObject( QLatin1String("/"), this, QDBusConnection::ExportAdaptors );

    KHBox* page = new KHBox( this );
    setMainWidget( page );
    mListView = new QTreeWidget( page );
    mListView->setColumnCount( 1 );
    mListView->setAlternatingRowColors( true );
    mListView->setIndentation( 0 );
    mListView->setAllColumnsShowFocus( true );
    mListView->setHeaderHidden ( true );
    mListView->setSortingEnabled( true );

    KVBox* upDownBox = new KVBox( page );
    mUpButton = new KPushButton( upDownBox );
    mUpButton->setAutoRepeat(true);
    mUpButton->setObjectName( QLatin1String("mUpButton") );
    mUpButton->setIcon( KIcon(QLatin1String("go-up")) );
    mUpButton->setEnabled( false ); // b/c no item is selected yet
    mUpButton->setFocusPolicy( Qt::StrongFocus );

    mDownButton = new KPushButton( upDownBox );
    mDownButton->setAutoRepeat(true);
    mDownButton->setObjectName( QLatin1String("mDownButton") );
    mDownButton->setIcon( KIcon(QLatin1String("go-down")) );
    mDownButton->setEnabled( false ); // b/c no item is selected yet
    mDownButton->setFocusPolicy( Qt::StrongFocus );

    QWidget* spacer = new QWidget( upDownBox );
    upDownBox->setStretchFactor( spacer, 100 );

    connect( mListView, SIGNAL(itemSelectionChanged()),
             SLOT(slotSelectionChanged()) );
    connect( mListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             SLOT(slotSelectionChanged()) );
    connect( mUpButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()) );
    connect( mDownButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()) );
    connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()));

    loadCompletionItems();
    readConfig();
}

CompletionOrderEditor::~CompletionOrderEditor()
{
    writeConfig();
}

void CompletionOrderEditor::readConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionOrderEditor" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void CompletionOrderEditor::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionOrderEditor" );
    group.writeEntry( "Size", size() );
    group.sync();
}


void CompletionOrderEditor::addCompletionItemForIndex( const QModelIndex &index )
{
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( !collection.isValid() )
        return;

    SimpleCompletionItem *item = new SimpleCompletionItem( this, index.data().toString(), QString::number( collection.id() ), 60 );
    item->setIcon( index.data( Qt::DecorationRole ).value<QIcon>() );

    new CompletionViewItem( mListView, item, 0 );
}

void CompletionOrderEditor::loadCompletionItems()
{
    // The first step is to gather all the data, creating CompletionItem objects
    foreach ( KLDAP::LdapClient *client, mLdapSearch->clients() ) {
        new CompletionViewItem( mListView, new LDAPCompletionItem( client ), 0 );
    }

    Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
    monitor->fetchCollection( true );
    monitor->setCollectionMonitored( Akonadi::Collection::root() );
    monitor->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
    monitor->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

    Akonadi::EntityTreeModel *model = new Akonadi::EntityTreeModel( monitor, this );
    model->setItemPopulationStrategy( Akonadi::EntityTreeModel::NoItemPopulation );

    KDescendantsProxyModel *descendantsProxy = new KDescendantsProxyModel( this );
    descendantsProxy->setDisplayAncestorData( true );
    descendantsProxy->setSourceModel( model );

    Akonadi::CollectionFilterProxyModel *mimeTypeProxy = new Akonadi::CollectionFilterProxyModel( this );
    mimeTypeProxy->addMimeTypeFilters( QStringList() << KABC::Addressee::mimeType()
                                       << KABC::ContactGroup::mimeType() );
    mimeTypeProxy->setSourceModel( descendantsProxy );
    mimeTypeProxy->setExcludeVirtualCollections( true );

    mCollectionModel = mimeTypeProxy;

    connect( mimeTypeProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SLOT(rowsInserted(QModelIndex,int,int)) );

    for ( int row = 0; row < mCollectionModel->rowCount(); ++row )
        addCompletionItemForIndex( mCollectionModel->index( row, 0 ) );

    mListView->sortItems( 0, Qt::AscendingOrder );
}

void CompletionOrderEditor::rowsInserted( const QModelIndex &parent, int start, int end )
{
    for ( int row = start; row <= end; ++row )
        addCompletionItemForIndex( mCollectionModel->index( row, 0, parent ) );

    mListView->sortItems( 0, Qt::AscendingOrder );
}

void CompletionOrderEditor::slotSelectionChanged()
{
    QTreeWidgetItem *item = mListView->currentItem();
    mDownButton->setEnabled( item && mListView->itemBelow( item ) );
    mUpButton->setEnabled( item && mListView->itemAbove( item ) );
}

static void swapItems( CompletionViewItem *one, CompletionViewItem *other )
{
    CompletionItem* oneCompletion = one->item();
    CompletionItem* otherCompletion = other->item();

    int weight = otherCompletion->completionWeight();
    otherCompletion->setCompletionWeight( oneCompletion->completionWeight() );
    oneCompletion->setCompletionWeight( weight );

    one->setItem( otherCompletion );
    other->setItem( oneCompletion );
}

void CompletionOrderEditor::slotMoveUp()
{
    CompletionViewItem *item = static_cast<CompletionViewItem *>( mListView->currentItem() );
    if ( !item ) return;
    CompletionViewItem *above = static_cast<CompletionViewItem *>( mListView->itemAbove( item ) );
    if ( !above ) return;
    swapItems( item, above );
    mListView->setCurrentItem( above, 0, QItemSelectionModel::Select | QItemSelectionModel::Current );
    mListView->sortItems( 0, Qt::AscendingOrder );
    mDirty = true;
}

void CompletionOrderEditor::slotMoveDown()
{
    CompletionViewItem *item = static_cast<CompletionViewItem *>( mListView->currentItem() );
    if ( !item ) return;
    CompletionViewItem *below = static_cast<CompletionViewItem *>( mListView->itemBelow( item ) );
    if ( !below ) return;
    swapItems( item, below );
    mListView->setCurrentItem( below );
    mListView->setCurrentItem( below, 0, QItemSelectionModel::Select | QItemSelectionModel::Current );
    mListView->sortItems( 0, Qt::AscendingOrder );
    mDirty = true;
}

void CompletionOrderEditor::slotOk()
{
    if ( mDirty ) {
        int w = 100;
        for ( int itemIndex = 0; itemIndex < mListView->topLevelItemCount(); ++itemIndex ) {
            CompletionViewItem *item =
                    static_cast<CompletionViewItem *>( mListView->topLevelItem( itemIndex ) );
            item->item()->setCompletionWeight( w );
            item->item()->save( this );
            --w;
        }
        emit completionOrderChanged();
    }
    KDialog::accept();
}

#include "moc_completionordereditor_p.cpp"
