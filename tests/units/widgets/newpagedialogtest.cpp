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

#include <QtTestGui>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>

#include "presentation/querytreemodelbase.h"

#include "widgets/newpagedialog.h"

class UserInputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit UserInputSimulator(QObject *parent = Q_NULLPTR)
        : QObject(parent), dialog(Q_NULLPTR), reject(false), sourceComboIndex(-1), typeComboIndex(-1) {}

    void exec()
    {
        Q_ASSERT(dialog);
        QTimer::singleShot(50, this, SLOT(onTimeout()));
        dialog->exec();
    }

private slots:
    void onTimeout()
    {
        if (!nameInput.isEmpty()) {
            auto nameEdit = dialog->findChild<QLineEdit*>("nameEdit");
            QTest::keyClicks(nameEdit, nameInput);
        }

        if (sourceComboIndex >= 0) {
            auto sourceCombo = dialog->findChild<QComboBox*>("sourceCombo");
            sourceCombo->setCurrentIndex(sourceComboIndex);
        }

        if (typeComboIndex >= 0) {
            auto typeCombo = dialog->findChild<QComboBox*>("typeCombo");
            typeCombo->setCurrentIndex(typeComboIndex);
        }

        auto buttonBox = dialog->findChild<QDialogButtonBox*>("buttonBox");
        if (reject)
            buttonBox->button(QDialogButtonBox::Cancel)->click();
        else
            buttonBox->button(QDialogButtonBox::Ok)->click();
    }

public:
    Widgets::NewPageDialog *dialog;
    bool reject;
    QString nameInput;
    int sourceComboIndex;
    int typeComboIndex;
};

class NewPageDialogTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createSourceItem(const QString &name, QStandardItem *parent = Q_NULLPTR)
    {
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);

        auto item = new QStandardItem(name);
        item->setData(QVariant::fromValue(source), Presentation::QueryTreeModelBase::ObjectRole);
        if (parent)
            parent->appendRow(item);
        return item;
    }

    QStandardItem *createTaskSourceItem(const QString &name, QStandardItem *parent = Q_NULLPTR)
    {
        auto item = createSourceItem(name, parent);
        auto source = item->data(Presentation::QueryTreeModelBase::ObjectRole).value<Domain::DataSource::Ptr>();
        source->setContentTypes(Domain::DataSource::Tasks);
        return item;
    }

    QStandardItem *createDefaultSourceItem(const QString &name, QStandardItem *parent = Q_NULLPTR)
    {
        auto item = createTaskSourceItem(name, parent);
        item->setData(true, Presentation::QueryTreeModelBase::IsDefaultRole);
        return item;
    }

    QStandardItemModel *createSourceModel()
    {
        auto model = new QStandardItemModel(this);

        auto root1 = createSourceItem("Root 1");
        createSourceItem("Null", root1);
        createTaskSourceItem("Task 1.1", root1);
        createTaskSourceItem("Task 1.2", root1);
        model->appendRow(root1);

        auto root2 = createSourceItem("Root 2");
        createDefaultSourceItem("Task 2.1", root2);
        createTaskSourceItem("Task 2.2", root2);
        model->appendRow(root2);

        return model;
    }

