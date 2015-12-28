/*
  This file is part of libkdepim.

  Copyright (c) 2002 Helge Deller <deller@gmx.de>
  Copyright (c) 2002 Lubos Lunak <llunak@suse.cz>
  Copyright (c) 2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
  Copyright (c) 2004 Daniel Molkentin <danimo@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "addresseelineedit.h"
#include "addresseelineedit_p.h"
#include "ldap/ldapclientsearch.h"

#include <Job>
#include <QRegularExpression>
#include <QUrl>
#include <kconfiggroup.h>

#include <KEmailAddress>
#include <KColorScheme>
#include <KJobWidgets>
#include <kdelibs4configmigrator.h>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <KContacts/VCardConverter>
#include <kcontacts/contactgrouptool.h>
#include <KIO/StoredTransferJob>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include "recentaddress/recentaddresses.h"
#include <KLDAP/LdapServer>

#include <KCompletionBox>
#include "libkdepim_debug.h"
#include <KLocalizedString>
#include <KStandardShortcut>
#include <KCodecs>

#include <QMimeData>
#include <QApplication>
#include <QObject>
#include <QBuffer>
#include <QRegExp>
#include <QEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QMenu>
#include <KSharedConfig>
#include <KMessageBox>
#include <ContactGroupExpandJob>
#include <kcontacts/vcarddrag.h>
#include "completionconfiguredialog/completionconfiguredialog.h"

using namespace KPIM;

inline bool itemIsHeader(const QListWidgetItem *item)
{
    return item && !item->text().startsWith(QStringLiteral("     "));
}

// needs to be unique, but the actual name doesn't matter much
static QString newLineEditObjectName()
{
    static int s_count = 0;
    QString name(QStringLiteral("KPIM::AddresseeLineEdit"));
    if (s_count++) {
        name += QLatin1Char('-');
        name += QString::number(s_count);
    }
    return name;
}

AddresseeLineEdit::AddresseeLineEdit(QWidget *parent, bool enableCompletion)
    : KLineEdit(parent), d(new AddresseeLineEditPrivate(this, enableCompletion))
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("addressline"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kpimbalooblacklist") << QStringLiteral("kpimcompletionorder"));
    migrate.migrate();

    setObjectName(newLineEditObjectName());
    setPlaceholderText(QString());

    d->init();
}

AddresseeLineEdit::~AddresseeLineEdit()
{
    delete d;
}

void AddresseeLineEdit::setFont(const QFont &font)
{
    KLineEdit::setFont(font);

    if (d->useCompletion()) {
        completionBox()->setFont(font);
    }
}

bool AddresseeLineEdit::expandIntern() const
{
    return d->expandIntern();
}

bool AddresseeLineEdit::groupsIsEmpty() const
{
    return d->groupsIsEmpty();
}

void AddresseeLineEdit::setExpandIntern(bool expand)
{
    d->setExpandIntern(expand);
}

bool AddresseeLineEdit::autoGroupExpand() const
{
    return d->autoGroupExpand();
}

void AddresseeLineEdit::setAutoGroupExpand(bool autoGroupExpand)
{
    d->setAutoGroupExpand(autoGroupExpand);
}

void AddresseeLineEdit::setEnableBalooSearch(bool enable)
{
    d->setEnableBalooSearch(enable);
}

void AddresseeLineEdit::allowSemicolonAsSeparator(bool useSemicolonAsSeparator)
{
    d->setUseSemicolonAsSeparator(useSemicolonAsSeparator);
}

bool AddresseeLineEdit::showRecentAddresses() const
{
    return d->showRecentAddresses();
}

void AddresseeLineEdit::setShowRecentAddresses(bool b)
{
    d->setShowRecentAddresses(b);
}

void AddresseeLineEdit::keyPressEvent(QKeyEvent *event)
{
    bool accept = false;

    const int key = event->key() | event->modifiers();

    if (KStandardShortcut::shortcut(KStandardShortcut::SubstringCompletion).contains(key)) {
        //TODO: add LDAP substring lookup, when it becomes available in KPIM::LDAPSearch
        d->updateSearchString();
        d->startSearches();
        d->doCompletion(true);
        accept = true;
    } else if (KStandardShortcut::shortcut(KStandardShortcut::TextCompletion).contains(key)) {
        const int len = text().length();

        if (len == cursorPosition()) {   // at End?
            d->updateSearchString();
            d->startSearches();
            d->doCompletion(true);
            accept = true;
        }
    }

    const QString oldContent = text();
    if (!accept) {
        KLineEdit::keyPressEvent(event);
    }

    // if the text didn't change (eg. because a cursor navigation key was pressed)
    // we don't need to trigger a new search
    if (oldContent == text()) {
        return;
    }

    if (event->isAccepted()) {
        d->updateSearchString();

        QString searchString(d->searchString());
        //LDAP does not know about our string manipulation, remove it
        if (d->searchExtended()) {
            searchString = d->searchString().mid(1);
        }

        d->restartTime(searchString);
    }
}

void AddresseeLineEdit::insert(const QString &t)
{
    if (!d->smartPaste()) {
        KLineEdit::insert(t);
        return;
    }

    QString newText = t.trimmed();
    if (newText.isEmpty()) {
        return;
    }

    // remove newlines in the to-be-pasted string
    QStringList lines = newText.split(QRegExp(QLatin1String("\r?\n")), QString::SkipEmptyParts);
    QStringList::iterator end(lines.end());
    for (QStringList::iterator it = lines.begin(); it != end; ++it) {
        // remove trailing commas and whitespace
        (*it).remove(QRegExp(QLatin1String(",?\\s*$")));
    }
    newText = lines.join(QStringLiteral(", "));

    if (newText.startsWith(QStringLiteral("mailto:"))) {
        const QUrl url(newText);
        newText = url.path();
    } else if (newText.indexOf(QStringLiteral(" at ")) != -1) {
        // Anti-spam stuff
        newText.replace(QStringLiteral(" at "), QStringLiteral("@"));
        newText.replace(QStringLiteral(" dot "), QStringLiteral("."));
    } else if (newText.indexOf(QStringLiteral("(at)")) != -1) {
        newText.replace(QRegularExpression(QStringLiteral("\\s*\\(at\\)\\s*")), QStringLiteral("@"));
    }

    QString contents = text();
    int start_sel = 0;
    int pos = cursorPosition();

    if (hasSelectedText()) {
        // Cut away the selection.
        start_sel = selectionStart();
        pos = start_sel;
        contents = contents.left(start_sel) + contents.mid(start_sel + selectedText().length());
    }

    int eot = contents.length();
    while ((eot > 0) && contents.at(eot - 1).isSpace()) {
        --eot;
    }
    if (eot == 0) {
        contents.clear();
    } else if (pos >= eot) {
        if (contents.at(eot - 1) == QLatin1Char(',')) {
            --eot;
        }
        contents.truncate(eot);
        contents += QStringLiteral(", ");
        pos = eot + 2;
    }

    contents = contents.left(pos) + newText + contents.mid(pos);
    setText(contents);
    setModified(true);
    setCursorPosition(pos + newText.length());
}

void AddresseeLineEdit::setText(const QString &text)
{
    const int cursorPos = cursorPosition();
    KLineEdit::setText(text.trimmed());
    setCursorPosition(cursorPos);
}

void AddresseeLineEdit::paste()
{
    if (d->useCompletion()) {
        d->setSmartPaste(true);
    }

    KLineEdit::paste();
    d->setSmartPaste(false);
}

void AddresseeLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    // reimplemented from QLineEdit::mouseReleaseEvent()
#ifndef QT_NO_CLIPBOARD
    if (d->useCompletion() &&
            QApplication::clipboard()->supportsSelection() &&
            !isReadOnly() &&
            event->button() == Qt::MidButton) {
        d->setSmartPaste(true);
    }
#endif

    KLineEdit::mouseReleaseEvent(event);
    d->setSmartPaste(false);
}

#ifndef QT_NO_DRAGANDDROP
void AddresseeLineEdit::dropEvent(QDropEvent *event)
{
    const QMimeData *md = event->mimeData();

    // Case one: The user dropped a text/directory (i.e. vcard), so decode its
    //           contents
    if (KContacts::VCardDrag::canDecode(md)) {
        KContacts::Addressee::List list;
        KContacts::VCardDrag::fromMimeData(md, list);

        KContacts::Addressee::List::ConstIterator ait;
        KContacts::Addressee::List::ConstIterator end(list.constEnd());
        for (ait = list.constBegin(); ait != end; ++ait) {
            insertEmails((*ait).emails());
        }
    }

    // Case two: The user dropped a list or Urls.
    // Iterate over that list. For mailto: Urls, just add the addressee to the list,
    // and for other Urls, download the Url and assume it points to a vCard
    else if (md->hasUrls()) {
        QList<QUrl> urls = md->urls();
        KContacts::Addressee::List list;

        foreach (const QUrl &url, urls) {

            // First, let's deal with mailto Urls. The path() part contains the
            // email-address.
            if (url.scheme() == QLatin1String("mailto")) {
                KContacts::Addressee addressee;
                addressee.insertEmail(KEmailAddress::decodeMailtoUrl(url), true /* preferred */);
                list += addressee;
            }

            // Otherwise, download the vCard to which the Url points
            else {
                KContacts::VCardConverter converter;
                auto job = KIO::storedGet(url);
                KJobWidgets::setWindow(job, parentWidget());
                if (job->exec()) {
                    QByteArray data = job->data();
                    list += converter.parseVCards(data);

                    if (list.isEmpty()) {  // try to parse a contact group
                        KContacts::ContactGroup group;
                        QBuffer dataStream(&data);
                        dataStream.open(QIODevice::ReadOnly);
                        QString error;
                        if (KContacts::ContactGroupTool::convertFromXml(&dataStream, group, &error)) {
                            Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob(group);
                            connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &AddresseeLineEdit::groupExpandResult);
                            expandJob->start();
                        }
                    }
                } else {
                    const QString caption(i18n("vCard Import Failed"));
                    const QString text = i18n("<qt>Unable to access <b>%1</b>.</qt>", url.url());
                    KMessageBox::error(parentWidget(), text, caption);
                }
            }
        }

        // Now, let the user choose which addressee to add.
        foreach (const KContacts::Addressee &addressee, list) {
            insertEmails(addressee.emails());
        }
    }

    // Case three: Let AddresseeLineEdit deal with the rest
    else {
        if (!isReadOnly()) {
            const QList<QUrl> uriList = event->mimeData()->urls();
            if (!uriList.isEmpty()) {
                QString contents = text();
                // remove trailing white space and comma
                int eot = contents.length();
                while ((eot > 0) && contents.at(eot - 1).isSpace()) {
                    --eot;
                }
                if (eot == 0) {
                    contents.clear();
                } else if (contents.at(eot - 1) == QLatin1Char(',')) {
                    --eot;
                    contents.truncate(eot);
                }
                bool mailtoURL = false;
                // append the mailto URLs
                foreach (const QUrl &url, uriList) {
                    if (url.scheme() == QLatin1String("mailto")) {
                        mailtoURL = true;
                        QString address;
                        address = QUrl::fromPercentEncoding(url.path().toLatin1());
                        address = KCodecs::decodeRFC2047String(address);
                        if (!contents.isEmpty()) {
                            contents.append(QStringLiteral(", "));
                        }
                        contents.append(address);
                    }
                }
                if (mailtoURL) {
                    setText(contents);
                    setModified(true);
                    return;
                }
            } else {
                // Let's see if this drop contains a comma separated list of emails
                const QMimeData *mimeData = event->mimeData();
                if (mimeData->hasText()) {
                    const QString dropData = mimeData->text();
                    const QStringList addrs = KEmailAddress::splitAddressList(dropData);
                    if (!addrs.isEmpty()) {
                        setText(KEmailAddress::normalizeAddressesAndDecodeIdn(dropData));
                        setModified(true);
                        return;
                    }
                }
            }
        }

        if (d->useCompletion()) {
            d->setSmartPaste(true);
        }

        QLineEdit::dropEvent(event);
        d->setSmartPaste(false);
    }
}
#endif // QT_NO_DRAGANDDROP

