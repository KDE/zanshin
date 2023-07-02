/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "editormodel.h"

#include <QAbstractListModel>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QMimeDatabase>
#include <QTemporaryFile>
#include <QTimer>

#include <KLocalizedString>

#include "domain/task.h"
#include "errorhandler.h"

namespace Presentation {
class AttachmentModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AttachmentModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    void setTask(const Domain::Task::Ptr &task)
    {
        if (m_task == task)
            return;

        beginResetModel();
        if (m_task) {
            disconnect(m_task.data(), &Domain::Task::attachmentsChanged,
                       this, &AttachmentModel::triggerReset);
        }
        m_task = task;
        if (m_task) {
            connect(m_task.data(), &Domain::Task::attachmentsChanged,
                    this, &AttachmentModel::triggerReset);
        }
        endResetModel();
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid() || !m_task)
            return 0;

        return m_task->attachments().size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant();

        auto attachment = m_task->attachments().at(index.row());

        switch (role) {
        case Qt::DisplayRole:
            return attachment.label();
        case Qt::DecorationRole:
            return QVariant::fromValue(QIcon::fromTheme(attachment.iconName()));
        default:
            return QVariant();
        }
    }

private slots:
    void triggerReset()
    {
        beginResetModel();
        endResetModel();
    }

private:
    Domain::Task::Ptr m_task;
};
}

using namespace Presentation;

static int s_autoSaveDelay = 500;

EditorModel::EditorModel(QObject *parent)
    : QObject(parent),
      m_done(false),
      m_recurrence(Domain::Task::NoRecurrence),
      m_attachmentModel(new AttachmentModel(this)),
      m_saveTimer(new QTimer(this)),
      m_saveNeeded(false),
      m_editingInProgress(false)
{
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, &QTimer::timeout, this, &EditorModel::save);
}

EditorModel::~EditorModel()
{
    save();
}

Domain::Task::Ptr EditorModel::task() const
{
    return m_task;
}

void EditorModel::setTask(const Domain::Task::Ptr &task)
{
    if (m_task == task)
        return;

    save();

    m_text = QString();
    m_title = QString();
    m_done = false;
    m_start = QDate();
    m_due = QDate();
    m_recurrence = Domain::Task::NoRecurrence;
    m_attachmentModel->setTask(Domain::Task::Ptr());

    if (m_task)
        disconnect(m_task.data(), nullptr, this, nullptr);

    m_task = task;

    if (m_task) {
        m_text = m_task->text();
        m_title = m_task->title();
        m_done = m_task->isDone();
        m_start = m_task->startDate();
        m_due = m_task->dueDate();
        m_recurrence = m_task->recurrence();
        m_attachmentModel->setTask(m_task);

        connect(m_task.data(), &Domain::Task::textChanged, this, &EditorModel::onTextChanged);
        connect(m_task.data(), &Domain::Task::titleChanged, this, &EditorModel::onTitleChanged);
        connect(m_task.data(), &Domain::Task::doneChanged, this, &EditorModel::onDoneChanged);
        connect(m_task.data(), &Domain::Task::startDateChanged, this, &EditorModel::onStartDateChanged);
        connect(m_task.data(), &Domain::Task::dueDateChanged, this, &EditorModel::onDueDateChanged);
        connect(m_task.data(), &Domain::Task::recurrenceChanged, this, &EditorModel::onRecurrenceChanged);
    }


    emit textChanged(m_text);
    emit titleChanged(m_title);
    emit doneChanged(m_done);
    emit startDateChanged(m_start);
    emit dueDateChanged(m_due);
    emit recurrenceChanged(m_recurrence);
    emit taskChanged(m_task);
}

bool EditorModel::hasSaveFunction() const
{
    return bool(m_saveFunction);
}

void EditorModel::setSaveFunction(const SaveFunction &function)
{
    m_saveFunction = function;
}

QString EditorModel::text() const
{
    return m_text;
}

QString EditorModel::title() const
{
    return m_title;
}

bool EditorModel::isDone() const
{
    return m_done;
}

QDate EditorModel::startDate() const
{
    return m_start;
}

QDate EditorModel::dueDate() const
{
    return m_due;
}

Domain::Task::Recurrence EditorModel::recurrence() const
{
    return m_recurrence;
}

QAbstractItemModel *EditorModel::attachmentModel() const
{
    return m_attachmentModel;
}

int EditorModel::autoSaveDelay()
{
    return s_autoSaveDelay;
}

void EditorModel::setAutoSaveDelay(int delay)
{
    s_autoSaveDelay = delay;
}

bool EditorModel::editingInProgress() const
{
    return m_editingInProgress;
}

void EditorModel::setText(const QString &text)
{
    if (m_text == text)
        return;
    applyNewText(text);
    setSaveNeeded(true);
}

void EditorModel::setTitle(const QString &title)
{
    if (m_title == title)
        return;
    applyNewTitle(title);
    setSaveNeeded(true);
}

