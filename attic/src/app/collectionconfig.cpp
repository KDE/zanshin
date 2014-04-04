/* This file is part of Zanshin Todo.

   Copyright 2013 Christian Mollekopf <chrigi_1@fastmail.fm>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/
#include "collectionconfig.h"
#include <core/pimitem.h>
#include <akonadi/akonadistoragesettings.h>
#include <Akonadi/EntityTreeView>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <KLocalizedString>
#include <kidentityproxymodel.h>
#include <QLabel>
#include <QVBoxLayout>
#include <KSettings/Dialog>

class SelectorModel: public KIdentityProxyModel
{
public:
    explicit SelectorModel(QObject* parent = 0): KIdentityProxyModel(parent){};
    
    virtual Qt::ItemFlags flags(const QModelIndex& /*index*/) const
    {
        return Qt::ItemIsUserCheckable|Qt::ItemIsEnabled;
    }

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const
    {
        if (role == Qt::CheckStateRole) {
            const Akonadi::Collection col = proxyIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (mSelected.contains(col)) {
                return Qt::Checked;
            }
            return Qt::Unchecked;
        }
        return KIdentityProxyModel::data(proxyIndex, role);
    }
    
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)
    {
        if (role == Qt::CheckStateRole) {
            const Akonadi::Collection col = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (col.isValid() && col.id() >= 0) {
                if (value.toBool()) {
                    mSelected.append(col);
                } else {
                    mSelected.removeAll(col);
                }
            }
            return true;
        }
        return KIdentityProxyModel::setData(index, value, role);
    }
    
    Akonadi::Collection::List mSelected;
};

CollectionConfig::CollectionConfig(QWidget* parent)
: QWidget(parent)
{
    setLayout(new QVBoxLayout(this));

    QLabel *description = new QLabel(this);
    description->setWordWrap(true);
    description->setText(i18n("Please select the collections you would like to have displayed."));
    layout()->addWidget(description);
 
    Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder(this);
    changeRecorder->fetchCollection(true);
    changeRecorder->setCollectionMonitored(Akonadi::Collection::root());
    changeRecorder->setMimeTypeMonitored(PimItem::mimeType(PimItem::Todo));
    changeRecorder->setMimeTypeMonitored(PimItem::mimeType(PimItem::Note));

    Akonadi::EntityTreeModel *model = new Akonadi::EntityTreeModel(changeRecorder, this);
    
    Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
    collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
    collectionsModel->setSourceModel(model);
    
    mSelectorModel = new SelectorModel(this);
    mSelectorModel->mSelected = Akonadi::StorageSettings::instance().activeCollections();
    mSelectorModel->setSourceModel(collectionsModel); 
    
    Akonadi::EntityTreeView *etv = new Akonadi::EntityTreeView(this);
    etv->setModel(mSelectorModel);
    layout()->addWidget(etv);
}

void CollectionConfig::accept()
{
    Akonadi::StorageSettings::instance().setActiveCollections(mSelectorModel->mSelected);
}

