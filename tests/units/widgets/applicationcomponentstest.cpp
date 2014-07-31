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

#include <QStandardItemModel>

#include "presentation/applicationmodel.h"

#include "widgets/applicationcomponents.h"
#include "widgets/datasourcecombobox.h"
#include "widgets/pageview.h"

typedef std::function<Widgets::DataSourceComboBox*(Widgets::ApplicationComponents*)> ComboGetterFunction;
Q_DECLARE_METATYPE(ComboGetterFunction)
Q_DECLARE_METATYPE(QAbstractItemModel*)

class ApplicationComponentsTest : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationComponentsTest(QObject *parent = 0)
        : QObject(parent)
    {
        qRegisterMetaType<ComboGetterFunction>();
    }

private slots:
    void shouldHaveApplicationModel()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        QObject model;

        // WHEN
        components.setModel(&model);

        // THEN
        QCOMPARE(components.model(), &model);
    }

    void shouldApplyCurrentPageModelToPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        QObject model;
        QObject currentPage;
        model.setProperty("currentPage", QVariant::fromValue(&currentPage));

        // WHEN
        components.setModel(&model);

        // THEN
        QCOMPARE(components.pageView()->model(), &currentPage);
    }

    void shouldApplyCurrentPageModelAlsoToCreatedPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.pageView();

        QObject model;
        QObject currentPage;
        model.setProperty("currentPage", QVariant::fromValue(&currentPage));

        // WHEN
        components.setModel(&model);

        // THEN
        QCOMPARE(components.pageView()->model(), &currentPage);
    }

    void shouldApplySourceModelAndPropertyToComboBox_data()
    {
        QTest::addColumn<QByteArray>("modelProperty");
        QTest::addColumn<QByteArray>("defaultProperty");
        QTest::addColumn<ComboGetterFunction>("comboGetter");

        QTest::newRow("notes") << QByteArray("noteSourcesModel") << QByteArray("defaultNoteDataSource")
                               << ComboGetterFunction(std::mem_fn(&Widgets::ApplicationComponents::defaultNoteSourceCombo));
        QTest::newRow("tasks") << QByteArray("taskSourcesModel") << QByteArray("defaultTaskDataSource")
                               << ComboGetterFunction(std::mem_fn(&Widgets::ApplicationComponents::defaultTaskSourceCombo));
    }

    void shouldApplySourceModelAndPropertyToComboBox()
    {
        // GIVEN
        QFETCH(QByteArray, modelProperty);
        QFETCH(ComboGetterFunction, comboGetter);

        Widgets::ApplicationComponents components;
        QObject model;
        QAbstractItemModel *sourcesModel = new QStandardItemModel(&model);
        model.setProperty(modelProperty, QVariant::fromValue(sourcesModel));

        // WHEN
        components.setModel(&model);

        // THEN
        Widgets::DataSourceComboBox *combo = comboGetter(&components);
        QCOMPARE(combo->model(), sourcesModel);

        QFETCH(QByteArray, defaultProperty);
        QCOMPARE(combo->defaultSourceObject(), &model);
        QCOMPARE(combo->defaultSourceProperty(), defaultProperty);
    }

    void shouldApplySourceModelAndPropertyAlsoToCreatedComboBox_data()
    {
        shouldApplySourceModelAndPropertyToComboBox_data();
    }

    void shouldApplySourceModelAndPropertyAlsoToCreatedComboBox()
    {
        // GIVEN
        QFETCH(QByteArray, modelProperty);
        QFETCH(ComboGetterFunction, comboGetter);

        Widgets::ApplicationComponents components;
        // Force creation
        comboGetter(&components);

        QObject model;
        QAbstractItemModel *sourcesModel = new QStandardItemModel(&model);
        model.setProperty(modelProperty, QVariant::fromValue(sourcesModel));

        // WHEN
        components.setModel(&model);

        // THEN
        Widgets::DataSourceComboBox *combo = comboGetter(&components);
        QCOMPARE(combo->model(), sourcesModel);

        QFETCH(QByteArray, defaultProperty);
        QCOMPARE(combo->defaultSourceObject(), &model);
        QCOMPARE(combo->defaultSourceProperty(), defaultProperty);
    }
};

QTEST_MAIN(ApplicationComponentsTest)

#include "applicationcomponentstest.moc"
