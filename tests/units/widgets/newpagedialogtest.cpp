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
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>

#include "presentation/querytreemodelbase.h"

#include "widgets/newpagedialog.h"
#include "widgets/datasourcecombobox.h"

class UserInputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit UserInputSimulator(QObject *parent = 0)
        : QObject(parent), dialog(0), reject(false), comboIndex(-1) {}

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

        if (comboIndex >= 0) {
            auto sourceCombo = dialog->findChild<Widgets::DataSourceComboBox*>("sourceCombo");
            auto combo = sourceCombo->findChild<QComboBox*>();
            combo->setCurrentIndex(comboIndex);
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
    int comboIndex;
};

class NewPageDialogTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createSourceItem(const QString &name)
    {
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);

        auto item = new QStandardItem(name);
        item->setData(QVariant::fromValue(source), Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

private slots:
    void shouldHaveDefaultState()
    {
        Widgets::NewPageDialog dialog;

        QVERIFY(dialog.name().isEmpty());
        QVERIFY(dialog.dataSource().isNull());

        auto nameEdit = dialog.findChild<QLineEdit*>("nameEdit");
        QVERIFY(nameEdit);
        QVERIFY(nameEdit->isVisibleTo(&dialog));

        auto sourceCombo = dialog.findChild<Widgets::DataSourceComboBox*>("sourceCombo");
        QVERIFY(sourceCombo);
        QVERIFY(sourceCombo->isVisibleTo(&dialog));

        auto buttonBox = dialog.findChild<QDialogButtonBox*>("buttonBox");
        QVERIFY(buttonBox);
        QVERIFY(buttonBox->isVisibleTo(&dialog));
        QVERIFY(buttonBox->button(QDialogButtonBox::Ok));
        QVERIFY(buttonBox->button(QDialogButtonBox::Cancel));
    }

    void shouldProvideUserInputWhenAccepted()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;

        QStandardItemModel model;
        model.appendRow(createSourceItem("source 1"));
        model.appendRow(createSourceItem("source 2"));
        model.appendRow(createSourceItem("source 3"));
        dialog.setDataSourcesModel(&model);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.comboIndex = 1;
        userInput.nameInput = "name";

        auto expectedSource = model.item(userInput.comboIndex)
                                   ->data(Presentation::QueryTreeModelBase::ObjectRole)
                                   .value<Domain::DataSource::Ptr>();

        // WHEN
        userInput.exec();

        // THEN
        QCOMPARE(dialog.name(), userInput.nameInput);
        QCOMPARE(dialog.dataSource(), expectedSource);
    }

    void shouldNotProvideUserInputWhenReject()
    {
        // GIVEN
        Widgets::NewPageDialog dialog;

        QStandardItemModel model;
        model.appendRow(createSourceItem("source 1"));
        model.appendRow(createSourceItem("source 2"));
        model.appendRow(createSourceItem("source 3"));
        dialog.setDataSourcesModel(&model);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.comboIndex = 1;
        userInput.nameInput = "name";
        userInput.reject = true;

        // WHEN
        userInput.exec();

        // THEN
        QCOMPARE(dialog.name(), QString());
        QCOMPARE(dialog.dataSource(), Domain::DataSource::Ptr());
    }
};

QTEST_MAIN(NewPageDialogTest)

#include "newpagedialogtest.moc"
