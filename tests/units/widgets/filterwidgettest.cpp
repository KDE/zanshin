/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_gui_zanshin.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>

#include <KLocalizedString>

#include "widgets/filterwidget.h"

#include "presentation/taskfilterproxymodel.h"

class FilterWidgetTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::FilterWidget filter;

        QVERIFY(filter.proxyModel());
        QVERIFY(!filter.proxyModel()->sourceModel());
        QCOMPARE(filter.proxyModel()->filterRegularExpression(), QRegularExpression());
        QCOMPARE(filter.proxyModel()->sortOrder(), Qt::AscendingOrder);
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::TaskFilterProxyModel::TitleSort);

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>(QStringLiteral("filterEdit"));
        QVERIFY(filterEdit);
        QVERIFY(filterEdit->isVisibleTo(&filter));
        QVERIFY(filterEdit->text().isEmpty());
        QCOMPARE(filterEdit->placeholderText(), i18n("Filter..."));

        auto extensionButton = filter.findChild<QToolButton*>(QStringLiteral("extensionButton"));
        QVERIFY(extensionButton);
        QVERIFY(extensionButton->isVisibleTo(&filter));
        QVERIFY(!extensionButton->isChecked());
        QVERIFY(extensionButton->autoRaise());

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>(QStringLiteral("sortTypeCombo"));
        QVERIFY(sortTypeCombo);
        QVERIFY(!sortTypeCombo->isVisibleTo(&filter));
        QCOMPARE(sortTypeCombo->currentIndex(), 0);

        auto ascendingButton = filter.findChild<QToolButton*>(QStringLiteral("ascendingButton"));
        QVERIFY(ascendingButton);
        QVERIFY(!ascendingButton->isVisibleTo(&filter));
        QVERIFY(ascendingButton->isChecked());
        QVERIFY(ascendingButton->autoRaise());

        auto descendingButton = filter.findChild<QToolButton*>(QStringLiteral("descendingButton"));
        QVERIFY(descendingButton);
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isChecked());
        QVERIFY(descendingButton->autoRaise());
    }

    void shouldChangeAppliedFilter()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>(QStringLiteral("filterEdit"));
        QVERIFY(filterEdit);

        // WHEN
        QTest::keyClicks(filterEdit, QStringLiteral("find me"));

        // THEN
        QCOMPARE(filter.proxyModel()->filterRegularExpression().pattern(), QStringLiteral("find\\ me"));
    }

    void shouldClearFilter()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>(QStringLiteral("filterEdit"));
        QVERIFY(filterEdit);
        filterEdit->setText("Foo");

        // WHEN
        filter.clear();

        // THEN
        QVERIFY(filterEdit->text().isEmpty());
        QVERIFY(filter.proxyModel()->filterRegularExpression().pattern().isEmpty());
    }

    void shouldShowExtension()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QAbstractButton *extensionButton = filter.findChild<QAbstractButton*>(QStringLiteral("extensionButton"));
        QVERIFY(extensionButton);

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>(QStringLiteral("sortTypeCombo"));
        QVERIFY(sortTypeCombo);

        QAbstractButton *ascendingButton = filter.findChild<QAbstractButton*>(QStringLiteral("ascendingButton"));
        QVERIFY(ascendingButton);

        QAbstractButton *descendingButton = filter.findChild<QAbstractButton*>(QStringLiteral("descendingButton"));
        QVERIFY(descendingButton);

        // WHEN
        extensionButton->click();

        // THEN
        QVERIFY(extensionButton->isChecked());
        QVERIFY(sortTypeCombo->isVisibleTo(&filter));
        QVERIFY(descendingButton->isVisibleTo(&filter));
        QVERIFY(descendingButton->isVisibleTo(&filter));

        // WHEN
        extensionButton->click();

        // THEN
        QVERIFY(!extensionButton->isChecked());
        QVERIFY(!sortTypeCombo->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isVisibleTo(&filter));
    }

    void shouldChangeSortType()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>(QStringLiteral("sortTypeCombo"));
        QVERIFY(sortTypeCombo);

        // WHEN
        sortTypeCombo->setCurrentIndex(1);

        // THEN
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::TaskFilterProxyModel::DateSort);

        // WHEN
        sortTypeCombo->setCurrentIndex(0);

        // THEN
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::TaskFilterProxyModel::TitleSort);
    }

    void shouldChangeSortOrder()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QAbstractButton *ascendingButton = filter.findChild<QAbstractButton*>(QStringLiteral("ascendingButton"));
        QVERIFY(ascendingButton);

        QAbstractButton *descendingButton = filter.findChild<QAbstractButton*>(QStringLiteral("descendingButton"));
        QVERIFY(descendingButton);

        // WHEN
        descendingButton->click();

        // THEN
        QVERIFY(!ascendingButton->isChecked());
        QVERIFY(descendingButton->isChecked());
        QCOMPARE(filter.proxyModel()->sortOrder(), Qt::DescendingOrder);

        // WHEN
        ascendingButton->click();

        // THEN
        QVERIFY(ascendingButton->isChecked());
        QVERIFY(!descendingButton->isChecked());
        QCOMPARE(filter.proxyModel()->sortOrder(), Qt::AscendingOrder);
    }

    void shouldShowFutureTasks()
    {
        // GIVEN
        Widgets::FilterWidget filter;
        filter.setShowFutureTasks(false);

        // THEN
        QVERIFY(!filter.proxyModel()->showFutureTasks());

        // WHEN
        filter.setShowFutureTasks(true);

        // THEN
        QVERIFY(filter.proxyModel()->showFutureTasks());
    }

    void shouldShowDoneTasks()
    {
        // GIVEN
        Widgets::FilterWidget filter;
        filter.setShowDoneTasks(false);

        // THEN
        QVERIFY(!filter.proxyModel()->showDoneTasks());

        // WHEN
        filter.setShowDoneTasks(true);

        // THEN
        QVERIFY(filter.proxyModel()->showDoneTasks());
    }
};

ZANSHIN_TEST_MAIN(FilterWidgetTest)

#include "filterwidgettest.moc"
