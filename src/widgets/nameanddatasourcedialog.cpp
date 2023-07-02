/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "nameanddatasourcedialog.h"

#include "ui_nameanddatasourcedialog.h"

#include <QPushButton>
#include <QSortFilterProxyModel>
#include <KDescendantsProxyModel>

#include "presentation/querytreemodelbase.h"

using namespace Widgets;

class TaskSourceProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TaskSourceProxy(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override
    {
        auto sourceIndex = sourceModel()->index(sourceRow, 0);
        auto source = sourceIndex.data(Presentation::QueryTreeModelBase::ObjectRole)
                                 .value<Domain::DataSource::Ptr>();
        return source && (source->contentTypes() & Domain::DataSource::Tasks);
    }
};

NameAndDataSourceDialog::NameAndDataSourceDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::NameAndDataSourceDialog),
      m_flattenProxy(new KDescendantsProxyModel(this))
{
    ui->setupUi(this);

    connect(ui->nameEdit, &QLineEdit::textChanged, this, &NameAndDataSourceDialog::onUserInputChanged);

    auto taskSourceProxy = new TaskSourceProxy(this);
    taskSourceProxy->setSourceModel(m_flattenProxy);
    ui->sourceCombo->setModel(taskSourceProxy);
    m_flattenProxy->setDisplayAncestorData(true);
    connect(ui->sourceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &NameAndDataSourceDialog::onUserInputChanged);

    onUserInputChanged();
}

NameAndDataSourceDialog::~NameAndDataSourceDialog()
{
    delete ui;
}

int NameAndDataSourceDialog::exec()
{
    return QDialog::exec();
}

void NameAndDataSourceDialog::accept()
{
    m_name = ui->nameEdit->text();
    m_source = ui->sourceCombo->itemData(ui->sourceCombo->currentIndex(),
                                         Presentation::QueryTreeModelBase::ObjectRole)
                              .value<Domain::DataSource::Ptr>();
    QDialog::accept();
}

void NameAndDataSourceDialog::setWindowTitle(const QString &title)
{
    QDialog::setWindowTitle(title);
}

void NameAndDataSourceDialog::setDataSourcesModel(QAbstractItemModel *model)
{
    m_flattenProxy->setSourceModel(model);
    auto proxy = ui->sourceCombo->model();
    for (int row = 0; row < proxy->rowCount(); row++) {
        auto index = proxy->index(row, 0);
        if (index.data(Presentation::QueryTreeModelBase::IsDefaultRole).toBool()) {
            ui->sourceCombo->setCurrentIndex(row);
        }
    }
}

QString NameAndDataSourceDialog::name() const
{
    return m_name;
}

Domain::DataSource::Ptr NameAndDataSourceDialog::dataSource() const
{
    return m_source;
}

void NameAndDataSourceDialog::onUserInputChanged()
{
    const auto text = ui->nameEdit->text();
    const auto source = ui->sourceCombo->itemData(ui->sourceCombo->currentIndex(),
                                                  Presentation::QueryTreeModelBase::ObjectRole)
                                       .value<Domain::DataSource::Ptr>();

    auto buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    buttonOk->setEnabled(!text.isEmpty() && source);
}

#include "nameanddatasourcedialog.moc"

#include "moc_nameanddatasourcedialog.cpp"
