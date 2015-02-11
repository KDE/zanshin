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


#ifndef WIDGETS_NEWPAGEDIALOG_H
#define WIDGETS_NEWPAGEDIALOG_H

#include <QDialog>

#include "widgets/newpagedialoginterface.h"

namespace Ui {
    class NewPageDialog;
}

namespace Widgets {

class NewPageDialog : public QDialog, public NewPageDialogInterface
{
    Q_OBJECT
public:
    explicit NewPageDialog(QWidget *parent = Q_NULLPTR);
    ~NewPageDialog();

    int exec();

    void accept();

    void setDataSourcesModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    void setDefaultSource(const Domain::DataSource::Ptr &source) Q_DECL_OVERRIDE;
    void setPageType(PageType type) Q_DECL_OVERRIDE;

    QString name() const Q_DECL_OVERRIDE;
    PageType pageType() const Q_DECL_OVERRIDE;
    Domain::DataSource::Ptr dataSource() const Q_DECL_OVERRIDE;

private slots:
    void onNameTextChanged(const QString &text);
    void onTypeIndexChanged(int index);

private:
    Ui::NewPageDialog *ui;
    QString m_name;
    PageType m_pageType;
    Domain::DataSource::Ptr m_source;

    int indexOfType(NewPageDialogInterface::PageType type);
};

}

#endif // WIDGETS_NEWPAGEDIALOG_H
