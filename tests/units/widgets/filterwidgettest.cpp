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

#include <testlib/qtest_gui_zanshin.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>

#include "widgets/filterwidget.h"

#include "presentation/artifactfilterproxymodel.h"

class FilterWidgetTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::FilterWidget filter;

        QVERIFY(filter.proxyModel());
        QVERIFY(!filter.proxyModel()->sourceModel());
        QCOMPARE(filter.proxyModel()->filterRegExp(), QRegExp());
        QCOMPARE(filter.proxyModel()->sortOrder(), Qt::AscendingOrder);
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::ArtifactFilterProxyModel::TitleSort);

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>(QStringLiteral("filterEdit"));
        QVERIFY(filterEdit);
        QVERIFY(filterEdit->isVisibleTo(&filter));
        QVERIFY(filterEdit->text().isEmpty());
        QCOMPARE(filterEdit->placeholderText(), tr("Filter..."));

        auto extensionButton = filter.findChild<QToolButton*>(QStringLiteral("extensionButton"));
        QVERIFY(extensionButton);
        QVERIFY(extensionButton->isVisibleTo(&filter));
        QVERIFY(!extensionButton->isChecked());
        QCOMPARE(extensionButton->icon(), QIcon::fromTheme(QStringLiteral("arrow-down-double")));
        QVERIFY(extensionButton->autoRaise());

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>(QStringLiteral("sortTypeCombo"));
        QVERIFY(sortTypeCombo);
        QVERIFY(!sortTypeCombo->isVisibleTo(&filter));
        QCOMPARE(sortTypeCombo->currentIndex(), 0);

        auto ascendingButton = filter.findChild<QToolButton*>(QStringLiteral("ascendingButton"));
        QVERIFY(ascendingButton);
        QVERIFY(!ascendingButton->isVisibleTo(&filter));
        QVERIFY(ascendingButton->isChecked());
        QCOMPARE(ascendingButton->icon(), QIcon::fromTheme(QStringLiteral("arrow-up")));
        QVERIFY(ascendingButton->autoRaise());

        auto descendingButton = filter.findChild<QToolButton*>(QStringLiteral("descendingButton"));
        QVERIFY(descendingButton);
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isChecked());
        QCOMPARE(descendingButton->icon(), QIcon::fromTheme(QStringLiteral("arrow-down")));
        QVERIFY(descendingButton->autoRaise());

        auto showFutureCheck = filter.findChild<QCheckBox*>(QStringLiteral("showFutureCheck"));
        QVERIFY(showFutureCheck);
        QVERIFY(!showFutureCheck->isVisibleTo(&filter));
        QVERIFY(!showFutureCheck->isChecked());
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
        QCOMPARE(filter.proxyModel()->filterRegExp().pattern(), QStringLiteral("find me"));
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
        QVERIFY(filter.proxyModel()->filterRegExp().pattern().isEmpty());
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

        auto showFutureCheck = filter.findChild<QCheckBox*>(QStringLiteral("showFutureCheck"));
        QVERIFY(showFutureCheck);

        // WHEN
        extensionButton->click();

        // THEN
        QVERIFY(extensionButton->isChecked());
        QVERIFY(sortTypeCombo->isVisibleTo(&filter));
        QVERIFY(descendingButton->isVisibleTo(&filter));
        QVERIFY(descendingButton->isVisibleTo(&filter));
        QVERIFY(showFutureCheck->isVisibleTo(&filter));

        // WHEN
        extensionButton->click();

        // THEN
        QVERIFY(!extensionButton->isChecked());
        QVERIFY(!sortTypeCombo->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!showFutureCheck->isVisibleTo(&filter));
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
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::ArtifactFilterProxyModel::DateSort);

        // WHEN
        sortTypeCombo->setCurrentIndex(0);

        // THEN
        QCOMPARE(filter.proxyModel()->sortType(), Presentation::ArtifactFilterProxyModel::TitleSort);
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

    void shouldShowHideFutureTasks()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        auto showFutureCheck = filter.findChild<QCheckBox*>(QStringLiteral("showFutureCheck"));
        QVERIFY(showFutureCheck);

        // WHEN
        showFutureCheck->click();

        // THEN
        QVERIFY(showFutureCheck->isChecked());
        QVERIFY(filter.proxyModel()->showFutureTasks());

        // WHEN
        showFutureCheck->click();

        // THEN
        QVERIFY(!showFutureCheck->isChecked());
        QVERIFY(!filter.proxyModel()->showFutureTasks());
    }
};

ZANSHIN_TEST_MAIN(FilterWidgetTest)

#include "filterwidgettest.moc"
