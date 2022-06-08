/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_gui_zanshin.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>

#include "presentation/querytreemodelbase.h"

#include "widgets/nameanddatasourcedialog.h"

class UserInputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit UserInputSimulator(QObject *parent = nullptr)
        : QObject(parent), dialog(nullptr), reject(false), sourceComboIndex(-1) {}

    void exec()
    {
        Q_ASSERT(dialog);
        QTimer::singleShot(50, Qt::PreciseTimer, this, &UserInputSimulator::onTimeout);
        dialog->exec();
    }

private Q_SLOTS:
    void onTimeout()
    {
        if (!nameInput.isEmpty()) {
            auto nameEdit = dialog->findChild<QLineEdit*>(QStringLiteral("nameEdit"));
            QTest::keyClicks(nameEdit, nameInput);
        }

        auto sourceCombo = dialog->findChild<QComboBox*>(QStringLiteral("sourceCombo"));
        sourceCombo->setCurrentIndex(sourceComboIndex);

        auto buttonBox = dialog->findChild<QDialogButtonBox*>(QStringLiteral("buttonBox"));
        if (reject)
            buttonBox->button(QDialogButtonBox::Cancel)->click();
        else
            buttonBox->button(QDialogButtonBox::Ok)->click();
    }

public:
    Widgets::NameAndDataSourceDialog *dialog;
    bool reject;
    QString nameInput;
    int sourceComboIndex;
};

class NameAndDataSourceDialogTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createSourceItem(const QString &name, QStandardItem *parent = nullptr)
    {
        auto source = Domain::DataSource::Ptr::create();
        source->setName(name);

        auto item = new QStandardItem(name);
        item->setData(QVariant::fromValue(source), Presentation::QueryTreeModelBase::ObjectRole);
        if (parent)
            parent->appendRow(item);
        return item;
    }

    QStandardItem *createTaskSourceItem(const QString &name, QStandardItem *parent = nullptr)
    {
        auto item = createSourceItem(name, parent);
        auto source = item->data(Presentation::QueryTreeModelBase::ObjectRole).value<Domain::DataSource::Ptr>();
        source->setContentTypes(Domain::DataSource::Tasks);
        return item;
    }

    QStandardItem *createDefaultSourceItem(const QString &name, QStandardItem *parent = nullptr)
    {
        auto item = createTaskSourceItem(name, parent);
        item->setData(true, Presentation::QueryTreeModelBase::IsDefaultRole);
        return item;
    }

    QStandardItemModel *createSourceModel()
    {
        auto model = new QStandardItemModel(this);

        auto root1 = createSourceItem(QStringLiteral("Root 1"));
        createSourceItem(QStringLiteral("Null"), root1);
        createTaskSourceItem(QStringLiteral("Task 1.1"), root1);
        createTaskSourceItem(QStringLiteral("Task 1.2"), root1);
        model->appendRow(root1);

        auto root2 = createSourceItem(QStringLiteral("Root 2"));
        createDefaultSourceItem(QStringLiteral("Task 2.1"), root2);
        createTaskSourceItem(QStringLiteral("Task 2.2"), root2);
        model->appendRow(root2);

        return model;
    }

private Q_SLOTS:
    void shouldHaveDefaultState()
    {
        Widgets::NameAndDataSourceDialog dialog;

        QVERIFY(dialog.name().isEmpty());
        QVERIFY(dialog.dataSource().isNull());

        auto nameEdit = dialog.findChild<QLineEdit*>(QStringLiteral("nameEdit"));
        QVERIFY(nameEdit);
        QVERIFY(nameEdit->isVisibleTo(&dialog));

        auto sourceCombo = dialog.findChild<QComboBox*>(QStringLiteral("sourceCombo"));
        QVERIFY(sourceCombo);
        QVERIFY(sourceCombo->isVisibleTo(&dialog));

        auto buttonBox = dialog.findChild<QDialogButtonBox*>(QStringLiteral("buttonBox"));
        QVERIFY(buttonBox);
        QVERIFY(buttonBox->isVisibleTo(&dialog));
        QVERIFY(buttonBox->button(QDialogButtonBox::Ok));
        QVERIFY(buttonBox->button(QDialogButtonBox::Cancel));
    }

    void shouldPositionDefaultProperties()
    {
        // GIVEN
        Widgets::NameAndDataSourceDialog dialog;
        auto sourceModel = createSourceModel();
        auto sourceCombo = dialog.findChild<QComboBox*>(QStringLiteral("sourceCombo"));

        // WHEN
        dialog.setDataSourcesModel(sourceModel);

        // THEN
        QCOMPARE(sourceCombo->currentIndex(), 2);
        QCOMPARE(sourceCombo->currentText(), QStringLiteral("Root 2 / Task 2.1"));
    }

    void shouldProvideUserInputWhenAccepted()
    {
        // GIVEN
        Widgets::NameAndDataSourceDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 1;
        userInput.nameInput = QStringLiteral("name");

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
        Widgets::NameAndDataSourceDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 1;
        userInput.nameInput = QStringLiteral("name");
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
        Widgets::NameAndDataSourceDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = 0;
        userInput.nameInput = QString();
        userInput.reject = true;

        // WHEN
        userInput.exec();

        // THEN
        auto buttonOk = dialog.findChild<QDialogButtonBox*>(QStringLiteral("buttonBox"))->button(QDialogButtonBox::Ok);
        QVERIFY(!buttonOk->isEnabled());
        QCOMPARE(dialog.name(), QString());
        QCOMPARE(dialog.dataSource(), Domain::DataSource::Ptr());
    }

    void shouldNotAllowNoSelectedSource()
    {
        // GIVEN
        Widgets::NameAndDataSourceDialog dialog;

        auto sourceModel = createSourceModel();
        dialog.setDataSourcesModel(sourceModel);

        UserInputSimulator userInput;
        userInput.dialog = &dialog;
        userInput.sourceComboIndex = -1;
        userInput.nameInput = QStringLiteral("name");
        userInput.reject = true;

        // WHEN
        userInput.exec();

        // THEN
        auto buttonOk = dialog.findChild<QDialogButtonBox*>(QStringLiteral("buttonBox"))->button(QDialogButtonBox::Ok);
        QVERIFY(!buttonOk->isEnabled());
        QCOMPARE(dialog.name(), QString());
        QCOMPARE(dialog.dataSource(), Domain::DataSource::Ptr());
    }
};

ZANSHIN_TEST_MAIN(NameAndDataSourceDialogTest)

#include "nameanddatasourcedialogtest.moc"
