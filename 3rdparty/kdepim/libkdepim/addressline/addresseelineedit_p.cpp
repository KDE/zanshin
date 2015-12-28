/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "completionorder/completionordereditor.h"
#include "addresseelineeditstatic.h"
#include "addresseelineedit_p.h"
#include "addresseelineedit.h"
#include "kmailcompletion.h"
#include "libkdepim_debug.h"
#include <QMap>
#include <QTimer>
#include <kcolorscheme.h>
#include <kcompletionbox.h>
#include <AkonadiCore/Session>
#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/Job>
#include <QApplication>
#include <KConfigGroup>
#include <KLocalizedString>
#include <AkonadiSearch/PIM/contactcompleter.h>

#include <ldap/ldapclientsearch.h>
#include <addressline/baloocompletionemail.h>
#include <akonadi/contact/contactsearchjob.h>
#include <QNetworkConfigurationManager>

#include <blacklistbaloocompletion/blacklistbalooemailcompletiondialog.h>
static QNetworkConfigurationManager *s_networkConfigMgr = 0;

namespace KPIM
{
Q_GLOBAL_STATIC(AddresseeLineEditStatic, s_static)
AddresseeLineEditPrivate::AddresseeLineEditPrivate(KPIM::AddresseeLineEdit *qq, bool enableCompletion)
    : QObject(qq),
      q(qq),
      m_recentAddressConfig(Q_NULLPTR),
      m_useCompletion(enableCompletion),
      m_completionInitialized(false),
      m_smartPaste(false),
      m_addressBookConnected(false),
      m_lastSearchMode(false),
      m_searchExtended(false),
      m_useSemicolonAsSeparator(false),
      m_showOU(false),
      m_enableBalooSearch(true),
      mExpandIntern(true),
      mAutoGroupExpand(false),
      mShowRecentAddresses(true)
{
    if (!s_networkConfigMgr) {
        s_networkConfigMgr = new QNetworkConfigurationManager(QCoreApplication::instance());
    }

    m_delayedQueryTimer.setSingleShot(true);
    connect(&m_delayedQueryTimer, &QTimer::timeout, this, &AddresseeLineEditPrivate::slotTriggerDelayedQueries);

}

AddresseeLineEditPrivate::~AddresseeLineEditPrivate()
{
    if (s_static->ldapSearch && s_static->ldapLineEdit == q) {
        stopLDAPLookup();
    }
}

void AddresseeLineEditPrivate::restartTime(const QString &searchString)
{
    if (useCompletion() && s_static->ldapTimer) {
        if (s_static->ldapText != searchString || s_static->ldapLineEdit != q) {
            stopLDAPLookup();
        }

        s_static->ldapText = searchString;
        s_static->ldapLineEdit = q;
        s_static->ldapTimer->setSingleShot(true);
        s_static->ldapTimer->start(500);
    }
}

static const QString s_completionItemIndentString = QStringLiteral("     ");

class SourceWithWeight
{
public:
    int weight;           // the weight of the source
    int index;            // index into s_static->completionSources
    QString sourceName;   // the name of the source, e.g. "LDAP Server"

