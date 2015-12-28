/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "completionorderwidget.h"

#include <kdescendantsproxymodel.h>
#include "ldap/ldapclient.h"
#include "ldap/ldapclientsearch.h"
#include "ldap/ldapclientsearchconfig.h"

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/CollectionFilterProxyModel>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/Monitor>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <kldap/ldapserver.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolButton>
#include <QDBusConnection>

using namespace KPIM;

CompletionOrderEditorAdaptor::CompletionOrderEditorAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

class LDAPCompletionItem : public CompletionItem
{
public:
    LDAPCompletionItem(KLDAP::LdapClient *ldapClient)
        : mLdapClient(ldapClient)
    {
        mWeight = mLdapClient->completionWeight();
    }

    QString label() const Q_DECL_OVERRIDE
    {
        return i18n("LDAP server %1", mLdapClient->server().host());
    }

    QIcon icon() const Q_DECL_OVERRIDE
    {
        return QIcon::fromTheme(QStringLiteral("view-ldap-resource"));
    }

    int completionWeight() const Q_DECL_OVERRIDE
    {
        return mWeight;
    }

    void save(CompletionOrderWidget *) Q_DECL_OVERRIDE {
        KConfig *config = KLDAP::LdapClientSearchConfig::config();
        KConfigGroup group(config, "LDAP");
        group.writeEntry(QStringLiteral("SelectedCompletionWeight%1").arg(mLdapClient->clientNumber()),
        mWeight);
        group.sync();
    }

    bool hasEnableSupport() const Q_DECL_OVERRIDE
    {
        return false;
    }

    bool isEnabled() const Q_DECL_OVERRIDE
    {
        return true;
    }

    void setIsEnabled(bool b) Q_DECL_OVERRIDE {
        Q_UNUSED(b);
    }

protected:
    void setCompletionWeight(int weight) Q_DECL_OVERRIDE {
        mWeight = weight;
    }

private:
    KLDAP::LdapClient *mLdapClient;
    int mWeight;
};

class SimpleCompletionItem : public CompletionItem
{
public:
    SimpleCompletionItem(CompletionOrderWidget *editor, const QString &label, const QString &identifier, int weight, bool enableSupport = false)
        : mLabel(label), mIdentifier(identifier), mHasEnableSupport(enableSupport), mEnabled(true)
    {
        KConfigGroup groupCompletionWeights(editor->configFile(), "CompletionWeights");
        mWeight = groupCompletionWeights.readEntry(mIdentifier, weight);
        if (mHasEnableSupport) {
            KConfigGroup groupEnabled(editor->configFile(), "CompletionEnabled");
            mEnabled = groupEnabled.readEntry(mIdentifier, true);
        }
    }

    bool isEnabled() const  Q_DECL_OVERRIDE
    {
        return mEnabled;
    }

    bool hasEnableSupport() const Q_DECL_OVERRIDE
    {
        return mHasEnableSupport;
    }

    void setIcon(const QIcon &icon)
    {
        mIcon = icon;
    }

    QString label() const Q_DECL_OVERRIDE
    {
        return mLabel;
    }

    QIcon icon() const Q_DECL_OVERRIDE
    {
        return mIcon;
    }

    int completionWeight() const Q_DECL_OVERRIDE
    {
        return mWeight;
    }

    void setIsEnabled(bool b) Q_DECL_OVERRIDE {
        mEnabled = b;
    }

    void save(CompletionOrderWidget *editor) Q_DECL_OVERRIDE {
        KConfigGroup group(editor->configFile(), "CompletionWeights");
        group.writeEntry(mIdentifier, mWeight);
        if (mHasEnableSupport)
        {
            KConfigGroup groupEnabled(editor->configFile(), "CompletionEnabled");
            //groupEnabled.writeEntry(mIdentifier, );
        }
    }

protected:
    void setCompletionWeight(int weight) Q_DECL_OVERRIDE {
        mWeight = weight;
    }

private:
    QString mLabel;
    QString mIdentifier;
    int mWeight;
    QIcon mIcon;
    bool mHasEnableSupport;
    bool mEnabled;
};

/////////

class CompletionViewItem : public QTreeWidgetItem
{
public:
    CompletionViewItem(QTreeWidget *parent, CompletionItem *item)
        : QTreeWidgetItem(parent)
    {
        setItem(item);
    }

    void setItem(CompletionItem *item)
    {
        mItem = item;
        setText(0, mItem->label());
        setIcon(0, mItem->icon());
        if (mItem->hasEnableSupport()) {
            setFlags(flags() | Qt::ItemIsUserCheckable);
            setCheckState(0, mItem->isEnabled() ? Qt::Checked : Qt::Unchecked);
        } else {
            setFlags(flags() & ~ Qt::ItemIsUserCheckable);
        }

    }

