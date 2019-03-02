/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef WIDGETS_NAMEANDDATASOURCEDIALOG_H
#define WIDGETS_NAMEANDDATASOURCEDIALOG_H

#include <QDialog>

#include "widgets/nameanddatasourcedialoginterface.h"

class QModelIndex;
class KDescendantsProxyModel;

namespace Ui {
    class NameAndDataSourceDialog;
}

namespace Widgets {

class NameAndDataSourceDialog : public QDialog, public NameAndDataSourceDialogInterface
{
    Q_OBJECT
public:
    explicit NameAndDataSourceDialog(QWidget *parent = nullptr);
    ~NameAndDataSourceDialog();

    int exec() override;

    void accept() override;

    void setWindowTitle(const QString &title) override;
    void setDataSourcesModel(QAbstractItemModel *model) override;

    QString name() const override;
    Domain::DataSource::Ptr dataSource() const override;

private slots:
    void onUserInputChanged();

private:
    void applyDefaultSource(const QModelIndex &root);

    Ui::NameAndDataSourceDialog *ui;
    KDescendantsProxyModel *m_flattenProxy;
    QString m_name;
    Domain::DataSource::Ptr m_source;
};

}

#endif // WIDGETS_NAMEANDDATASOURCEDIALOG_H
