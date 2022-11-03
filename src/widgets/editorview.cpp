/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "editorview.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>

#include <KDateComboBox>
#include <KLocalizedString>

#include "domain/task.h"

#include "ui_editorview.h"

using namespace Widgets;

EditorView::EditorView(QWidget *parent)
    : QWidget(parent),
      m_model(nullptr),
      ui(new Ui::EditorView)
{
    m_requestFileNameFunction = [](QWidget *parent) {
        return QFileDialog::getOpenFileName(parent, i18n("Add Attachment"));
    };

    ui->setupUi(this);

    ui->recurrenceCombo->addItem(i18n("Never"), QVariant::fromValue(Domain::Task::NoRecurrence));
    ui->recurrenceCombo->addItem(i18n("Daily"), QVariant::fromValue(Domain::Task::RecursDaily));
    ui->recurrenceCombo->addItem(i18n("Weekly"), QVariant::fromValue(Domain::Task::RecursWeekly));
    ui->recurrenceCombo->addItem(i18n("Monthly"), QVariant::fromValue(Domain::Task::RecursMonthly));
    ui->recurrenceCombo->addItem(i18n("Yearly"), QVariant::fromValue(Domain::Task::RecursYearly));

    ui->textEdit->installEventFilter(this);
    ui->startDateEdit->installEventFilter(this);
    ui->dueDateEdit->installEventFilter(this);
    ui->doneButton->installEventFilter(this);
    ui->recurrenceCombo->installEventFilter(this);

    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextEditChanged);
    connect(ui->startDateEdit, &KDateComboBox::dateEntered, this, &EditorView::onStartEditEntered);
    connect(ui->dueDateEdit, &KDateComboBox::dateEntered, this, &EditorView::onDueEditEntered);
    connect(ui->doneButton, &QAbstractButton::toggled, this, &EditorView::onDoneButtonChanged);
    connect(ui->startTodayButton, &QAbstractButton::clicked, this, &EditorView::onStartTodayClicked);
    connect(ui->recurrenceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &EditorView::onRecurrenceComboChanged);
    connect(ui->attachmentList, &QAbstractItemView::doubleClicked, this, &EditorView::onAttachmentDoubleClicked);
    connect(ui->addAttachmentButton, &QToolButton::clicked, this, &EditorView::onAddAttachmentClicked);
    connect(ui->removeAttachmentButton, &QToolButton::clicked, this, &EditorView::onRemoveAttachmentClicked);

    setEnabled(false);
}

EditorView::~EditorView()
{
    delete ui;
}

QObject *EditorView::model() const
{
    return m_model;
}

EditorView::RequestFileNameFunction EditorView::requestFileNameFunction() const
{
    return m_requestFileNameFunction;
}

void EditorView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    if (m_model) {
        disconnect(ui->attachmentList->selectionModel(), &QItemSelectionModel::selectionChanged,
                   this, &EditorView::onAttachmentSelectionChanged);
        ui->attachmentList->setModel(nullptr);
        disconnect(m_model, nullptr, this, nullptr);
        disconnect(this, nullptr, m_model, nullptr);
    }

    m_model = model;

    setEnabled(m_model);

    if (!m_model) {
        ui->taskGroup->setVisible(false);
        ui->textEdit->clear();
        return;
    }

    auto attachments = m_model->property("attachmentModel").value<QAbstractItemModel*>();
    ui->attachmentList->setModel(attachments);
    connect(ui->attachmentList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EditorView::onAttachmentSelectionChanged);

    onTaskChanged();
    onTextOrTitleChanged();
    onStartDateChanged();
    onDueDateChanged();
    onDoneChanged();
    onRecurrenceChanged();
    onAttachmentSelectionChanged();

    connect(m_model, SIGNAL(taskChanged(Domain::Task::Ptr)),
            this, SLOT(onTaskChanged()));
    connect(m_model, SIGNAL(titleChanged(QString)), this, SLOT(onTextOrTitleChanged()));
    connect(m_model, SIGNAL(textChanged(QString)), this, SLOT(onTextOrTitleChanged()));
    connect(m_model, SIGNAL(startDateChanged(QDate)), this, SLOT(onStartDateChanged()));
    connect(m_model, SIGNAL(dueDateChanged(QDate)), this, SLOT(onDueDateChanged()));
    connect(m_model, SIGNAL(doneChanged(bool)), this, SLOT(onDoneChanged()));
    connect(m_model, SIGNAL(recurrenceChanged(Domain::Task::Recurrence)), this, SLOT(onRecurrenceChanged()));

    connect(this, SIGNAL(titleChanged(QString)), m_model, SLOT(setTitle(QString)));
    connect(this, SIGNAL(textChanged(QString)), m_model, SLOT(setText(QString)));
    connect(this, SIGNAL(startDateChanged(QDate)), m_model, SLOT(setStartDate(QDate)));
    connect(this, SIGNAL(dueDateChanged(QDate)), m_model, SLOT(setDueDate(QDate)));
    connect(this, SIGNAL(doneChanged(bool)), m_model, SLOT(setDone(bool)));
    connect(this, SIGNAL(recurrenceChanged(Domain::Task::Recurrence)), m_model, SLOT(setRecurrence(Domain::Task::Recurrence)));
}

