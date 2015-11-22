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


#ifndef WIDGETS_NEWPROJECTDIALOG_H
#define WIDGETS_NEWPROJECTDIALOG_H

#include <QDialog>

#include "widgets/newprojectdialoginterface.h"

class QModelIndex;
class KDescendantsProxyModel;

namespace Ui {
    class NewProjectDialog;
}

namespace Widgets {

class NewProjectDialog : public QDialog, public NewProjectDialogInterface
{
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget *parent = Q_NULLPTR);
    ~NewProjectDialog();

    int exec() Q_DECL_OVERRIDE;

    void accept() Q_DECL_OVERRIDE;

    void setDataSourcesModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    void setPageType(PageType type) Q_DECL_OVERRIDE;

    QString name() const Q_DECL_OVERRIDE;
    PageType pageType() const Q_DECL_OVERRIDE;
    Domain::DataSource::Ptr dataSource() const Q_DECL_OVERRIDE;

private slots:
    void onNameTextChanged(const QString &text);
    void onTypeIndexChanged(int index);

private:
    int indexOfType(NewProjectDialogInterface::PageType type);
    void applyDefaultSource(const QModelIndex &root);

    Ui::NewProjectDialog *ui;
    KDescendantsProxyModel *m_flattenProxy;
    QString m_name;
    PageType m_pageType;
    Domain::DataSource::Ptr m_source;
};

}

#endif // WIDGETS_NEWPROJECTDIALOG_H