void EditorModel::setDone(bool done)
{
    if (m_done == done)
        return;
    applyNewDone(done);
    setSaveNeeded(true);
}

void EditorModel::setStartDate(const QDate &start)
{
    if (m_start == start)
        return;
    applyNewStartDate(start);
    setSaveNeeded(true);
}

void EditorModel::setDueDate(const QDate &due)
{
    if (m_due == due)
        return;
    applyNewDueDate(due);
    setSaveNeeded(true);
}

void EditorModel::setRecurrence(Domain::Task::Recurrence recurrence)
{
    if (m_recurrence == recurrence)
        return;
    applyNewRecurrence(recurrence);
    setSaveNeeded(true);
}

void EditorModel::addAttachment(const QString &fileName)
{
    if (!m_task)
        return;

    QMimeDatabase mimeDb;
    auto mimeType = mimeDb.mimeTypeForFile(fileName);

    auto attachment = Domain::Task::Attachment();
    attachment.setLabel(QFileInfo(fileName).fileName());
    attachment.setMimeType(mimeType.name());
    attachment.setIconName(mimeType.iconName());

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        // TODO: Might be worth extending error handling
        // to deal with job-less errors later on
        qWarning() << "Couldn't open" << fileName;
        return;
    }

    attachment.setData(file.readAll());

    file.close();

    auto attachments = m_task->attachments();
    attachments.append(attachment);
    m_task->setAttachments(attachments);

    setSaveNeeded(true);
}

void EditorModel::removeAttachment(const QModelIndex &index)
{
    if (!m_task)
        return;

    auto attachments = m_task->attachments();
    attachments.removeAt(index.row());
    m_task->setAttachments(attachments);

    setSaveNeeded(true);
}

void EditorModel::openAttachment(const QModelIndex &index)
{
    Q_ASSERT(m_task);
    auto attachment = m_task->attachments().at(index.row());

    auto uri = attachment.uri();
    if (!attachment.isUri()) {
        auto tempFile = new QTemporaryFile(QDir::tempPath() + QStringLiteral("/zanshin_attachment_XXXXXX"), this);
        tempFile->open();
        tempFile->setPermissions(QFile::ReadUser);
        tempFile->write(attachment.data());
        tempFile->close();
        uri = QUrl::fromLocalFile(tempFile->fileName());
    }

    QDesktopServices::openUrl(uri);
}

void EditorModel::setEditingInProgress(bool editing)
{
    m_editingInProgress = editing;
}

void EditorModel::onTextChanged(const QString &text)
{
    if (!m_editingInProgress)
        applyNewText(text);
}

void EditorModel::onTitleChanged(const QString &title)
{
    if (!m_editingInProgress)
        applyNewTitle(title);
}

void EditorModel::onDoneChanged(bool done)
{
    if (!m_editingInProgress)
        applyNewDone(done);
}

void EditorModel::onStartDateChanged(const QDate &start)
{
    if (!m_editingInProgress)
        applyNewStartDate(start);
}

void EditorModel::onDueDateChanged(const QDate &due)
{
    if (!m_editingInProgress)
        applyNewDueDate(due);
}

void EditorModel::onRecurrenceChanged(Domain::Task::Recurrence recurrence)
{
    if (!m_editingInProgress)
        applyNewRecurrence(recurrence);
}

void EditorModel::save()
{
    if (!isSaveNeeded())
        return;

    Q_ASSERT(m_task);

    const auto currentTitle = m_task->title();
    m_task->setTitle(m_title);
    m_task->setText(m_text);

    m_task->setDone(m_done);
    m_task->setStartDate(m_start);
    m_task->setDueDate(m_due);
    m_task->setRecurrence(m_recurrence);

    const auto job = m_saveFunction(m_task);
    installHandler(job, i18n("Cannot modify task %1", currentTitle));
    setSaveNeeded(false);
}

void EditorModel::setSaveNeeded(bool needed)
{
    if (needed)
        m_saveTimer->start(autoSaveDelay());
    else
        m_saveTimer->stop();

    m_saveNeeded = needed;
}

bool EditorModel::isSaveNeeded() const
{
    return m_saveNeeded;
}

void EditorModel::applyNewText(const QString &text)
{
    m_text = text;
    emit textChanged(m_text);
}

void EditorModel::applyNewTitle(const QString &title)
{
    m_title = title;
    emit titleChanged(m_title);
}

void EditorModel::applyNewDone(bool done)
{
    m_done = done;
    emit doneChanged(m_done);
}

void EditorModel::applyNewStartDate(const QDate &start)
{
    m_start = start;
    emit startDateChanged(m_start);
}

void EditorModel::applyNewDueDate(const QDate &due)
{
    m_due = due;
    emit dueDateChanged(m_due);
}

void EditorModel::applyNewRecurrence(Domain::Task::Recurrence recurrence)
{
    m_recurrence = recurrence;
    emit recurrenceChanged(m_recurrence);
}

#include "editormodel.moc"

#include "moc_editormodel.cpp"
