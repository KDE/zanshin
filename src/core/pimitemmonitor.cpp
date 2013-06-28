/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pimitemmonitor.h"
#include "pimitemfactory.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/EntityDisplayAttribute>

PimItemMonitor::PimItemMonitor(const AkonadiBaseItem::Ptr &item, QObject* parent)
:   QObject(parent),
    m_monitor(0),
    m_session(new Akonadi::Session("zanshinpimitemmonitor", this)),
    m_itemOutdated(false),
    mItem(item)
{
    QMetaObject::invokeMethod(this, "enableMonitor", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "fetchPayload", Qt::QueuedConnection);
}

bool PimItemMonitor::saveItem()
{
    kDebug();
    if (m_itemOutdated) {
        kWarning() << "item fetch in progress, cannot save without conflict";
        return false;
    }
    m_itemOutdated = true;

    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(mItem->getItem(), m_session);
    connect(modifyJob, SIGNAL(result(KJob*)), SLOT(modifyDone(KJob*)) );
    return true;
}

void PimItemMonitor::modifyDone(KJob* job)
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        m_itemOutdated = false;
        return;
    }
    Akonadi::ItemModifyJob *modifyJob = static_cast<Akonadi::ItemModifyJob*>(job);
    itemUpdated(modifyJob->item());
}

void PimItemMonitor::itemUpdated(const Akonadi::Item &newItem)
{
    mItem->setItem(newItem);
    m_itemOutdated = false;
}

void PimItemMonitor::updateItem(const Akonadi::Item &item, const QSet<QByteArray> &changes)
{
//     kDebug() << "new item" << item.id() << item.revision() << item.modificationTime() << "old item" << m_item.id() << m_item.revision() << m_item.modificationTime();
    kDebug() << changes;
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.id() == mItem->getItem().id());

    if (changes.contains("REMOTEID") && changes.size() == 1) { //we don't care if the remoteid changed
        kDebug() << "remoteid changed";
        itemUpdated(item);
        return;
    }

    int parts = 0;

    if (item.modificationTime() != mItem->m_item.modificationTime()) {
        parts |= LastModifiedDate;
    }

    /* TODO we should check here if this really is the data which was modified in the last call to saveItem
     * It could be possible that we receive a foreign update (another app changed the same akonadi item),
     * during the time where we wait for the updated content after the item save
     */
    PimItem::Ptr oldItem(PimItemFactory::getItem(mItem->getItem()));
    itemUpdated(item);

    //TODO what about lastModified
    if (changes.contains("ATR:ENTITYDISPLAY") || changes.contains("PLD:RFC822")) { //only the displayattribute and the payload are relevant for the content
        parts |= Payload;
        if (changes.contains("ATR:ENTITYDISPLAY")) { //the entitydisplayattribue was modified, so it is more up to date than the title from the payload
            Q_ASSERT(item.hasAttribute<Akonadi::EntityDisplayAttribute>());
            Akonadi::EntityDisplayAttribute *att = item.attribute<Akonadi::EntityDisplayAttribute>();
            mItem->setTitle(att->displayName());
        }

        if (oldItem->text() != mItem->text()) {
            // kDebug() << "text changed";
            parts |= Text;
        }
        if (oldItem->title() != mItem->title()) {
            // kDebug() << "title changed";
            parts |= Title;
        }
    }

    if (parts) {
        emit changed(static_cast<ChangedParts>(parts));
    }
}

void PimItemMonitor::fetchPayload()
{
    if (mItem->hasValidPayload()) {
        emit payloadFetchComplete();
        return;
    }
    kDebug() << "no valid payload, fetching...";
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(mItem->getItem(), this);
    fetchJob->fetchScope().fetchFullPayload();
    connect( fetchJob, SIGNAL( result( KJob* ) ),this, SLOT( itemFetchDone( KJob* ) ) );
    m_itemOutdated = true;
}

void PimItemMonitor::itemFetchDone( KJob *job )
{
    Akonadi::ItemFetchJob *fetchJob = static_cast<Akonadi::ItemFetchJob*>( job );
    if (job->error()) {
        kError() << job->errorString();
        return;
    }
    const Akonadi::Item item = fetchJob->items().first();
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload());
    itemUpdated(item);
    kDebug() << "item fetch complete";
    emit payloadFetchComplete();
}

void PimItemMonitor::enableMonitor()
{
    if (!mItem->getItem().isValid()) {
        kWarning() << "item is not valid, monitor not enabled";
        return;
    }
    if (m_monitor) {
        kDebug() << "monitor already enabled";
        return;
    }
    m_monitor = new Akonadi::Monitor(this);
    m_monitor->itemFetchScope().fetchFullPayload();
    m_monitor->itemFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>(true);
    m_monitor->setItemMonitored(mItem->getItem());
    //ignore this session so we can ignore our own changes
    //FIXME this doesn't work. Noone will receive signals about this change since everyone listens to the main session. At least thats what the tests say, according to the docs it should be different. Investigate.
    m_monitor->ignoreSession(m_session);
    connect( m_monitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), this, SLOT(updateItem(Akonadi::Item,QSet<QByteArray>)));
    connect( m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SIGNAL(removed()));
    // kDebug() << "monitoring of item " << mItem->m_item.id() << " started";
}