void AddresseeLineEdit::groupExpandResult(KJob *job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);

    if (!expandJob) {
        return;
    }

    const KContacts::Addressee::List contacts = expandJob->contacts();
    foreach (const KContacts::Addressee &addressee, contacts) {
        if (d->expandIntern() || text().isEmpty()) {
            insertEmails(QStringList() << addressee.fullEmail());
        } else {
            Q_EMIT addAddress(addressee.fullEmail());
        }
    }

    job->deleteLater();
}

void AddresseeLineEdit::insertEmails(const QStringList &emails)
{
    if (emails.empty()) {
        return;
    }

    QString contents = text();
    if (!contents.isEmpty()) {
        contents += QLatin1Char(',');
    }
    // only one address, don't need kpopup to choose
    if (emails.size() == 1) {
        setText(contents + emails.front());
        return;
    }
    //multiple emails, let the user choose one
    QMenu menu(this);
    menu.setTitle(i18n("Select email from contact"));
    menu.setObjectName(QStringLiteral("Addresschooser"));
    for (QStringList::const_iterator it = emails.constBegin(), end = emails.constEnd(); it != end; ++it) {
        menu.addAction(*it);
    }
    const QAction *result = menu.exec(QCursor::pos());
    if (!result) {
        return;
    }
    setText(contents + KLocalizedString::removeAcceleratorMarker(result->text()));
}

