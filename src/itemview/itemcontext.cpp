/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "itemcontext.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDropEvent>
#include <QListView>
#include <Nepomuk/Utils/SimpleResourceModel>
#include <Nepomuk/Utils/SearchWidget>
#include <Nepomuk/File>
#include <Nepomuk/ResourceManager>
#include <Akonadi/Item>
#include <KDebug>
#include "abstractpimitem.h"
#include "nepomukcontextmodel.h"
#include "nepomukcontextview.h"

#include <Nepomuk/Utils/FacetWidget>
#include <Nepomuk/Utils/SimpleFacet>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Vocabulary/NFO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Variant>

#include "guiclient.h"
#include <KActionCollection>
#include <KAction>
#include <KRun>

ItemContext::ItemContext(QWidget* parent)
: QWidget(parent)
{
    GuiClient *client = new GuiClient("contextviewui.rc", this);

    KActionCollection *actionCollection = client->actionCollection();
    KAction *action;

    action = actionCollection->addAction( "file_open" );
    action->setText( i18n( "&Open File" ) );
    action->setIcon( KIcon( "go-down" ) );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N) );
    connect( action, SIGNAL(triggered()), this, SLOT(openSelection()) );

    action = actionCollection->addAction( "context_remove" );
    action->setText( i18n( "&Remove from Context" ) );
    action->setIcon( KIcon( "go-down" ) );
    action->setShortcut( QKeySequence( Qt::Key_Delete) );
    connect( action, SIGNAL(triggered()), this, SLOT(removeSelection()) );

    setAcceptDrops(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_model = new NepomukContextModel(this);
    m_contextView = new NepomukContextView(client, this);
    client->setupActions(m_contextView);
    m_contextView->setModel(m_model);
    //view->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    layout->addWidget(m_contextView);

/*    Nepomuk::Utils::SearchWidget *search = new Nepomuk::Utils::SearchWidget(this);
    layout->addWidget(search);
*/
    /*Nepomuk::Utils::FacetWidget *facetWidget = new Nepomuk::Utils::FacetWidget(this);
    Nepomuk::Utils::SimpleFacet* imageSizeFacet = new Nepomuk::Utils::SimpleFacet();
    imageSizeFacet->addTerm( i18n("Small"), Nepomuk::Vocabulary::NFO::width() <= Nepomuk::Query::LiteralTerm(300));
    imageSizeFacet->addTerm( i18n("Medium"), (Nepomuk::Vocabulary::NFO::width() > Nepomuk::Query::LiteralTerm(300)) &&
    (Nepomuk::Vocabulary::NFO::width() <= Nepomuk::Query::LiteralTerm(800)));
    imageSizeFacet->addTerm( i18n("Large"), Nepomuk::Vocabulary::NFO::width() > Nepomuk::Query::LiteralTerm(800));
    facetWidget->addFacet(imageSizeFacet);
    layout->addWidget(facetWidget);*/

    setLayout(layout);
    setMinimumSize(0, 30);
    //setBackgroundRole(QPalette::Base);
    //setAutoFillBackground(true);

}


void ItemContext::setResource(const Nepomuk::Resource &resource)
{
    m_resource = resource;
    updateContext();
}

void ItemContext::addContext(const KUrl& uri)
{
    kDebug() << "added context: " << uri;
    if (uri.isLocalFile()) {
        //TODO add option to copy file to a repository managed by notetaker
        Nepomuk::File file = Nepomuk::File(uri, Nepomuk::ResourceManager::instance());
        //FIXME this should happen in the Nepomuk::File class
        file.setProperty(Nepomuk::Vocabulary::NIE::url(), Nepomuk::Variant(uri.url()));
        //TODO actually the file thing should be added as relation not the file itself. Also the file should be
        m_resource.addIsRelated(file);
    } else {
        Akonadi::Item item = Akonadi::Item::fromUrl(uri);
        if (item.isValid()) {
            kDebug() << "item is valid";
            //The resource should already be available from the nepomukrunner
            //We do not create a new resource because for this we would need to fetch the akonadi item first (for the mimetype)
            Nepomuk::Resource res(uri);
            if (res.exists()) {
                kDebug() << "resource is existing";
                m_resource.addIsRelated(res.pimoThing());
            }
            //TODO find correct resource to add (maybe the one created by
        }
    }
    updateContext();
}

void ItemContext::updateContext()
{
    m_model->clear();
    m_model->addResources(m_resource.isRelateds());
}

void ItemContext::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
    kDebug() << event->proposedAction();
    //QWidget::dragEnterEvent();
}

void ItemContext::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        foreach (const QUrl &url, event->mimeData()->urls()) {
            addContext(KUrl(url));
        }
    }
    event->acceptProposedAction();
}

void ItemContext::openSelection()
{
    foreach (const Nepomuk::Resource &res, m_contextView->selectedResources()) {
        if (res.isFile()) {
            Nepomuk::File file = Nepomuk::File(res);
            KRun *run = new KRun(file.property(Nepomuk::Vocabulary::NIE::url()).toUrl(), this);
        } else {
            emit itemActivated(res);
        }

    }
}

void ItemContext::removeSelection()
{
    foreach (const Nepomuk::Resource &res, m_contextView->selectedResources()) {
        m_resource.removeProperty( Nepomuk::Resource::isRelatedUri(), res);
    }
    updateContext();
}