    CompletionItem *item() const
    {
        return mItem;
    }

    bool operator<(const QTreeWidgetItem &other) const Q_DECL_OVERRIDE
    {
        const QTreeWidgetItem *otherItem = &other;
        const CompletionViewItem *completionItem = static_cast<const CompletionViewItem *>(otherItem);
        // item with weight 100 should be on the top -> reverse sorting
        return (mItem->completionWeight() > completionItem->item()->completionWeight());
    }

private:
    CompletionItem *mItem;
};

CompletionOrderWidget::CompletionOrderWidget(QWidget *parent)
    : QWidget(parent),
      mConfig(QStringLiteral("kpimcompletionorder")),
      mCollectionModel(Q_NULLPTR),
      mLdapSearch(0),
      mDirty(false)
{
    new CompletionOrderEditorAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAdaptors);

    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);

    QWidget *page = new QWidget(this);
    QHBoxLayout *pageHBoxLayout = new QHBoxLayout(page);
    pageHBoxLayout->setMargin(0);
    hbox->addWidget(page);
    mListView = new QTreeWidget(page);
    mListView->setObjectName(QStringLiteral("listview"));

    pageHBoxLayout->addWidget(mListView);
    mListView->setColumnCount(1);
    mListView->setAlternatingRowColors(true);
    mListView->setIndentation(0);
    mListView->setAllColumnsShowFocus(true);
    mListView->setHeaderHidden(true);
    mListView->setSortingEnabled(true);

    QWidget *upDownBox = new QWidget(page);
    QVBoxLayout *upDownBoxVBoxLayout = new QVBoxLayout(upDownBox);
    upDownBoxVBoxLayout->setMargin(0);
    pageHBoxLayout->addWidget(upDownBox);
    mUpButton = new QPushButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(mUpButton);
    mUpButton->setAutoRepeat(true);
    mUpButton->setObjectName(QStringLiteral("mUpButton"));
    mUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    mUpButton->setEnabled(false);   // b/c no item is selected yet
    mUpButton->setFocusPolicy(Qt::StrongFocus);

    mDownButton = new QPushButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(mDownButton);
    mDownButton->setAutoRepeat(true);
    mDownButton->setObjectName(QStringLiteral("mDownButton"));
    mDownButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    mDownButton->setEnabled(false);   // b/c no item is selected yet
    mDownButton->setFocusPolicy(Qt::StrongFocus);

    QWidget *spacer = new QWidget(upDownBox);
    upDownBoxVBoxLayout->addWidget(spacer);
    upDownBoxVBoxLayout->setStretchFactor(spacer, 100);

    connect(mListView, &QTreeWidget::itemSelectionChanged,
            this, &CompletionOrderWidget::slotSelectionChanged);
    connect(mListView, &QTreeWidget::currentItemChanged,
            this, &CompletionOrderWidget::slotSelectionChanged);
    connect(mUpButton, &QAbstractButton::clicked, this, &CompletionOrderWidget::slotMoveUp);
    connect(mDownButton, &QAbstractButton::clicked, this, &CompletionOrderWidget::slotMoveDown);

}

CompletionOrderWidget::~CompletionOrderWidget()
{
}

void CompletionOrderWidget::save()
{
    if (mDirty) {
        int w = 100;
        //Clean up order
        KConfigGroup group(configFile(), "CompletionWeights");
        group.deleteGroup();

        for (int itemIndex = 0; itemIndex < mListView->topLevelItemCount(); ++itemIndex) {
            CompletionViewItem *item =
                static_cast<CompletionViewItem *>(mListView->topLevelItem(itemIndex));
            item->item()->setCompletionWeight(w);
            item->item()->setIsEnabled(item->checkState(0) == Qt::Checked);
            item->item()->save(this);
            --w;
        }
        Q_EMIT completionOrderChanged();
    }
}

KConfig *CompletionOrderWidget::configFile()
{
    return &mConfig;
}

void CompletionOrderWidget::addRecentAddressItem()
{
    //Be default it's the first.
    SimpleCompletionItem *item = new SimpleCompletionItem(this, i18n("Recent Addresses"), QStringLiteral("Recent Addresses"), 10);
    item->setIcon(QIcon::fromTheme(QStringLiteral("kmail")));
    new CompletionViewItem(mListView, item);
}

