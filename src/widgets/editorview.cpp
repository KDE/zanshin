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
#include "kdateedit.h"
#include "addressline/addresseelineedit.h"

#include "domain/artifact.h"


using namespace Widgets;

EditorView::EditorView(QWidget *parent)
    : QWidget(parent),
      m_model(Q_NULLPTR),
      m_delegateLabel(new QLabel(this)),
      m_textEdit(new QPlainTextEdit(this)),
      m_taskGroup(new QWidget(this)),
      m_startDateEdit(new KPIM::KDateEdit(m_taskGroup)),
      m_dueDateEdit(new KPIM::KDateEdit(m_taskGroup)),
      m_startTodayButton(new QPushButton(tr("Start today"), m_taskGroup)),
      m_doneButton(new QCheckBox(tr("Done"), m_taskGroup)),
      m_delegateEdit(Q_NULLPTR)
{
    // To avoid having unit tests talking to akonadi
    // while we don't need the completion for them
    if (qgetenv("ZANSHIN_UNIT_TEST_RUN").isEmpty())
        m_delegateEdit = new KPIM::AddresseeLineEdit(this);
    else
        m_delegateEdit = new KLineEdit(this);

    m_delegateLabel->setObjectName("delegateLabel");
    m_delegateEdit->setObjectName("delegateEdit");
    m_textEdit->setObjectName("textEdit");
    m_startDateEdit->setObjectName("startDateEdit");
    m_dueDateEdit->setObjectName("dueDateEdit");
    m_doneButton->setObjectName("doneButton");
    m_startTodayButton->setObjectName("startTodayButton");

    m_startDateEdit->setMinimumContentsLength(10);
    m_dueDateEdit->setMinimumContentsLength(10);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_delegateLabel);
    layout->addWidget(m_textEdit);
    layout->addWidget(m_taskGroup);
    setLayout(layout);

    auto vbox = new QVBoxLayout;
    auto delegateHBox = new QHBoxLayout;
    delegateHBox->addWidget(new QLabel(tr("Delegate to"), m_taskGroup));
    delegateHBox->addWidget(m_delegateEdit);
    vbox->addLayout(delegateHBox);
    auto datesHBox = new QHBoxLayout;
    datesHBox->addWidget(new QLabel(tr("Start date"), m_taskGroup));
    datesHBox->addWidget(m_startDateEdit, 1);
    datesHBox->addWidget(new QLabel(tr("Due date"), m_taskGroup));
    datesHBox->addWidget(m_dueDateEdit, 1);
    vbox->addLayout(datesHBox);
    auto bottomHBox = new QHBoxLayout;
    bottomHBox->addWidget(m_startTodayButton);
    bottomHBox->addWidget(m_doneButton);
    bottomHBox->addStretch();
    vbox->addLayout(bottomHBox);
    m_taskGroup->setLayout(vbox);

    // Make sure our minimum width is always the one with
    // the task group visible
    layout->activate();
    setMinimumWidth(minimumSizeHint().width());

    m_delegateLabel->setVisible(false);
    m_taskGroup->setVisible(false);

    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
    connect(m_startDateEdit, SIGNAL(dateEntered(QDate)), this, SLOT(onStartEditEntered(QDate)));
    connect(m_dueDateEdit, SIGNAL(dateEntered(QDate)), this, SLOT(onDueEditEntered(QDate)));
    connect(m_doneButton, SIGNAL(toggled(bool)), this, SLOT(onDoneButtonChanged(bool)));
    connect(m_startTodayButton, SIGNAL(clicked()), this, SLOT(onStartTodayClicked()));
    connect(m_delegateEdit, SIGNAL(returnPressed()), this, SLOT(onDelegateEntered()));

    setEnabled(false);
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
        disconnect(m_model, Q_NULLPTR, this, Q_NULLPTR);
        disconnect(this, Q_NULLPTR, m_model, Q_NULLPTR);
    }

    m_model = model;

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

void EditorView::onArtifactChanged()
{
    auto artifact = m_model->property("artifact").value<Domain::Artifact::Ptr>();
    setEnabled(artifact);
    m_delegateEdit->clear();
}

void EditorView::onHasTaskPropertiesChanged()
{
    m_taskGroup->setVisible(m_model->property("hasTaskProperties").toBool());
}

void EditorView::onTextOrTitleChanged()
{
    const QString text = m_model->property("title").toString()
                       + "\n"
                       + m_model->property("text").toString();

    if (text != m_textEdit->toPlainText())
        m_textEdit->setPlainText(text);
}

void EditorView::onStartDateChanged()
{
    m_startDateEdit->setDate(m_model->property("startDate").toDateTime().date());
}

void EditorView::onDueDateChanged()
{
    m_dueDateEdit->setDate(m_model->property("dueDate").toDateTime().date());
}

void EditorView::onDoneChanged()
{
    m_doneButton->setChecked(m_model->property("done").toBool());
}

void EditorView::onDelegateTextChanged()
{
    const auto delegateText = m_model->property("delegateText").toString();
    const auto labelText = delegateText.isEmpty() ? QString()
                         : tr("Delegated to: <b>%1</b>").arg(delegateText);

    m_delegateLabel->setVisible(!labelText.isEmpty());
    m_delegateLabel->setText(labelText);
}

void EditorView::onTextEditChanged()
{
    const QString plainText = m_textEdit->toPlainText();
    const int index = plainText.indexOf('\n');
    const QString title = plainText.left(index);
    const QString text = plainText.mid(index + 1);
    emit titleChanged(title);
    emit textChanged(text);
}

void EditorView::onStartEditEntered(const QDate &start)
{
    emit startDateChanged(QDateTime(start));
}

void EditorView::onDueEditEntered(const QDate &due)
{
    emit dueDateChanged(QDateTime(due));
}

void EditorView::onDoneButtonChanged(bool checked)
{
    emit doneChanged(checked);
}

void EditorView::onStartTodayClicked()
{
    QDate today(QDate::currentDate());
    m_startDateEdit->setDate(today);
    emit startDateChanged(QDateTime(today));
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
