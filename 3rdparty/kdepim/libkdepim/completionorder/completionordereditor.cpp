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
#include "completionorderwidget.h"
#include <kdescendantsproxymodel.h>
#include "ldap/ldapclient.h"
#include "ldap/ldapclientsearch.h"
#include "ldap/ldapclientsearchconfig.h"

#include <kcontacts/addressee.h>
#include <kcontacts/contactgroup.h>
#include <kldap/ldapserver.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtDBus/QDBusConnection>
#include <QTreeWidget>
#include <KSharedConfig>
#include <QDialogButtonBox>

using namespace KPIM;

class KPIM::CompletionOrderEditorPrivate
{
public:
    CompletionOrderEditorPrivate()
        : mCompletionOrderWidget(Q_NULLPTR)
    {

    }

    CompletionOrderWidget *mCompletionOrderWidget;
};

CompletionOrderEditor::CompletionOrderEditor(KLDAP::LdapClientSearch *ldapSearch,
        QWidget *parent)
    : QDialog(parent), d(new KPIM::CompletionOrderEditorPrivate)
{
    setWindowTitle(i18n("Edit Completion Order"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CompletionOrderEditor::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CompletionOrderEditor::reject);
    okButton->setDefault(true);

    QWidget *page = new QWidget(this);
    QHBoxLayout *pageHBoxLayout = new QHBoxLayout(page);
    pageHBoxLayout->setMargin(0);

    d->mCompletionOrderWidget = new CompletionOrderWidget(this);
    d->mCompletionOrderWidget->setObjectName(QStringLiteral("completionorderwidget"));

    mainLayout->addWidget(d->mCompletionOrderWidget);
    mainLayout->addWidget(buttonBox);

    d->mCompletionOrderWidget->setLdapClientSearch(ldapSearch);

    d->mCompletionOrderWidget->loadCompletionItems();
    readConfig();
}

CompletionOrderEditor::~CompletionOrderEditor()
{
    writeConfig();
    delete d;
}

void CompletionOrderEditor::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "CompletionOrderEditor");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void CompletionOrderEditor::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "CompletionOrderEditor");
    group.writeEntry("Size", size());
    group.sync();
}

void CompletionOrderEditor::slotOk()
{
    d->mCompletionOrderWidget->save();
    accept();
}