void AddresseeLineEdit::cursorAtEnd()
{
    setCursorPosition(text().length());
}

void AddresseeLineEdit::enableCompletion(bool enable)
{
    d->setUseCompletion(enable);
}

bool AddresseeLineEdit::isCompletionEnabled() const
{
    return d->useCompletion();
}

void AddresseeLineEdit::addItem(const Akonadi::Item &item, int weight, int source)
{
    //Let Akonadi results always have a higher weight than baloo results
    if (item.hasPayload<KContacts::Addressee>()) {
        addContact(item.payload<KContacts::Addressee>(), weight + 1, source);
    } else if (item.hasPayload<KContacts::ContactGroup>()) {
        addContactGroup(item.payload<KContacts::ContactGroup>(), weight + 1, source);
    }
}

void AddresseeLineEdit::addContactGroup(const KContacts::ContactGroup &group, int weight, int source)
{
    d->addCompletionItem(group.name(), weight, source);
}

void AddresseeLineEdit::addContact(const KContacts::Addressee &addr, int weight, int source, QString append)
{
    const QStringList emails = addr.emails();
    QStringList::ConstIterator it;
    int isPrefEmail = 1; //first in list is preferredEmail
    QStringList::ConstIterator end(emails.constEnd());
    for (it = emails.constBegin(); it != end; ++it) {
        //TODO: highlight preferredEmail
        const QString email((*it));
        const QString givenName = addr.givenName();
        const QString familyName = addr.familyName();
        const QString nickName  = addr.nickName();
        QString fullEmail       = addr.fullEmail(email);

        QString appendix;

        if (!append.isEmpty()) {
            appendix = QStringLiteral(" (%1)");
            append = append.replace(QStringLiteral("("), QStringLiteral("["));
            append = append.replace(QStringLiteral(")"), QStringLiteral("]"));
            appendix = appendix.arg(append);
        }

        // Prepare "givenName" + ' ' + "familyName"
        QString fullName = givenName;
        if (!familyName.isEmpty()) {
            if (!fullName.isEmpty()) {
                fullName += QLatin1Char(' ');
            }
            fullName += familyName;
        }

        // Finally, we can add the completion items
        if (!fullName.isEmpty()) {
            const QString address = KEmailAddress::normalizedAddress(fullName, email, QString());
            if (fullEmail != address) {
                // This happens when fullEmail contains a middle name, while our own fullName+email only has "first last".
                // Let's offer both, the fullEmail with 3 parts, looks a tad formal.
                d->addCompletionItem(address + appendix, weight + isPrefEmail, source);
            }
        }

        QStringList keyWords;
        if (!nickName.isEmpty()) {
            keyWords.append(nickName);
        }

        d->addCompletionItem(fullEmail + appendix, weight + isPrefEmail, source, &keyWords);

        isPrefEmail = 0;
    }
}