    bool operator< (const SourceWithWeight &other) const
    {
        if (weight > other.weight) {
            return true;
        }

        if (weight < other.weight) {
            return false;
        }

        return sourceName < other.sourceName;
    }
};

void AddresseeLineEditPrivate::init()
{
    if (!s_static.exists()) {
        s_static->completion->setOrder(KCompletion::Weighted);
        s_static->completion->setIgnoreCase(true);
    }

    if (m_useCompletion) {
        if (!s_static->ldapTimer) {
            s_static->ldapTimer = new QTimer;
            s_static->ldapSearch = new KLDAP::LdapClientSearch;

            /* The reasoning behind this filter is:
             * If it's a person, or a distlist, show it, even if it doesn't have an email address.
             * If it's not a person, or a distlist, only show it if it has an email attribute.
             * This allows both resource accounts with an email address which are not a person and
             * person entries without an email address to show up, while still not showing things
             * like structural entries in the ldap tree.
             */

#if 0
            s_static->ldapSearch->setFilter(QStringLiteral("&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                            "(|(cn=%1*)(mail=%1*)(mail=*@%1*)(givenName=%1*)(sn=%1*))"));
#endif
            //Fix bug 323272 "Exchange doesn't like any queries beginning with *."
            s_static->ldapSearch->setFilter(QStringLiteral("&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                            "(|(cn=%1*)(mail=%1*)(givenName=%1*)(sn=%1*))"));

        }

        s_static->balooCompletionSource = q->addCompletionSource(i18nc("@title:group", "Contacts found in your data"), -1);

        s_static->updateLDAPWeights();
        if (!m_completionInitialized) {
            q->setCompletionObject(s_static->completion, false);
            connect(q, &KLineEdit::completion,
                    this, &AddresseeLineEditPrivate::slotCompletion);
            connect(q, SIGNAL(returnPressed(QString)),
                    this, SLOT(slotReturnPressed(QString)));

            KCompletionBox *box = q->completionBox();
            connect(box, SIGNAL(activated(QString)),
                    this, SLOT(slotPopupCompletion(QString)));
            connect(box, &KCompletionBox::userCancelled,
                    this, &AddresseeLineEditPrivate::slotUserCancelled);
            connect(s_static->ldapTimer, &QTimer::timeout, this, &AddresseeLineEditPrivate::slotStartLDAPLookup);
            connect(s_static->ldapSearch, SIGNAL(searchData(KLDAP::LdapResult::List)),
                    SLOT(slotLDAPSearchData(KLDAP::LdapResult::List)));

            m_completionInitialized = true;
        }

        KConfigGroup group(KSharedConfig::openConfig(), "AddressLineEdit");
        m_showOU = group.readEntry("ShowOU", false);
        mAutoGroupExpand = group.readEntry("AutoGroupExpand", false);

        loadBalooBlackList();
    }
}

void AddresseeLineEditPrivate::startLoadingLDAPEntries()
{
    QString text(s_static->ldapText);

    // TODO cache last?
    QString prevAddr;
    const int index = text.lastIndexOf(QLatin1Char(','));
    if (index >= 0) {
        prevAddr = text.left(index + 1) + QLatin1Char(' ');
        text = text.mid(index + 1, 255).trimmed();
    }

    if (text.isEmpty()) {
        return;
    }

    s_static->ldapSearch->startSearch(text);
}

void AddresseeLineEditPrivate::stopLDAPLookup()
{
    s_static->ldapSearch->cancelSearch();
    s_static->ldapLineEdit = 0;
}

QStringList AddresseeLineEdit::cleanupEmailList(const QStringList &inputList)
{
    return d->cleanupEmailList(inputList);
}

QStringList AddresseeLineEditPrivate::cleanupEmailList(const QStringList &inputList)
{
    KPIM::BalooCompletionEmail completionEmail;
    completionEmail.setEmailList(inputList);
    completionEmail.setBlackList(m_balooBlackList);
    completionEmail.setExcludeDomain(m_domainExcludeList);
    const QStringList listEmail = completionEmail.cleanupEmailList();
    return listEmail;
}

void AddresseeLineEditPrivate::searchInBaloo()
{
    const QString trimmedString = m_searchString.trimmed();
    Akonadi::Search::PIM::ContactCompleter com(trimmedString, 20);
    const QStringList listEmail = cleanupEmailList(com.complete());
    Q_FOREACH (const QString &email, listEmail) {
        addCompletionItem(email, 1, s_static->balooCompletionSource);
    }
    doCompletion(m_lastSearchMode);
    //  if ( q->hasFocus() || q->completionBox()->hasFocus() ) {
    //}
}

void AddresseeLineEditPrivate::alternateColor()
{
    const KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
    m_alternateColor = colorScheme.background(KColorScheme::AlternateBackground).color();
}

void AddresseeLineEditPrivate::setCompletedItems(const QStringList &items, bool autoSuggest)
{
    KCompletionBox *completionBox = q->completionBox();

    if (!items.isEmpty() &&
            !(items.count() == 1 && m_searchString == items.first())) {

        completionBox->clear();
        const int numberOfItems(items.count());
        for (int i = 0; i < numberOfItems; ++i) {
            QListWidgetItem *item = new QListWidgetItem(items.at(i), completionBox);
            if (!items.at(i).startsWith(s_completionItemIndentString)) {
                if (!m_alternateColor.isValid()) {
                    alternateColor();
                }
                item->setFlags(item->flags() & ~ Qt::ItemIsSelectable);
                item->setBackgroundColor(m_alternateColor);
            }
            completionBox->addItem(item);
        }
        if (!completionBox->isVisible()) {
            if (!m_searchString.isEmpty()) {
                completionBox->setCancelledText(m_searchString);
            }
            completionBox->popup();
            // we have to install the event filter after popup(), since that
            // calls show(), and that's where KCompletionBox installs its filter.
            // We want to be first, though, so do it now.
            if (s_static->completion->order() == KCompletion::Weighted) {
                qApp->installEventFilter(q);
            }
        }

        QListWidgetItem *item = completionBox->item(1);
        if (item) {
            completionBox->blockSignals(true);
            completionBox->setCurrentItem(item);
            item->setSelected(true);
            completionBox->blockSignals(false);
        }

        if (autoSuggest) {
            const int index = items.first().indexOf(m_searchString);
            const QString newText = items.first().mid(index);
            q->callSetUserSelection(false);
            q->callSetCompletedText(newText, true);
        }
    } else {
        if (completionBox && completionBox->isVisible()) {
            completionBox->hide();
            completionBox->setItems(QStringList());
        }
    }
}

void AddresseeLineEditPrivate::addCompletionItem(const QString &string, int weight,
        int completionItemSource,
        const QStringList *keyWords)
{
    // Check if there is an exact match for item already, and use the
    // maximum weight if so. Since there's no way to get the information
    // from KCompletion, we have to keep our own QMap.
    // We also update the source since the item should always be shown from the source with the highest weight

    AddresseeLineEditStatic::CompletionItemsMap::iterator it = s_static->completionItemMap.find(string);
    if (it != s_static->completionItemMap.end()) {
        weight = qMax((*it).first, weight);
        (*it).first = weight;
        (*it).second = completionItemSource;
    } else {
        s_static->completionItemMap.insert(string, qMakePair(weight, completionItemSource));
    }

    s_static->completion->addItem(string, weight);
    if (keyWords && !keyWords->isEmpty()) {
        s_static->completion->addItemWithKeys(string, weight, keyWords);    // see kmailcompletion.cpp
    }
}

const QStringList KPIM::AddresseeLineEditPrivate::adjustedCompletionItems(bool fullSearch)
{
    QStringList items = fullSearch ?
                        s_static->completion->allMatches(m_searchString) :
                        s_static->completion->substringCompletion(m_searchString);

    //force items to be sorted by email
    items.sort();

    // For weighted mode, the algorithm is the following:
    // In the first loop, we add each item to its section (there is one section per completion source)
    // We also add spaces in front of the items.
    // The sections are appended to the items list.
    // In the second loop, we then walk through the sections and add all the items in there to the
    // sorted item list, which is the final result.
    //
    // The algo for non-weighted mode is different.

    int lastSourceIndex = -1;
    unsigned int i = 0;

    // Maps indices of the items list, which are section headers/source items,
    // to a QStringList which are the items of that section/source.
    QMap<int, QStringList> sections;
    QStringList sortedItems;
    for (QStringList::Iterator it = items.begin(); it != items.end(); ++it, ++i) {
        AddresseeLineEditStatic::CompletionItemsMap::const_iterator cit = s_static->completionItemMap.constFind(*it);
        if (cit == s_static->completionItemMap.constEnd()) {
            continue;
        }

        const int index = (*cit).second;

        if (s_static->completion->order() == KCompletion::Weighted) {
            if (lastSourceIndex == -1 || lastSourceIndex != index) {
                const QString sourceLabel(s_static->completionSources.at(index));
                if (sections.find(index) == sections.end()) {
                    it = items.insert(it, sourceLabel);
                    ++it; //skip new item
                }
                lastSourceIndex = index;
            }

            (*it) = (*it).prepend(s_completionItemIndentString);
            // remove preferred email sort <blank> added in  addContact()
            (*it).replace(QLatin1String("  <"), QStringLiteral(" <"));
        }
        sections[ index ].append(*it);

        if (s_static->completion->order() == KCompletion::Sorted) {
            sortedItems.append(*it);
        }
    }

    if (s_static->completion->order() == KCompletion::Weighted) {

        // Sort the sections
        QList<SourceWithWeight> sourcesAndWeights;
        const int numberOfCompletionSources(s_static->completionSources.size());
        sourcesAndWeights.reserve(numberOfCompletionSources);
        for (int i = 0; i < numberOfCompletionSources; ++i) {
            SourceWithWeight sww;
            sww.sourceName = s_static->completionSources.at(i);
            sww.weight = s_static->completionSourceWeights[sww.sourceName];
            sww.index = i;
            sourcesAndWeights.append(sww);
        }

        qSort(sourcesAndWeights.begin(), sourcesAndWeights.end());
        // Add the sections and their items to the final sortedItems result list
        const int numberOfSources(sourcesAndWeights.size());
        for (int i = 0; i < numberOfSources; ++i) {
            const SourceWithWeight source = sourcesAndWeights.at(i);
            const QStringList sectionItems = sections[source.index];
            if (!sectionItems.isEmpty()) {
                sortedItems.append(source.sourceName);
                foreach (const QString &itemInSection, sectionItems) {
                    sortedItems.append(itemInSection);
                }
            }
        }
    } else {
        sortedItems.sort();
    }

    return sortedItems;
}

void AddresseeLineEditPrivate::updateSearchString()
{
    m_searchString = q->text();

    int n = -1;
    bool inQuote = false;
    const int searchStringLength = m_searchString.length();
    for (int i = 0; i < searchStringLength; ++i) {
        const QChar searchChar = m_searchString.at(i);
        if (searchChar == QLatin1Char('"')) {
            inQuote = !inQuote;
        }

        if (searchChar == QLatin1Char('\\') &&
                (i + 1) < searchStringLength && m_searchString.at(i + 1) == QLatin1Char('"')) {
            ++i;
        }

        if (inQuote) {
            continue;
        }

        if (i < searchStringLength &&
                (searchChar == QLatin1Char(',') ||
                 (m_useSemicolonAsSeparator && searchChar == QLatin1Char(';')))) {
            n = i;
        }
    }

    if (n >= 0) {
        ++n; // Go past the ","

        const int len = m_searchString.length();

        // Increment past any whitespace...
        while (n < len && m_searchString.at(n).isSpace()) {
            ++n;
        }

        m_previousAddresses = m_searchString.left(n);
        m_searchString = m_searchString.mid(n).trimmed();
    } else {
        m_previousAddresses.clear();
    }
}

void AddresseeLineEditPrivate::slotTriggerDelayedQueries()
{
    if (m_searchString.isEmpty() || m_searchString.trimmed().size() <= 2) {
        return;
    }

    if (m_enableBalooSearch) {
        searchInBaloo();
    }

    // We send a contactsearch job through akonadi.
    // This not only searches baloo but also servers if remote search is enabled
    akonadiPerformSearch();
}

void AddresseeLineEditPrivate::startSearches()
{
    if (!m_delayedQueryTimer.isActive()) {
        m_delayedQueryTimer.start(50);
    }
}

void AddresseeLineEditPrivate::akonadiPerformSearch()
{
    qCDebug(LIBKDEPIM_LOG) << "searching akonadi with:" << m_searchString;

    // first, kill all job still in flight, they are no longer current
    Q_FOREACH (const QWeakPointer<Akonadi::Job> &job, s_static->akonadiJobsInFlight) {
        if (!job.isNull()) {
            job.data()->kill();
        }
    }
    s_static->akonadiJobsInFlight.clear();

    // now start new jobs
    Akonadi::ContactSearchJob *contactJob = new Akonadi::ContactSearchJob(s_static->akonadiSession);
    contactJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    contactJob->setQuery(Akonadi::ContactSearchJob::NameOrEmail, m_searchString,
                         Akonadi::ContactSearchJob::ContainsWordBoundaryMatch);
    connect(contactJob, &Akonadi::ItemSearchJob::itemsReceived,
            this, &AddresseeLineEditPrivate::slotAkonadiHandleItems);
    connect(contactJob, &KJob::result,
            this, &AddresseeLineEditPrivate::slotAkonadiSearchResult);

    Akonadi::ContactGroupSearchJob *groupJob = new Akonadi::ContactGroupSearchJob(s_static->akonadiSession);
    groupJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    groupJob->setQuery(Akonadi::ContactGroupSearchJob::Name, m_searchString,
                       Akonadi::ContactGroupSearchJob::ContainsMatch);
    connect(contactJob, &Akonadi::ItemSearchJob::itemsReceived,
            this, &AddresseeLineEditPrivate::slotAkonadiHandleItems);
    connect(groupJob, &KJob::result,
            this, &AddresseeLineEditPrivate::slotAkonadiSearchResult);

    s_static->akonadiJobsInFlight.append(contactJob);
    s_static->akonadiJobsInFlight.append(groupJob);
    akonadiHandlePending();
}

void AddresseeLineEditPrivate::akonadiHandlePending()
{
    qCDebug(LIBKDEPIM_LOG) << "Pending items: " << s_static->akonadiPendingItems.size();
    Akonadi::Item::List::iterator it = s_static->akonadiPendingItems.begin();
    while (it != s_static->akonadiPendingItems.end()) {
        const Akonadi::Item item = *it;

        const AddresseeLineEditStatic::collectionInfo sourceIndex =
            s_static->akonadiCollectionToCompletionSourceMap.value(item.parentCollection().id(), AddresseeLineEditStatic::collectionInfo());
        if (sourceIndex.index >= 0) {
            qCDebug(LIBKDEPIM_LOG) << "identified collection: " << s_static->completionSources[sourceIndex.index];
            if (sourceIndex.enabled) {
                q->addItem(item, 1, sourceIndex.index);
            }

            // remove from the pending
            it = s_static->akonadiPendingItems.erase(it);
        } else {
            ++it;
        }
    }
}

void AddresseeLineEditPrivate::doCompletion(bool ctrlT)
{
    m_lastSearchMode = ctrlT;

    const KCompletion::CompletionMode mode = q->completionMode();

    if (mode == KCompletion::CompletionNone) {
        return;
    }

    s_static->completion->setOrder(KCompletion::Weighted);

    // cursor at end of string - or Ctrl+T pressed for substring completion?
    if (ctrlT) {
        const QStringList completions = adjustedCompletionItems(false);

        if (completions.count() > 1) {
            ; //m_previousAddresses = prevAddr;
        } else if (completions.count() == 1) {
            q->setText(m_previousAddresses + completions.first().trimmed());
        }

        // Make sure the completion popup is closed if no matching items were found
        setCompletedItems(completions, true);

        q->cursorAtEnd();
        q->setCompletionMode(mode);   //set back to previous mode
        return;
    }

    switch (mode) {
    case KCompletion::CompletionPopupAuto: {
        if (m_searchString.isEmpty()) {
            break;
        }
        //else: fall-through to the CompletionPopup case
    }

    case KCompletion::CompletionPopup: {
        const QStringList items = adjustedCompletionItems(false);
        setCompletedItems(items, false);
    }
    break;

    case KCompletion::CompletionShell: {
        const QString match = s_static->completion->makeCompletion(m_searchString);
        if (!match.isNull() && match != m_searchString) {
            q->setText(m_previousAddresses + match);
            q->setModified(true);
            q->cursorAtEnd();
        }
    }
    break;

    case KCompletion::CompletionMan: // Short-Auto in fact
    case KCompletion::CompletionAuto: {
        //force autoSuggest in KLineEdit::keyPressed or setCompletedText will have no effect
        q->setCompletionMode(q->completionMode());

        if (!m_searchString.isEmpty()) {

            //if only our \" is left, remove it since user has not typed it either
            if (m_searchExtended && m_searchString == QLatin1String("\"")) {
                m_searchExtended = false;
                m_searchString.clear();
                q->setText(m_previousAddresses);
                break;
            }

            QString match = s_static->completion->makeCompletion(m_searchString);

            if (!match.isEmpty()) {
                if (match != m_searchString) {
                    QString adds = m_previousAddresses + match;
                    q->callSetCompletedText(adds);
                }
            } else {
                if (!m_searchString.startsWith(QLatin1Char('\"'))) {
                    //try with quoted text, if user has not type one already
                    match = s_static->completion->makeCompletion(QLatin1String("\"") + m_searchString);
                    if (!match.isEmpty() && match != m_searchString) {
                        m_searchString = QLatin1String("\"") + m_searchString;
                        m_searchExtended = true;
                        q->setText(m_previousAddresses + m_searchString);
                        q->callSetCompletedText(m_previousAddresses + match);
                    }
                } else if (m_searchExtended) {
                    //our added \" does not work anymore, remove it
                    m_searchString = m_searchString.mid(1);
                    m_searchExtended = false;
                    q->setText(m_previousAddresses + m_searchString);
                    //now try again
                    match = s_static->completion->makeCompletion(m_searchString);
                    if (!match.isEmpty() && match != m_searchString) {
                        const QString adds = m_previousAddresses + match;
                        q->setCompletedText(adds);
                    }
                }
            }
        }
    }
    break;

    case KCompletion::CompletionNone:
    default: // fall through
        break;
    }
}

void AddresseeLineEditPrivate::slotCompletion()
{
    // Called by KLineEdit's keyPressEvent for CompletionModes
    // Auto,Popup -> new text, update search string.
    // not called for CompletionShell, this is been taken care of
    // in AddresseeLineEdit::keyPressEvent

    updateSearchString();
    if (q->completionBox()) {
        q->completionBox()->setCancelledText(m_searchString);
    }

    startSearches();
    doCompletion(false);
}

void AddresseeLineEditPrivate::slotPopupCompletion(const QString &completion)
{
    QString c = completion.trimmed();
    if (c.endsWith(QLatin1Char(')'))) {
        c = completion.mid(0, completion.lastIndexOf(QLatin1String(" ("))).trimmed();
    }
    q->setText(m_previousAddresses + c);
    q->cursorAtEnd();
    updateSearchString();
    q->emitTextCompleted();
}

void AddresseeLineEditPrivate::slotReturnPressed(const QString &)
{
    if (!q->completionBox()->selectedItems().isEmpty()) {
        slotPopupCompletion(q->completionBox()->selectedItems().first()->text());
    }
}

void AddresseeLineEditPrivate::slotStartLDAPLookup()
{
    if (s_networkConfigMgr->isOnline()) {

        const KCompletion::CompletionMode mode = q->completionMode();
        if (mode == KCompletion::CompletionNone) {
            return;
        }
        if (!s_static->ldapSearch->isAvailable()) {
            return;
        }
        if (s_static->ldapLineEdit != q) {
            return;
        }
        startLoadingLDAPEntries();
    }
}

void AddresseeLineEditPrivate::slotLDAPSearchData(const KLDAP::LdapResult::List &results)
{
    if (results.isEmpty() || s_static->ldapLineEdit != q) {
        return;
    }

    foreach (const KLDAP::LdapResult &result, results) {
        KContacts::Addressee contact;
        contact.setNameFromString(result.name);
        contact.setEmails(result.email);
        QString ou;

        if (m_showOU) {
            const int depth = result.dn.depth();
            for (int i = 0; i < depth; ++i) {
                const QString rdnStr = result.dn.rdnString(i);
                if (rdnStr.startsWith(QStringLiteral("ou="), Qt::CaseInsensitive)) {
                    ou = rdnStr.mid(3);
                    break;
                }
            }
        }

        if (!s_static->ldapClientToCompletionSourceMap.contains(result.clientNumber)) {
            s_static->updateLDAPWeights(); // we got results from a new source, so update the completion sources
        }

        q->addContact(contact, result.completionWeight,
                      s_static->ldapClientToCompletionSourceMap[ result.clientNumber ], ou);
    }

    if ((q->hasFocus() || q->completionBox()->hasFocus()) &&
            q->completionMode() != KCompletion::CompletionNone &&
            q->completionMode() != KCompletion::CompletionShell) {
        q->setText(m_previousAddresses + m_searchString);
        // only complete again if the user didn't change the selection while
        // we were waiting; otherwise the completion box will be closed
        const QListWidgetItem *current = q->completionBox()->currentItem();
        if (!current || m_searchString.trimmed() != current->text().trimmed()) {
            doCompletion(m_lastSearchMode);
        }
    }
}

void AddresseeLineEditPrivate::slotEditCompletionOrder()
{
    init(); // for s_static->ldapSearch
    if (m_useCompletion) {
        s_static->slotEditCompletionOrder();
    }
}

KLDAP::LdapClientSearch *AddresseeLineEditPrivate::ldapSearch()
{
    init(); // for s_static->ldapSearch
    return s_static->ldapSearch;
}

void AddresseeLineEditPrivate::slotUserCancelled(const QString &cancelText)
{
    if (s_static->ldapSearch && s_static->ldapLineEdit == q) {
        stopLDAPLookup();
    }

    q->callUserCancelled(m_previousAddresses + cancelText);   // in KLineEdit
}

void AddresseeLineEditPrivate::slotAkonadiHandleItems(const Akonadi::Item::List &items)
{
    /* We have to fetch the collections of the items, so that
       the source name can be correctly labeled.*/
    foreach (const Akonadi::Item &item, items) {

        // check the local cache of collections
        const AddresseeLineEditStatic::collectionInfo sourceIndex =
            s_static->akonadiCollectionToCompletionSourceMap.value(item.parentCollection().id(), AddresseeLineEditStatic::collectionInfo());
        if (sourceIndex.index == -1) {
            qCDebug(LIBKDEPIM_LOG) << "Fetching New collection: " << item.parentCollection().id();
            // the collection isn't there, start the fetch job.
            Akonadi::CollectionFetchJob *collectionJob =
                new Akonadi::CollectionFetchJob(item.parentCollection(),
                                                Akonadi::CollectionFetchJob::Base,
                                                s_static->akonadiSession);
            connect(collectionJob, &Akonadi::CollectionFetchJob::collectionsReceived,
                    this, &AddresseeLineEditPrivate::slotAkonadiCollectionsReceived);
            /* we don't want to start multiple fetch jobs for the same collection,
            so insert the collection with an index value of -2 */
            AddresseeLineEditStatic::collectionInfo info;
            info.index = -2;
            s_static->akonadiCollectionToCompletionSourceMap.insert(item.parentCollection().id(), info);
            s_static->akonadiPendingItems.append(item);
        } else if (sourceIndex.index == -2) {
            /* fetch job already started, don't need to start another one,
            so just append the item as pending */
            s_static->akonadiPendingItems.append(item);
        } else {
            if (sourceIndex.enabled) {
                q->addItem(item, 1, sourceIndex.index);
            }
        }
    }

    if (!items.isEmpty()) {
        const QListWidgetItem *current = q->completionBox()->currentItem();
        if (!current || m_searchString.trimmed() != current->text().trimmed()) {
            doCompletion(m_lastSearchMode);
        }
    }
}

void AddresseeLineEditPrivate::slotAkonadiSearchResult(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << "Akonadi search job failed: " << job->errorString();
    } else {
        Akonadi::ItemSearchJob *searchJob = static_cast<Akonadi::ItemSearchJob *>(job);
        qCDebug(LIBKDEPIM_LOG) << "Found" << searchJob->items().size() << "items";
    }
    const int index = s_static->akonadiJobsInFlight.indexOf(qobject_cast<Akonadi::Job *>(job));
    if (index != -1) {
        s_static->akonadiJobsInFlight.remove(index);
    }
}

void AddresseeLineEditPrivate::slotAkonadiCollectionsReceived(
    const Akonadi::Collection::List &collections)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimcompletionorder"));
    KConfigGroup groupCompletionWeights(config, "CompletionWeights");
    KConfigGroup groupCompletionEnabled(config, "CompletionEnabled");
    foreach (const Akonadi::Collection &collection, collections) {
        if (collection.isValid()) {
            const QString sourceString = collection.displayName();
            const int weight = groupCompletionWeights.readEntry(QString::number(collection.id()), 1);
            const int index = q->addCompletionSource(sourceString, weight);
            AddresseeLineEditStatic::collectionInfo info;
            info.index = index;
            info.enabled = groupCompletionEnabled.readEntry(QString::number(collection.id()), true);
            qCDebug(LIBKDEPIM_LOG) << "\treceived: " << sourceString << "index: " << index;
            s_static->akonadiCollectionToCompletionSourceMap.insert(collection.id(), info);
        }
    }

    // now that we have added the new collections, recheck our list of pending contacts
    akonadiHandlePending();
    // do completion
    const QListWidgetItem *current = q->completionBox()->currentItem();
    if (!current || m_searchString.trimmed() != current->text().trimmed()) {
        doCompletion(m_lastSearchMode);
    }
}

void AddresseeLineEditPrivate::slotShowOUChanged(bool checked)
{
    if (checked != m_showOU) {
        KConfigGroup group(KSharedConfig::openConfig(), "AddressLineEdit");
        group.writeEntry("ShowOU", checked);
        m_showOU = checked;
    }
}

void AddresseeLineEditPrivate::updateBalooBlackList()
{
    loadBalooBlackList();
    q->removeCompletionSource(i18nc("@title:group", "Contacts found in your data"));
    s_static->balooCompletionSource = q->addCompletionSource(i18nc("@title:group", "Contacts found in your data"), -1);
}

void AddresseeLineEditPrivate::updateCompletionOrder()
{
    s_static->updateCompletionOrder();
}

void AddresseeLineEditPrivate::slotConfigureBalooBlackList()
{
    QPointer<KPIM::BlackListBalooEmailCompletionDialog> dlg = new KPIM::BlackListBalooEmailCompletionDialog(q);
    dlg->setEmailBlackList(m_balooBlackList);
    if (dlg->exec()) {
        updateBalooBlackList();
    }
    delete dlg;
}

KConfig *AddresseeLineEditPrivate::recentAddressConfig() const
{
    return m_recentAddressConfig;
}

bool AddresseeLineEditPrivate::showRecentAddresses() const
{
    return mShowRecentAddresses;
}

void AddresseeLineEditPrivate::setRecentAddressConfig(KConfig *config)
{
    m_recentAddressConfig = config;
}

KContacts::ContactGroup::List AddresseeLineEditPrivate::groups() const
{
    return mGroups;
}

void AddresseeLineEditPrivate::setGroups(const KContacts::ContactGroup::List &groups)
{
    mGroups = groups;
}

QList<KJob *> AddresseeLineEditPrivate::mightBeGroupJobs() const
{
    return mMightBeGroupJobs;
}

void AddresseeLineEditPrivate::setMightBeGroupJobs(const QList<KJob *> &mightBeGroupJobs)
{
    mMightBeGroupJobs = mightBeGroupJobs;
}

bool AddresseeLineEditPrivate::autoGroupExpand() const
{
    return mAutoGroupExpand;
}

void AddresseeLineEditPrivate::setAutoGroupExpand(bool autoGroupExpand)
{
    mAutoGroupExpand = autoGroupExpand;
}

QStringList AddresseeLineEditPrivate::balooBlackList() const
{
    return m_balooBlackList;
}

void AddresseeLineEditPrivate::setExpandIntern(bool b)
{
    mExpandIntern = b;
}

bool AddresseeLineEditPrivate::expandIntern() const
{
    return mExpandIntern;
}

bool AddresseeLineEditPrivate::useSemicolonAsSeparator() const
{
    return m_useSemicolonAsSeparator;
}

void AddresseeLineEditPrivate::setUseSemicolonAsSeparator(bool useSemicolonAsSeparator)
{
    m_useSemicolonAsSeparator = useSemicolonAsSeparator;
}

bool AddresseeLineEditPrivate::enableBalooSearch() const
{
    return m_enableBalooSearch;
}

void AddresseeLineEditPrivate::setEnableBalooSearch(bool enableBalooSearch)
{
    m_enableBalooSearch = enableBalooSearch;
}

QString AddresseeLineEditPrivate::searchString() const
{
    return m_searchString;
}

void AddresseeLineEditPrivate::setSearchString(const QString &searchString)
{
    m_searchString = searchString;
}

bool AddresseeLineEditPrivate::searchExtended() const
{
    return m_searchExtended;
}

void AddresseeLineEditPrivate::setSearchExtended(bool searchExtended)
{
    m_searchExtended = searchExtended;
}

bool AddresseeLineEditPrivate::smartPaste() const
{
    return m_smartPaste;
}

void AddresseeLineEditPrivate::setSmartPaste(bool smartPaste)
{
    m_smartPaste = smartPaste;
}

bool AddresseeLineEditPrivate::completionInitialized() const
{
    return m_completionInitialized;
}

void AddresseeLineEditPrivate::setCompletionInitialized(bool completionInitialized)
{
    m_completionInitialized = completionInitialized;
}

bool AddresseeLineEditPrivate::useCompletion() const
{
    return m_useCompletion;
}

void AddresseeLineEditPrivate::setUseCompletion(bool useCompletion)
{
    m_useCompletion = useCompletion;
}

bool AddresseeLineEditPrivate::showOU() const
{
    return m_showOU;
}

void AddresseeLineEditPrivate::setShowOU(bool showOU)
{
    m_showOU = showOU;
}

void AddresseeLineEditPrivate::loadBalooBlackList()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    m_balooBlackList = group.readEntry("BalooBackList", QStringList());
    m_domainExcludeList = group.readEntry("ExcludeDomain", QStringList());
}

void AddresseeLineEditPrivate::removeCompletionSource(const QString &source)
{
    s_static->removeCompletionSource(source);
}

int AddresseeLineEditPrivate::addCompletionSource(const QString &source, int weight)
{
    return s_static->addCompletionSource(source, weight);
}

void AddresseeLineEditPrivate::mightBeGroupJobsClear()
{
    mMightBeGroupJobs.clear();
}

bool AddresseeLineEditPrivate::groupsIsEmpty() const
{
    return mGroups.isEmpty();
}

void AddresseeLineEditPrivate::setShowRecentAddresses(bool b)
{
    mShowRecentAddresses = b;
}

void AddresseeLineEditPrivate::groupsClear()
{
    mGroups.clear();
}

void AddresseeLineEditPrivate::addGroups(const KContacts::ContactGroup::List &lst)
{
    mGroups << lst;
}

void AddresseeLineEditPrivate::mightBeGroupJobsRemoveOne(Akonadi::ContactGroupSearchJob *search)
{
    mMightBeGroupJobs.removeOne(search);
}

void AddresseeLineEditPrivate::mightBeGroupJobsAdd(Akonadi::ContactGroupSearchJob *job)
{
    mMightBeGroupJobs.append(job);
}

}
