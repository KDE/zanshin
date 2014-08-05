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
#include <QTreeView>

#include "domain/note.h"
#include "domain/task.h"

#include "presentation/applicationmodel.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/applicationcomponents.h"
#include "widgets/datasourcecombobox.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

typedef std::function<Widgets::DataSourceComboBox*(Widgets::ApplicationComponents*)> ComboGetterFunction;
Q_DECLARE_METATYPE(ComboGetterFunction)

class PageModelStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    QAbstractItemModel *centralListModel()
    {
        return &itemModel;
    }

    template<typename T>
    void addItem(const QString &title)
    {
        auto artifact = T::Ptr::create();
        artifact->setTitle(title);
        addItem(artifact);
    }

    void addItem(const Domain::Artifact::Ptr &artifact)
    {
        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(artifact), Presentation::QueryTreeModelBase::ObjectRole);
        item->setData(artifact->title(), Qt::DisplayRole);
        itemModel.appendRow(item);
    }

    Domain::Artifact::Ptr itemAtRow(int row) const
    {
        return itemModel.index(row, 0).data(Presentation::QueryTreeModelBase::ObjectRole)
                                      .value<Domain::Artifact::Ptr>();
    }

public:
    QStandardItemModel itemModel;
};

class EditorModelStub : public QObject
{
    Q_OBJECT
public:
    explicit EditorModelStub(QObject *parent = 0)
        : QObject(parent)
    {
    }

    void setPropertyAndSignal(const QByteArray &name, const QVariant &value)
    {
        if (property(name) == value)
            return;

        setProperty(name, value);
        if (name == "text")
            emit textChanged(value.toString());
        else if (name == "title")
            emit titleChanged(value.toString());
        else if (name == "done")
            emit doneChanged(value.toBool());
        else if (name == "startDate")
            emit startDateChanged(value.toDateTime());
        else if (name == "dueDate")
            emit dueDateChanged(value.toDateTime());
        else if (name == "hasTaskProperties")
            emit hasTaskPropertiesChanged(value.toBool());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public slots:
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDateTime &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDateTime &due) { setPropertyAndSignal("dueDate", due); }

signals:
    void hasTaskPropertiesChanged(bool hasTaskProperties);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDateTime &date);
    void dueDateChanged(const QDateTime &due);
};

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

    void shouldApplyEditorModelToEditorView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        QObject model;
        QObject *editorModel = new EditorModelStub(&model);
        model.setProperty("editor", QVariant::fromValue(editorModel));

        // WHEN
        components.setModel(&model);

        // THEN
        QCOMPARE(components.editorView()->model(), editorModel);
    }

    void shouldApplyEditorModelAltoToCreatedPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.editorView();

        QObject model;
        QObject *editorModel = new EditorModelStub(&model);
        model.setProperty("editor", QVariant::fromValue(editorModel));

        // WHEN
        components.setModel(&model);

        // THEN
        QCOMPARE(components.editorView()->model(), editorModel);
    }

    void shouldApplyPageViewSelectionToEditorModel()
    {
        // GIVEN
        QObject model;

        PageModelStub pageModel;
        pageModel.addItem<Domain::Task>("0. First task");
        pageModel.addItem<Domain::Note>("1. A note");
        pageModel.addItem<Domain::Task>("2. Second task");
        pageModel.addItem<Domain::Note>("3. Another note");
        model.setProperty("currentPage", QVariant::fromValue<QObject*>(&pageModel));

        EditorModelStub editorModel;
        model.setProperty("editor", QVariant::fromValue<QObject*>(&editorModel));

        Widgets::ApplicationComponents components;
        components.setModel(&model);

        Widgets::PageView *pageView = components.pageView();
        auto centralView = pageView->findChild<QTreeView*>("centralView");
        QModelIndex index = centralView->model()->index(2, 0);

        // WHEN
        centralView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

        // THEN
        QCOMPARE(editorModel.property("artifact").value<Domain::Artifact::Ptr>(),
                 pageModel.itemAtRow(index.row()));
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
