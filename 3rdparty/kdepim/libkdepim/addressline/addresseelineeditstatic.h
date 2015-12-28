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

#ifndef ADDRESSEELINEEDITSTATIC_H
#define ADDRESSEELINEEDITSTATIC_H

#include <QMap>
#include <AkonadiCore/Collection>

#include <AkonadiCore/Item>
#include <AkonadiCore/Session>
#include <QVector>
#include <QWeakPointer>
#include <ldap/ldapclientsearch.h>
#include <qstringlist.h>
class QTimer;
namespace KPIM
{
class KMailCompletion;
class AddresseeLineEdit;
class AddresseeLineEditStatic
{
public:
    typedef QMap< QString, QPair<int, int> > CompletionItemsMap;

    AddresseeLineEditStatic();

    ~AddresseeLineEditStatic();

    void slotEditCompletionOrder();

    void updateCollectionWeights();

    void updateLDAPWeights();
    void updateCompletionOrder();

    int addCompletionSource(const QString &source, int weight);

    void removeCompletionSource(const QString &source);

    KMailCompletion *completion;
    CompletionItemsMap completionItemMap;
    QStringList completionSources;

    QTimer *ldapTimer;
    KLDAP::LdapClientSearch *ldapSearch;

    QString ldapText;
    AddresseeLineEdit *ldapLineEdit;
    // The weights associated with the completion sources in s_static->completionSources.
    // Both are maintained by addCompletionSource(), don't attempt to modifiy those yourself.
    QMap<QString, int> completionSourceWeights;
    // maps LDAP client indices to completion source indices
    // the assumption that they are always the first n indices in s_static->completion
    // does not hold when clients are added later on
    QMap<int, int> ldapClientToCompletionSourceMap;
    // holds the cached mapping from akonadi collection id to the completion source index
    struct collectionInfo {
        collectionInfo()
            : index(-1),
              enabled(true)
        {

        }
        int index;
        bool enabled;
    };

    QMap<Akonadi::Collection::Id, collectionInfo> akonadiCollectionToCompletionSourceMap;
    // a list of akonadi items (contacts) that have not had their collection fetched yet
    Akonadi::Item::List akonadiPendingItems;
    Akonadi::Session *akonadiSession;
    QVector<QWeakPointer<Akonadi::Job> > akonadiJobsInFlight;
    int balooCompletionSource;
};
}
#endif // ADDRESSEELINEEDITSTATIC_H
