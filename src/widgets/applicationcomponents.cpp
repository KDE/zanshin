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


#include "applicationcomponents.h"

#include <QBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QWidget>
#include <QWidgetAction>

#include "availablepagesview.h"
#include "availablesourcesview.h"
#include "datasourcecombobox.h"
#include "editorview.h"
#include "pageview.h"

#include "presentation/applicationmodel.h"

using namespace Widgets;

ApplicationComponents::ApplicationComponents(QWidget *parent)
    : QObject(parent),
      m_parent(parent),
      m_availableSourcesView(Q_NULLPTR),
      m_availablePagesView(Q_NULLPTR),
      m_pageView(Q_NULLPTR),
      m_editorView(Q_NULLPTR),
      m_noteCombo(Q_NULLPTR),
      m_taskCombo(Q_NULLPTR)
{
}

QObjectPtr ApplicationComponents::model() const
{
    return m_model;
}

AvailableSourcesView *ApplicationComponents::availableSourcesView() const
{
    if (!m_availableSourcesView) {
        auto availableSourcesView = new AvailableSourcesView(m_parent);
        if (m_model) {
            availableSourcesView->setModel(m_model->property("availableSources").value<QObject*>());
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_availableSourcesView = availableSourcesView;
    }

    return m_availableSourcesView;
}

AvailablePagesView *ApplicationComponents::availablePagesView() const
{
    if (!m_availablePagesView) {
        auto availablePagesView = new AvailablePagesView(m_parent);
        if (m_model) {
            availablePagesView->setModel(m_model->property("availablePages").value<QObject*>());
            availablePagesView->setProjectSourcesModel(m_model->property("taskSourcesModel").value<QAbstractItemModel*>());
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_availablePagesView = availablePagesView;

        connect(self->m_availablePagesView, SIGNAL(currentPageChanged(QObject*)),
                self, SLOT(onCurrentPageChanged(QObject*)));
    }

    return m_availablePagesView;
}

PageView *ApplicationComponents::pageView() const
{
    if (!m_pageView) {
        auto pageView = new PageView(m_parent);
        if (m_model) {
            pageView->setModel(m_model->property("currentPage").value<QObject*>());
            connect(m_model.data(), SIGNAL(currentPageChanged(QObject*)),
                    pageView, SLOT(setModel(QObject*)));
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_pageView = pageView;

        connect(self->m_pageView, SIGNAL(currentArtifactChanged(Domain::Artifact::Ptr)),
                self, SLOT(onCurrentArtifactChanged(Domain::Artifact::Ptr)));
    }

    return m_pageView;
}

EditorView *ApplicationComponents::editorView() const
{
    if (!m_editorView) {
        auto editorView = new EditorView(m_parent);
        if (m_model) {
            editorView->setModel(m_model->property("editor").value<QObject*>());
        }

        auto self = const_cast<ApplicationComponents*>(this);
        self->m_editorView = editorView;
    }

    return m_editorView;
}

QList<QAction *> ApplicationComponents::configureActions() const
{
    if (m_configureActions.isEmpty()) {
        QList<QAction*> actions;

        auto widget = new QWidget;
        widget->setLayout(new QHBoxLayout);
        widget->layout()->addWidget(new QLabel(tr("Default task source")));
        widget->layout()->addWidget(defaultTaskSourceCombo());

        auto comboAction = new QWidgetAction(m_parent);
        comboAction->setObjectName("zanshin_settings_task_sources");
        comboAction->setDefaultWidget(widget);
        actions << comboAction;

        widget = new QWidget;
        widget->setLayout(new QHBoxLayout);
        widget->layout()->addWidget(new QLabel(tr("Default note source")));
        widget->layout()->addWidget(defaultNoteSourceCombo());

        comboAction = new QWidgetAction(m_parent);
        comboAction->setObjectName("zanshin_settings_note_sources");
        comboAction->setDefaultWidget(widget);
        actions << comboAction;

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_configureActions = actions;
    }

    return m_configureActions;
}

DataSourceComboBox *ApplicationComponents::defaultNoteSourceCombo() const
{
    if (!m_noteCombo) {
        auto combo = new DataSourceComboBox(m_parent);
        combo->setObjectName("noteSourceCombo");
        combo->setFixedWidth(300);
        if (m_model.data()) {
            combo->setModel(m_model->property("noteSourcesModel").value<QAbstractItemModel*>());
            combo->setDefaultSourceProperty(m_model.data(), "defaultNoteDataSource");
            connect(combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)),
                    m_model.data(), SLOT(setDefaultNoteDataSource(Domain::DataSource::Ptr)));
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_noteCombo = combo;
    }

    return m_noteCombo;
}

DataSourceComboBox *ApplicationComponents::defaultTaskSourceCombo() const
{
    if (!m_taskCombo) {
        auto combo = new DataSourceComboBox(m_parent);
        combo->setObjectName("taskSourceCombo");
        combo->setFixedWidth(300);
        if (m_model) {
            combo->setModel(m_model->property("taskSourcesModel").value<QAbstractItemModel*>());
            combo->setDefaultSourceProperty(m_model.data(), "defaultTaskDataSource");
            connect(combo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)),
                    m_model.data(), SLOT(setDefaultTaskDataSource(Domain::DataSource::Ptr)));
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_taskCombo = combo;
    }

    return m_taskCombo;
}

void ApplicationComponents::setModel(const QObjectPtr &model)
{
    if (m_model == model)
        return;

    m_model = model;

    if (m_availableSourcesView) {
        m_availableSourcesView->setModel(m_model->property("availableSources").value<QObject*>());
    }

    if (m_availablePagesView) {
        m_availablePagesView->setModel(m_model->property("availablePages").value<QObject*>());
        m_availablePagesView->setProjectSourcesModel(m_model->property("taskSourcesModel").value<QAbstractItemModel*>());
    }

    if (m_pageView) {
        m_pageView->setModel(m_model->property("currentPage").value<QObject*>());
        connect(m_model.data(), SIGNAL(currentPageChanged(QObject*)),
                m_pageView, SLOT(setModel(QObject*)));
    }

    if (m_editorView)
        m_editorView->setModel(m_model->property("editor").value<QObject*>());

    if (m_noteCombo) {
        m_noteCombo->setModel(m_model->property("noteSourcesModel").value<QAbstractItemModel*>());
        m_noteCombo->setDefaultSourceProperty(m_model.data(), "defaultNoteDataSource");
        connect(m_noteCombo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)),
                m_model.data(), SLOT(setDefaultNoteDataSource(Domain::DataSource::Ptr)));
    }

    if (m_taskCombo) {
        m_taskCombo->setModel(m_model->property("taskSourcesModel").value<QAbstractItemModel*>());
        m_taskCombo->setDefaultSourceProperty(m_model.data(), "defaultTaskDataSource");
        connect(m_noteCombo, SIGNAL(sourceActivated(Domain::DataSource::Ptr)),
                m_model.data(), SLOT(setDefaultTaskDataSource(Domain::DataSource::Ptr)));
    }
}

void ApplicationComponents::onCurrentPageChanged(QObject *page)
{
    m_model->setProperty("currentPage", QVariant::fromValue(page));

    QObject *editorModel = m_model->property("editor").value<QObject*>();
    if (editorModel)
        editorModel->setProperty("artifact", QVariant::fromValue(Domain::Artifact::Ptr()));
}

void ApplicationComponents::onCurrentArtifactChanged(const Domain::Artifact::Ptr &artifact)
{
    auto editorModel = m_model->property("editor").value<QObject*>();
    editorModel->setProperty("artifact", QVariant::fromValue(artifact));
}