#ifndef QT_NO_CONTEXTMENU
void AddresseeLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    if (menu) {   // can be 0 on platforms with only a touch interface
        menu->exec(event->globalPos());
        delete menu;
    }
}

QMenu *AddresseeLineEdit::createStandardContextMenu()
{
    // disable modes not supported by KMailCompletion
    setCompletionModeDisabled(KCompletion::CompletionMan);
    setCompletionModeDisabled(KCompletion::CompletionPopupAuto);

    QMenu *menu = KLineEdit::createStandardContextMenu();
    if (!menu) {
        return Q_NULLPTR;
    }
    if (d->useCompletion()) {
        QAction *showOU = new QAction(i18n("Show Organization Unit for LDAP results"), menu);
        showOU->setCheckable(true);

        showOU->setChecked(d->showOU());
        connect(showOU, &QAction::triggered, d, &AddresseeLineEditPrivate::slotShowOUChanged);
        menu->addAction(showOU);
    }
    if (isCompletionEnabled()) {
        menu->addSeparator();
        QAction *act = menu->addAction(i18n("Configure Completion..."));
        connect(act, &QAction::triggered, this, &AddresseeLineEdit::configureCompletion);
    }
    menu->addSeparator();
    QAction *act = menu->addAction(i18n("Automatically expand groups"));
    act->setCheckable(true);
    act->setChecked(autoGroupExpand());
    connect(act, &QAction::triggered, this, &AddresseeLineEdit::slotToggleExpandGroups);

    if (!groupsIsEmpty()) {
        act = menu->addAction(i18n("Expand Groups..."));
        connect(act, &QAction::triggered, this, &AddresseeLineEdit::expandGroups);
    }
    return menu;
}
#endif