void EditorView::setRequestFileNameFunction(const EditorView::RequestFileNameFunction &function)
{
    m_requestFileNameFunction = function;
}

bool EditorView::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    switch (event->type()) {
    case QEvent::FocusIn:
        // We don't want to replace text being edited by the user with older text
        // coming from akonadi notifications (async, after some older save job)
        m_model->setProperty("editingInProgress", true);
        break;
    case QEvent::FocusOut:
        // We do react to notifications, however, when not having the focus,
        // for instance when changing the title using the central list.
        m_model->setProperty("editingInProgress", false);
        break;
    default:
        break;
    }
    return false;
}

void EditorView::onTaskChanged()
{
    auto task = m_model->property("task").value<Domain::Task::Ptr>();
    setEnabled(task);
}

void EditorView::onTextOrTitleChanged()
{
    const auto title = m_model->property("title").toString();
    const auto text = m_model->property("text").toString();
    const auto fullText = QString(title + '\n' + text);

    if (ui->textEdit->toPlainText() != fullText) // QPlainTextEdit doesn't do this check
        ui->textEdit->setPlainText(fullText);
}

void EditorView::onStartDateChanged()
{
    ui->startDateEdit->setDate(m_model->property("startDate").toDate());
}

void EditorView::onDueDateChanged()
{
    ui->dueDateEdit->setDate(m_model->property("dueDate").toDate());
}

void EditorView::onDoneChanged()
{
    ui->doneButton->setChecked(m_model->property("done").toBool());
}

void EditorView::onRecurrenceChanged()
{
    const auto recurrence = m_model->property("recurrence").value<Domain::Task::Recurrence>();
    for (int index = 0; index < ui->recurrenceCombo->count(); index++) {
        if (recurrence == ui->recurrenceCombo->itemData(index).value<Domain::Task::Recurrence>()) {
            ui->recurrenceCombo->setCurrentIndex(index);
            return;
        }
    }
}

void EditorView::onTextEditChanged()
{
    const QString plainText = ui->textEdit->toPlainText();
    const int index = plainText.indexOf('\n');
    if (index < 0) {
        emit titleChanged(plainText);
        emit textChanged(QString());
    } else {
        const QString title = plainText.left(index);
        const QString text = plainText.mid(index + 1);
        emit titleChanged(title);
        emit textChanged(text);
    }
}

void EditorView::onStartEditEntered(const QDate &start)
{
    emit startDateChanged(start);
}

void EditorView::onDueEditEntered(const QDate &due)
{
    emit dueDateChanged(due);
}

void EditorView::onDoneButtonChanged(bool checked)
{
    emit doneChanged(checked);
}

void EditorView::onStartTodayClicked()
{
    QDate today(QDate::currentDate());
    ui->startDateEdit->setDate(today);
    emit startDateChanged(today);
}

void EditorView::onRecurrenceComboChanged(int index)
{
    const auto recurrence = ui->recurrenceCombo->itemData(index).value<Domain::Task::Recurrence>();
    emit recurrenceChanged(recurrence);
}

void EditorView::onAttachmentSelectionChanged()
{
    if (!m_model)
        return;

    const auto selectionModel = ui->attachmentList->selectionModel();
    const auto selectedIndexes = selectionModel->selectedIndexes();
    ui->removeAttachmentButton->setEnabled(!selectedIndexes.isEmpty());
}

void EditorView::onAddAttachmentClicked()
{
    if (!m_model)
        return;

    auto fileName = m_requestFileNameFunction(this);
    if (!fileName.isEmpty())
        QMetaObject::invokeMethod(m_model, "addAttachment", Q_ARG(QString, fileName));
}

void EditorView::onRemoveAttachmentClicked()
{
    if (!m_model)
        return;

    const auto selectionModel = ui->attachmentList->selectionModel();
    const auto selectedIndexes = selectionModel->selectedIndexes();
    if (!selectedIndexes.isEmpty())
        QMetaObject::invokeMethod(m_model, "removeAttachment", Q_ARG(QModelIndex, selectedIndexes.first()));
}

void EditorView::onAttachmentDoubleClicked(const QModelIndex &index)
{
    if (!m_model)
        return;

    QMetaObject::invokeMethod(m_model, "openAttachment", Q_ARG(QModelIndex, index));
}
