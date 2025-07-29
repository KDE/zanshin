/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    Ui::NameAndDataSourceDialog *ui;
    KDescendantsProxyModel *m_flattenProxy;
    QString m_name;
    Domain::DataSource::Ptr m_source;
};

}

#endif // WIDGETS_NAMEANDDATASOURCEDIALOG_H