void AddresseeLineEdit::configureCompletion()
{
    QScopedPointer<KPIM::CompletionConfigureDialog> dlg(new KPIM::CompletionConfigureDialog(this));
    dlg->setRecentAddresses(KPIM::RecentAddresses::self(recentAddressConfig())->addresses());
    dlg->setLdapClientSearch(ldapSearch());
    dlg->setEmailBlackList(balooBlackList());
    dlg->load();
    if (dlg->exec() && dlg) {
        if (dlg->recentAddressWasChanged()) {
            KPIM::RecentAddresses::self(recentAddressConfig())->clear();
            dlg->storeAddresses(recentAddressConfig());
            loadContacts();
            updateBalooBlackList();
            updateCompletionOrder();
        }
    }
}

void AddresseeLineEdit::slotToggleExpandGroups()
{
    setAutoGroupExpand(!autoGroupExpand());
    KConfigGroup group(KSharedConfig::openConfig(), "AddressLineEdit");
    group.writeEntry("AutoGroupExpand", autoGroupExpand());
}

void AddresseeLineEdit::loadContacts()
{
    const QString recentAddressGroupName = i18n("Recent Addresses");
    if (showRecentAddresses()) {
        const QStringList recent =
            cleanupEmailList(KPIM::RecentAddresses::self(recentAddressConfig())->addresses());
        QStringList::ConstIterator it = recent.constBegin();
        QString name, email;

        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimcompletionorder"));
        KConfigGroup group(config, "CompletionWeights");
        const int weight = group.readEntry("Recent Addresses", 10);
        removeCompletionSource(recentAddressGroupName);
        const int idx = addCompletionSource(recentAddressGroupName, weight);

        QStringList::ConstIterator end = recent.constEnd();
        for (; it != end; ++it) {
            KContacts::Addressee addr;
            KEmailAddress::extractEmailAddressAndName(*it, email, name);
            name = KEmailAddress::quoteNameIfNecessary(name);
            if ((name[0] == QLatin1Char('"')) && (name[name.length() - 1] == QLatin1Char('"'))) {
                name.remove(0, 1);
                name.truncate(name.length() - 1);
            }
            addr.setNameFromString(name);
            addr.insertEmail(email, true);
            addContact(addr, weight, idx);
        }
    } else {
        removeCompletionSource(recentAddressGroupName);
    }
}

void KPIM::AddresseeLineEdit::removeCompletionSource(const QString &source)
{
    d->removeCompletionSource(source);
}

int KPIM::AddresseeLineEdit::addCompletionSource(const QString &source, int weight)
{
    return d->addCompletionSource(source, weight);
}

