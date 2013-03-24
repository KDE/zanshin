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

PimItemMonitor::PimItemMonitor(const PimItem::Ptr &item, QObject* parent)
:   QObject(parent),
    m_monitor(0),
    m_itemOutdated(false),
    mItem(item)
{
    QMetaObject::invokeMethod(this, "enableMonitor");
    QMetaObject::invokeMethod(this, "fetchPayload");
}

bool PimItemMonitor::saveItem()
{
    kDebug();
    if (m_itemOutdated) {
        kWarning() << "item fetch in progress, cannot save without conflict";
        return false;
    }
    m_itemOutdated = true;

    //create a session which is ignored by this monitor, other items still receive the changed signal from the monitor
    //FIXME this doesn't work. Noone will receive signals about this change since everyone listens to the main session. At least thats what the tests say, according to the docs it should be different. Investigate.
    Akonadi::Session *session = new Akonadi::Session();
    if (m_monitor) {
        m_monitor->ignoreSession(session);
    }
    //kDebug() << m_item.revision();

    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(mItem->getItem(), session);
    connect(modifyJob, SIGNAL(result(KJob*)), SLOT(modifyDone(KJob*)) );
    connect(modifyJob, SIGNAL(result(KJob*)), session, SLOT(deleteLater())); //This will delete the session as soon as the job finished which will in turn delete the job
    return true;
}

void PimItemMonitor::modifyDone( KJob *job )
{
    //Updateing the item does not help because the akonadi resource will modifiy the item again to update the remote revision,
    //which will result anyways in an outdate item here if the monitor is not enabled
    if ( job->error() ) {
        kWarning() << job->errorString();
        return;
    }
//     Akonadi::ItemModifyJob *modjob = qobject_cast<Akonadi::ItemModifyJob*>(job);
//     m_item->setItem(modjob->item());
//     m_itemOutdated = false;
//     Q_ASSERT(m_item.isValid());
//     kDebug() << "item modified";
}

void PimItemMonitor::updateItem(const Akonadi::Item &item, const QSet<QByteArray> &changes)
{
//     kDebug() << "new item" << item.id() << item.revision() << item.modificationTime() << "old item" << m_item.id() << m_item.revision() << m_item.modificationTime();
    kDebug() << changes;
    Q_ASSERT(item.id() == mItem->getItem().id());

    if (changes.contains("REMOTEID") && changes.size() == 1) { //we don't care if the remoteid changed
        kDebug() << "remoteid changed";
        mItem->m_item = item;
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
    Q_ASSERT(item.isValid());
    PimItem::Ptr oldItem(PimItemFactory::getItem(mItem->getItem()));
    mItem->setItem(item);
    m_itemOutdated = false;

    //TODO check what has changed, i.e we don't care about a change of the remoteid
    //TODO what about lastModified
    //FIXME the changes strings seem to be subject to change (PLD: -> PLD:RFC822), is there a not string based version?
    if (changes.contains("ATR:ENTITYDISPLAY") || changes.contains("PLD:RFC822")) { //only the displayattribute and the payload are relevant for the content

        //TODO update local variable like m_text, m_title, etc. otherwise the changes are overwritten on the next save item
        //we have to figure out though which data was update in case i.e. the title is stored in the entitydisplayattribue and the payload
        parts |= Payload;

        if (changes.contains("ATR:ENTITYDISPLAY")) { //the entitydisplayattribue was modified, so it is more up to date than the title from the payload
            Q_ASSERT(mItem->getItem().hasAttribute<Akonadi::EntityDisplayAttribute>());
            Akonadi::EntityDisplayAttribute *att = mItem->m_item.attribute<Akonadi::EntityDisplayAttribute>();
            mItem->setTitle(att->displayName());
            //kDebug() << "displayattribute was modified " << m_title;
        }

        if (oldItem->getText() != mItem->getText()) {
            kDebug() << "text changed";
            parts |= Text;
        }
        if (oldItem->getTitle() != mItem->getTitle()) {
            kDebug() << "title changed";
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
        kDebug() << "has validpayload";
        emit payloadFetchComplete();
        return;
    }
    kDebug() << "no valid payload, fetching...";
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(mItem->m_item, this);
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

    mItem->m_item = fetchJob->items().first();
    if (!mItem->m_item.isValid()) {
        kWarning() << "Item not valid";
        return;
    }
    Q_ASSERT(mItem->m_item.hasPayload());
    kDebug() << "item fetch complete";
    m_itemOutdated = false;
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
    /*
     * when monitoring is enabled, we work for a longer period with the item, and also save the item, so we will need the full payload
     */
    m_monitor->itemFetchScope().fetchFullPayload();
    m_monitor->itemFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>(true);
    m_monitor->setItemMonitored(mItem->getItem());
    connect( m_monitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), this, SLOT(updateItem(Akonadi::Item,QSet<QByteArray>)));
    connect( m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SIGNAL(removed()));
    kDebug() << "monitoring of item " << mItem->m_item.id() << " started";
}