private slots:
    int indexOfType(Widgets::NewPageDialog *dialog, const Widgets::NewPageDialogInterface::PageType type)
    {
        auto typeCombo = dialog->findChild<QComboBox*>("typeCombo");
        const int count = typeCombo->count();
        for (int index = 0 ; index < count ; index++ ) {
            auto pt = typeCombo->itemData(index).value<Widgets::NewPageDialogInterface::PageType>();
            if (pt == type)
                return index;
        }
        return -1;
    }

    void shouldHaveDefaultState()
    {
        Widgets::NewPageDialog dialog;

        QVERIFY(dialog.name().isEmpty());
        QCOMPARE(dialog.pageType(), Widgets::NewPageDialogInterface::Project);
        QVERIFY(dialog.dataSource().isNull());

        auto nameEdit = dialog.findChild<QLineEdit*>("nameEdit");
        QVERIFY(nameEdit);
        QVERIFY(nameEdit->isVisibleTo(&dialog));

        auto typeCombo = dialog.findChild<QComboBox*>("typeCombo");
        QVERIFY(typeCombo);
        QVERIFY(typeCombo->isVisibleTo(&dialog));

        auto sourceCombo = dialog.findChild<QComboBox*>("sourceCombo");
        QVERIFY(sourceCombo);
        QVERIFY(sourceCombo->isVisibleTo(&dialog));

        auto buttonBox = dialog.findChild<QDialogButtonBox*>("buttonBox");
        QVERIFY(buttonBox);
        QVERIFY(buttonBox->isVisibleTo(&dialog));
        QVERIFY(buttonBox->button(QDialogButtonBox::Ok));
        QVERIFY(buttonBox->button(QDialogButtonBox::Cancel));
    }

    void shouldPositionDefaultProperties()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;
        auto sourceModel = createSourceModel();
        auto sourceCombo = dialog.findChild<QComboBox*>("sourceCombo");
        auto pageTypeCombo = dialog.findChild<QComboBox*>("typeCombo");
        auto pageType = Widgets::NewPageDialogInterface::Project;

        // WHEN
        dialog.setDataSourcesModel(sourceModel);

        // THEN
        QCOMPARE(sourceCombo->currentIndex(), 2);
        QCOMPARE(sourceCombo->currentText(), QString("Root 2 / Task 2.1"));

        QVariant indexVariant = pageTypeCombo->itemData(pageTypeCombo->currentIndex());
        QCOMPARE(indexVariant.value<Widgets::NewPageDialogInterface::PageType>(), pageType);
    }

    void shouldProvideUserInputWhenAccepted()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 1;
        userInput.nameInput = "name";

        auto expectedSource = sourceModel->item(0)
                                         ->child(2)
                                         ->data(Presentation::QueryTreeModelBase::ObjectRole)
                                         .value<Domain::DataSource::Ptr>();

        // WHEN
        userInput.exec();

        // THEN
        QCOMPARE(dialog.name(), userInput.nameInput);
        QVERIFY(dialog.dataSource());
        QCOMPARE(dialog.dataSource(), expectedSource);
    }

    void shouldNotProvideUserInputWhenReject()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 1;
        userInput.nameInput = "name";
        userInput.reject = true;

        // WHEN
        userInput.exec();

        // THEN
        QCOMPARE(dialog.name(), QString());
        QCOMPARE(dialog.dataSource(), Domain::DataSource::Ptr());
    }

    void shouldNotAllowEmptyName()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 0;
        userInput.typeComboIndex = 0;
        userInput.nameInput = QString();
        userInput.reject = true;

        // WHEN
        userInput.exec();

        // THEN
        auto buttonOk = dialog.findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok);
        QVERIFY(!buttonOk->isEnabled());
        QCOMPARE(dialog.name(), QString());
        QCOMPARE(dialog.dataSource(), Domain::DataSource::Ptr());
    }

    void shouldHideSourceComboForNonProjectType_data()
    {
        QTest::addColumn<Widgets::NewPageDialogInterface::PageType>("pageType");

        QTest::newRow("typeComboWithContext") <<  Widgets::NewPageDialogInterface::Context;
        QTest::newRow("typeComboWithTag") <<  Widgets::NewPageDialogInterface::Tag;
    }

    void shouldHideSourceComboForNonProjectType()
    {
        // GIVEN
        QFETCH(Widgets::NewPageDialogInterface::PageType, pageType);
        Widgets::NewPageDialog dialog;

        int nonProjectIndex = indexOfType(&dialog, pageType);
        Q_ASSERT(nonProjectIndex != -1);

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 0;
        userInput.typeComboIndex = nonProjectIndex;
        userInput.nameInput = "name";
        userInput.reject = false;

        // WHEN
        userInput.exec();

        // THEN
        auto sourceCombo = dialog.findChild<QComboBox*>("sourceCombo");
        auto sourceLabel = dialog.findChild<QLabel*>("sourceLabel");

        QVERIFY(sourceCombo->isHidden());
        QVERIFY(sourceLabel->isHidden());
        QCOMPARE(dialog.name(), userInput.nameInput);
    }
};

QTEST_MAIN(NewPageDialogTest)

#include "newpagedialogtest.moc"