bool KPIM::AddresseeLineEdit::eventFilter(QObject *object, QEvent *event)
{
    if (d->completionInitialized() &&
            (object == completionBox() ||
             completionBox()->findChild<QWidget *>(object->objectName()) == object)) {
        if (event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseMove ||
                event->type() == QEvent::MouseButtonRelease ||
                event->type() == QEvent::MouseButtonDblClick) {

            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            // find list box item at the event position
            QListWidgetItem *item = completionBox()->itemAt(mouseEvent->pos());
            if (!item) {
                // In the case of a mouse move outside of the box we don't want
                // the parent to fuzzy select a header by mistake.
                const bool eat = event->type() == QEvent::MouseMove;
                return eat;
            }
            // avoid selection of headers on button press, or move or release while
            // a button is pressed
            const Qt::MouseButtons buttons = mouseEvent->buttons();
            if (event->type() == QEvent::MouseButtonPress ||
                    event->type() == QEvent::MouseButtonDblClick ||
                    buttons & Qt::LeftButton || buttons & Qt::MidButton ||
                    buttons & Qt::RightButton) {
                if (itemIsHeader(item)) {
                    return true; // eat the event, we don't want anything to happen
                } else {
                    // if we are not on one of the group heading, make sure the item
                    // below or above is selected, not the heading, inadvertedly, due
                    // to fuzzy auto-selection from QListBox
                    completionBox()->setCurrentItem(item);
                    item->setSelected(true);
                    if (event->type() == QEvent::MouseMove) {
                        return true; // avoid fuzzy selection behavior
                    }
                }
            }
        }
    }

    if ((object == this) &&
            (event->type() == QEvent::ShortcutOverride)) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down ||
                keyEvent->key() == Qt::Key_Tab) {
            keyEvent->accept();
            return true;
        }
    }

    if ((object == this) &&
            (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) &&
            completionBox()->isVisible()) {
        const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int currentIndex = completionBox()->currentRow();
        if (currentIndex < 0) {
            return true;
        }
        if (keyEvent->key() == Qt::Key_Up) {
            //qCDebug(LIBKDEPIM_LOG) <<"EVENTFILTER: Qt::Key_Up currentIndex=" << currentIndex;
            // figure out if the item we would be moving to is one we want
            // to ignore. If so, go one further
            const QListWidgetItem *itemAbove = completionBox()->item(currentIndex);
            if (itemAbove && itemIsHeader(itemAbove)) {
                // there is a header above is, check if there is even further up
                // and if so go one up, so it'll be selected
                if (currentIndex > 0 && completionBox()->item(currentIndex - 1)) {
                    //qCDebug(LIBKDEPIM_LOG) <<"EVENTFILTER: Qt::Key_Up -> skipping" << currentIndex - 1;
                    completionBox()->setCurrentRow(currentIndex - 1);
                    completionBox()->item(currentIndex - 1)->setSelected(true);
                } else if (currentIndex == 0) {
                    // nothing to skip to, let's stay where we are, but make sure the
                    // first header becomes visible, if we are the first real entry
                    completionBox()->scrollToItem(completionBox()->item(0));
                    QListWidgetItem *item = completionBox()->item(currentIndex);
                    if (item) {
                        if (itemIsHeader(item)) {
                            currentIndex++;
                            item = completionBox()->item(currentIndex);
                        }
                        completionBox()->setCurrentItem(item);
                        item->setSelected(true);
                    }
                }

                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Down) {
            // same strategy for downwards
            //qCDebug(LIBKDEPIM_LOG) <<"EVENTFILTER: Qt::Key_Down. currentIndex=" << currentIndex;
            const QListWidgetItem *itemBelow = completionBox()->item(currentIndex);
            if (itemBelow && itemIsHeader(itemBelow)) {
                if (completionBox()->item(currentIndex + 1)) {
                    //qCDebug(LIBKDEPIM_LOG) <<"EVENTFILTER: Qt::Key_Down -> skipping" << currentIndex+1;
                    completionBox()->setCurrentRow(currentIndex + 1);
                    completionBox()->item(currentIndex + 1)->setSelected(true);
                } else {
                    // nothing to skip to, let's stay where we are
                    QListWidgetItem *item = completionBox()->item(currentIndex);
                    if (item) {
                        completionBox()->setCurrentItem(item);
                        item->setSelected(true);
                    }
                }

                return true;
            }
            // special case of the initial selection, which is unfortunately a header.
            // Setting it to selected tricks KCompletionBox into not treating is special
            // and selecting making it current, instead of the one below.
            QListWidgetItem *item = completionBox()->item(currentIndex);
            if (item && itemIsHeader(item)) {
                completionBox()->setCurrentItem(item);
                item->setSelected(true);
            }
        } else if (event->type() == QEvent::KeyRelease &&
                   (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab)) {
            /// first, find the header of the current section
            QListWidgetItem *myHeader = Q_NULLPTR;
            int myHeaderIndex = -1;
            const int iterationStep = keyEvent->key() == Qt::Key_Tab ?  1 : -1;
            int index = qMin(qMax(currentIndex - iterationStep, 0), completionBox()->count() - 1);
            while (index >= 0) {
                if (itemIsHeader(completionBox()->item(index))) {
                    myHeader = completionBox()->item(index);
                    myHeaderIndex = index;
                    break;
                }

                index--;
            }
            Q_ASSERT(myHeader);   // we should always be able to find a header

            // find the next header (searching backwards, for Qt::Key_Backtab)
            QListWidgetItem *nextHeader = Q_NULLPTR;

            // when iterating forward, start at the currentindex, when backwards,
            // one up from our header, or at the end
            uint j;
            if (keyEvent->key() == Qt::Key_Tab) {
                j = currentIndex;
            } else {
                index = myHeaderIndex;
                if (index == 0) {
                    j = completionBox()->count() - 1;
                } else {
                    j = (index - 1) % completionBox()->count();
                }
            }
            while ((nextHeader = completionBox()->item(j)) && nextHeader != myHeader) {
                if (itemIsHeader(nextHeader)) {
                    break;
                }
                j = (j + iterationStep) % completionBox()->count();
            }

            if (nextHeader && nextHeader != myHeader) {
                QListWidgetItem *item = completionBox()->item(j + 1);
                if (item && !itemIsHeader(item)) {
                    completionBox()->setCurrentItem(item);
                    item->setSelected(true);
                }
            }

            return true;
        }
    }

    return KLineEdit::eventFilter(object, event);
}