void CompletionOrderWidget::addCompletionItemForCollection(const QModelIndex &index)
{
    const Akonadi::Collection collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    if (!collection.isValid()) {
        return;
    }

    SimpleCompletionItem *item = new SimpleCompletionItem(this, index.data().toString(), QString::number(collection.id()), 60);
    item->setIcon(index.data(Qt::DecorationRole).value<QIcon>());

    new CompletionViewItem(mListView, item);
}

void CompletionOrderWidget::loadCompletionItems()
{
    if (mLdapSearch) {
        // The first step is to gather all the data, creating CompletionItem objects
        foreach (KLDAP::LdapClient *client, mLdapSearch->clients()) {
            new CompletionViewItem(mListView, new LDAPCompletionItem(client));
        }
    }

    Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder(this);
    monitor->fetchCollection(true);
    monitor->setCollectionMonitored(Akonadi::Collection::root());
    monitor->setMimeTypeMonitored(KContacts::Addressee::mimeType(), true);
    monitor->setMimeTypeMonitored(KContacts::ContactGroup::mimeType(), true);

    Akonadi::EntityTreeModel *model = new Akonadi::EntityTreeModel(monitor, this);
    model->setItemPopulationStrategy(Akonadi::EntityTreeModel::NoItemPopulation);

    KDescendantsProxyModel *descendantsProxy = new KDescendantsProxyModel(this);
    descendantsProxy->setDisplayAncestorData(true);
    descendantsProxy->setSourceModel(model);

    Akonadi::CollectionFilterProxyModel *mimeTypeProxy = new Akonadi::CollectionFilterProxyModel(this);
    mimeTypeProxy->addMimeTypeFilters(QStringList() << KContacts::Addressee::mimeType()
                                      << KContacts::ContactGroup::mimeType());
    mimeTypeProxy->setSourceModel(descendantsProxy);
    mimeTypeProxy->setExcludeVirtualCollections(true);

    mCollectionModel = mimeTypeProxy;

    connect(mimeTypeProxy, &QAbstractItemModel::rowsInserted,
            this, &CompletionOrderWidget::rowsInserted);

    for (int row = 0; row < mCollectionModel->rowCount(); ++row) {
        addCompletionItemForCollection(mCollectionModel->index(row, 0));
    }

    addRecentAddressItem();

    mListView->sortItems(0, Qt::AscendingOrder);
}

void CompletionOrderWidget::setLdapClientSearch(KLDAP::LdapClientSearch *ldapSearch)
{
    mLdapSearch = ldapSearch;
}

void CompletionOrderWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {
        addCompletionItemForCollection(mCollectionModel->index(row, 0, parent));
    }

    mListView->sortItems(0, Qt::AscendingOrder);
}

void CompletionOrderWidget::slotSelectionChanged()
{
    QTreeWidgetItem *item = mListView->currentItem();
    mDownButton->setEnabled(item && mListView->itemBelow(item));
    mUpButton->setEnabled(item && mListView->itemAbove(item));
}

static void swapItems(CompletionViewItem *one, CompletionViewItem *other)
{
    CompletionItem *oneCompletion = one->item();
    CompletionItem *otherCompletion = other->item();

    int weight = otherCompletion->completionWeight();
    otherCompletion->setCompletionWeight(oneCompletion->completionWeight());
    oneCompletion->setCompletionWeight(weight);

    one->setItem(otherCompletion);
    other->setItem(oneCompletion);
}

void CompletionOrderWidget::slotMoveUp()
{
    CompletionViewItem *item = static_cast<CompletionViewItem *>(mListView->currentItem());
    if (!item) {
        return;
    }
    CompletionViewItem *above = static_cast<CompletionViewItem *>(mListView->itemAbove(item));
    if (!above) {
        return;
    }
    swapItems(item, above);
    mListView->setCurrentItem(above, 0, QItemSelectionModel::Select | QItemSelectionModel::Current);
    mListView->sortItems(0, Qt::AscendingOrder);
    mDirty = true;
}

void CompletionOrderWidget::slotMoveDown()
{
    CompletionViewItem *item = static_cast<CompletionViewItem *>(mListView->currentItem());
    if (!item) {
        return;
    }
    CompletionViewItem *below = static_cast<CompletionViewItem *>(mListView->itemBelow(item));
    if (!below) {
        return;
    }
    swapItems(item, below);
    mListView->setCurrentItem(below);
    mListView->setCurrentItem(below, 0, QItemSelectionModel::Select | QItemSelectionModel::Current);
    mListView->sortItems(0, Qt::AscendingOrder);
    mDirty = true;
}
