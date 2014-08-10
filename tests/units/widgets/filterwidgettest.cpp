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

#include <QAbstractButton>
#include <QComboBox>
#include <QLineEdit>

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

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>("filterEdit");
        QVERIFY(filterEdit);
        QVERIFY(filterEdit->isVisibleTo(&filter));
        QVERIFY(filterEdit->text().isEmpty());
        QCOMPARE(filterEdit->placeholderText(), tr("Filter..."));

        QAbstractButton *extensionButton = filter.findChild<QAbstractButton*>("extensionButton");
        QVERIFY(extensionButton);
        QVERIFY(extensionButton->isVisibleTo(&filter));
        QVERIFY(!extensionButton->isChecked());
        QCOMPARE(extensionButton->icon(), QIcon::fromTheme("arrow-down-double"));

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>("sortTypeCombo");
        QVERIFY(sortTypeCombo);
        QVERIFY(!sortTypeCombo->isVisibleTo(&filter));
        QCOMPARE(sortTypeCombo->currentIndex(), 0);

        QAbstractButton *ascendingButton = filter.findChild<QAbstractButton*>("ascendingButton");
        QVERIFY(ascendingButton);
        QVERIFY(!ascendingButton->isVisibleTo(&filter));
        QVERIFY(ascendingButton->isChecked());
        QCOMPARE(ascendingButton->icon(), QIcon::fromTheme("arrow-up"));

        QAbstractButton *descendingButton = filter.findChild<QAbstractButton*>("descendingButton");
        QVERIFY(descendingButton);
        QVERIFY(!descendingButton->isVisibleTo(&filter));
        QVERIFY(!descendingButton->isChecked());
        QCOMPARE(descendingButton->icon(), QIcon::fromTheme("arrow-down"));
    }

    void shouldChangeAppliedFilter()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QLineEdit *filterEdit = filter.findChild<QLineEdit*>("filterEdit");
        QVERIFY(filterEdit);

        // WHEN
        QTest::keyClicks(filterEdit, "find me");

        // THEN
        QCOMPARE(filter.proxyModel()->filterRegExp().pattern(), QString("find me"));
    }

    void shouldShowExtension()
    {
        // GIVEN
        Widgets::FilterWidget filter;

        QAbstractButton *extensionButton = filter.findChild<QAbstractButton*>("extensionButton");
        QVERIFY(extensionButton);

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>("sortTypeCombo");
        QVERIFY(sortTypeCombo);

        QAbstractButton *ascendingButton = filter.findChild<QAbstractButton*>("ascendingButton");
        QVERIFY(ascendingButton);

        QAbstractButton *descendingButton = filter.findChild<QAbstractButton*>("descendingButton");
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

        QComboBox *sortTypeCombo = filter.findChild<QComboBox*>("sortTypeCombo");
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

        QAbstractButton *ascendingButton = filter.findChild<QAbstractButton*>("ascendingButton");
        QVERIFY(ascendingButton);

        QAbstractButton *descendingButton = filter.findChild<QAbstractButton*>("descendingButton");
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
};

QTEST_MAIN(FilterWidgetTest)

#include "filterwidgettest.moc"