void AddresseeLineEdit::emitTextCompleted()
{
    Q_EMIT textCompleted();
}

void AddresseeLineEdit::callUserCancelled(const QString &str)
{
    userCancelled(str);
}

void AddresseeLineEdit::callSetCompletedText(const QString &text, bool marked)
{
    setCompletedText(text, marked);
}

void AddresseeLineEdit::callSetCompletedText(const QString &text)
{
    setCompletedText(text);
}

void AddresseeLineEdit::callSetUserSelection(bool b)
{
    setUserSelection(b);
}

void AddresseeLineEdit::updateBalooBlackList()
{
    d->updateBalooBlackList();
}

void AddresseeLineEdit::updateCompletionOrder()
{
    d->updateCompletionOrder();
}

KLDAP::LdapClientSearch *AddresseeLineEdit::ldapSearch() const
{
    return d->ldapSearch();
}

QStringList AddresseeLineEdit::balooBlackList() const
{
    return d->balooBlackList();
}

void AddresseeLineEdit::slotEditingFinished()
{
    foreach (KJob *job, d->mightBeGroupJobs()) {
        disconnect(job);
        job->deleteLater();
    }

    d->mightBeGroupJobsClear();
    d->groupsClear();

    if (!text().isEmpty()) {
        const QStringList addresses = KEmailAddress::splitAddressList(text());
        Q_FOREACH (const QString &address, addresses) {
            Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
            connect(job, &Akonadi::ContactGroupSearchJob::result, this, &AddresseeLineEdit::slotGroupSearchResult);
            d->mightBeGroupJobsAdd(job);
            job->setQuery(Akonadi::ContactGroupSearchJob::Name, address);
        }
    }
}

void AddresseeLineEdit::slotGroupSearchResult(KJob *job)
{
    Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob *>(job);

    // Laurent I don't understand why Akonadi::ContactGroupSearchJob send two "result(...)" signal. For the moment
    // avoid to go in this method twice, until I understand it.
    if (!d->mightBeGroupJobs().contains(searchJob)) {
        return;
    }
    //Q_ASSERT(d->mMightBeGroupJobs.contains(searchJob));
    d->mightBeGroupJobsRemoveOne(searchJob);

    const KContacts::ContactGroup::List contactGroups = searchJob->contactGroups();
    if (contactGroups.isEmpty()) {
        return; // Nothing todo, probably a normal email address was entered
    }

    d->addGroups(contactGroups);
    searchJob->deleteLater();

    if (autoGroupExpand()) {
        expandGroups();
    }
}

void AddresseeLineEdit::expandGroups()
{
    QStringList addresses = KEmailAddress::splitAddressList(text());

    foreach (const KContacts::ContactGroup &group, d->groups()) {
        Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob(group);
        connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &AddresseeLineEdit::groupExpandResult);
        addresses.removeAll(group.name());
        expandJob->start();
    }
    setText(addresses.join(QStringLiteral(", ")));
    d->groupsClear();
}

void AddresseeLineEdit::setRecentAddressConfig(KConfig *config)
{
    d->setRecentAddressConfig(config);
}

KConfig *AddresseeLineEdit::recentAddressConfig() const
{
    return d->recentAddressConfig();
}
