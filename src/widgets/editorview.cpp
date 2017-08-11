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


#include "editorview.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>

#include <KLocalizedString>

#include "kdateedit.h"
#include "addressline/addresseelineedit.h"

#include "domain/artifact.h"

#include "ui_editorview.h"

using namespace Widgets;

EditorView::EditorView(QWidget *parent)
    : QWidget(parent),
      m_model(Q_NULLPTR),
      ui(new Ui::EditorView),
      m_delegateEdit(Q_NULLPTR)
{
    ui->setupUi(this);

    // To avoid having unit tests talking to akonadi
    // while we don't need the completion for them
    if (qEnvironmentVariableIsEmpty("ZANSHIN_UNIT_TEST_RUN"))
        m_delegateEdit = new KPIM::AddresseeLineEdit(ui->delegateEditPlaceHolder);
    else
        m_delegateEdit = new KLineEdit(ui->delegateEditPlaceHolder);

    // placing our special DelegateEdit into the placeholder we prepared
    m_delegateEdit->setObjectName("delegateEdit");
    ui->delegateToLabel->setBuddy(m_delegateEdit);
    ui->delegateEditPlaceHolder->layout()->addWidget(m_delegateEdit);

    ui->startDateEdit->setMinimumContentsLength(10);
    ui->dueDateEdit->setMinimumContentsLength(10);

    // Make sure our minimum width is always the one with
    // the task group visible
    ui->layout->activate();
    setMinimumWidth(minimumSizeHint().width());

    ui->delegateLabel->setVisible(false);
    ui->taskGroup->setVisible(false);

    ui->textEdit->installEventFilter(this);
    ui->startDateEdit->installEventFilter(this);
    ui->dueDateEdit->installEventFilter(this);
    ui->doneButton->installEventFilter(this);
    m_delegateEdit->installEventFilter(this);

    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextEditChanged);
    connect(ui->startDateEdit, &KPIM::KDateEdit::dateEntered, this, &EditorView::onStartEditEntered);
    connect(ui->dueDateEdit, &KPIM::KDateEdit::dateEntered, this, &EditorView::onDueEditEntered);
    connect(ui->doneButton, &QAbstractButton::toggled, this, &EditorView::onDoneButtonChanged);
    connect(ui->startTodayButton, &QAbstractButton::clicked, this, &EditorView::onStartTodayClicked);
    connect(m_delegateEdit, &KLineEdit::returnPressed, this, &EditorView::onDelegateEntered);

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

void EditorView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    if (m_model) {
        ui->attachmentList->setModel(Q_NULLPTR);
        disconnect(m_model, Q_NULLPTR, this, Q_NULLPTR);
        disconnect(this, Q_NULLPTR, m_model, Q_NULLPTR);
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

    onArtifactChanged();
    onTextOrTitleChanged();
    onHasTaskPropertiesChanged();
    onStartDateChanged();
    onDueDateChanged();
    onDoneChanged();
    onDelegateTextChanged();

    connect(m_model, SIGNAL(artifactChanged(Domain::Artifact::Ptr)),
            this, SLOT(onArtifactChanged()));
    connect(m_model, SIGNAL(hasTaskPropertiesChanged(bool)),
            this, SLOT(onHasTaskPropertiesChanged()));
    connect(m_model, SIGNAL(titleChanged(QString)), this, SLOT(onTextOrTitleChanged()));
    connect(m_model, SIGNAL(textChanged(QString)), this, SLOT(onTextOrTitleChanged()));
    connect(m_model, SIGNAL(startDateChanged(QDateTime)), this, SLOT(onStartDateChanged()));
    connect(m_model, SIGNAL(dueDateChanged(QDateTime)), this, SLOT(onDueDateChanged()));
    connect(m_model, SIGNAL(doneChanged(bool)), this, SLOT(onDoneChanged()));
    connect(m_model, SIGNAL(delegateTextChanged(QString)), this, SLOT(onDelegateTextChanged()));

    connect(this, SIGNAL(titleChanged(QString)), m_model, SLOT(setTitle(QString)));
    connect(this, SIGNAL(textChanged(QString)), m_model, SLOT(setText(QString)));
    connect(this, SIGNAL(startDateChanged(QDateTime)), m_model, SLOT(setStartDate(QDateTime)));
    connect(this, SIGNAL(dueDateChanged(QDateTime)), m_model, SLOT(setDueDate(QDateTime)));
    connect(this, SIGNAL(doneChanged(bool)), m_model, SLOT(setDone(bool)));
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

void EditorView::onArtifactChanged()
{
    auto artifact = m_model->property("artifact").value<Domain::Artifact::Ptr>();
    setEnabled(artifact);
    m_delegateEdit->clear();
}

void EditorView::onHasTaskPropertiesChanged()
{
    ui->taskGroup->setVisible(m_model->property("hasTaskProperties").toBool());
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
    ui->startDateEdit->setDate(m_model->property("startDate").toDateTime().date());
}

void EditorView::onDueDateChanged()
{
    ui->dueDateEdit->setDate(m_model->property("dueDate").toDateTime().date());
}

void EditorView::onDoneChanged()
{
    ui->doneButton->setChecked(m_model->property("done").toBool());
}

void EditorView::onDelegateTextChanged()
{
    const auto delegateText = m_model->property("delegateText").toString();
    const auto labelText = delegateText.isEmpty() ? QString()
                         : i18n("Delegated to: <b>%1</b>", delegateText);

    ui->delegateLabel->setVisible(!labelText.isEmpty());
    ui->delegateLabel->setText(labelText);
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
    emit startDateChanged(QDateTime(start, QTime(), Qt::UTC));
}

void EditorView::onDueEditEntered(const QDate &due)
{
    emit dueDateChanged(QDateTime(due, QTime(), Qt::UTC));
}

void EditorView::onDoneButtonChanged(bool checked)
{
    emit doneChanged(checked);
}

void EditorView::onStartTodayClicked()
{
    QDate today(QDate::currentDate());
    ui->startDateEdit->setDate(today);
    emit startDateChanged(QDateTime(today, QTime(), Qt::UTC));
}

void EditorView::onDelegateEntered()
{
    const auto input = m_delegateEdit->text();
    auto name = QString();
    auto email = QString();
    auto gotMatch = false;

    QRegExp fullRx("\\s*(.*) <([\\w\\.]+@[\\w\\.]+)>\\s*");
    QRegExp emailOnlyRx("\\s*<?([\\w\\.]+@[\\w\\.]+)>?\\s*");

    if (input.contains(fullRx)) {
        name = fullRx.cap(1);
        email = fullRx.cap(2);
        gotMatch = true;
    } else if (input.contains(emailOnlyRx)) {
        email = emailOnlyRx.cap(1);
        gotMatch = true;
    }

    if (gotMatch) {
        QMetaObject::invokeMethod(m_model, "delegate",
                                  Q_ARG(QString, name),
                                  Q_ARG(QString, email));
        m_delegateEdit->clear();
    }
}
