/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_EDITORMODEL_H
#define PRESENTATION_EDITORMODEL_H

#include <QDate>
#include <QObject>

#include <functional>

#include "domain/task.h"

#include "presentation/errorhandlingmodelbase.h"

class QAbstractItemModel;
class QTimer;

namespace Presentation {

class AttachmentModel;

class EditorModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(Domain::Task::Ptr task READ task WRITE setTask NOTIFY taskChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool done READ isDone WRITE setDone NOTIFY doneChanged)
    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(Domain::Task::Recurrence recurrence READ recurrence WRITE setRecurrence NOTIFY recurrenceChanged)
    Q_PROPERTY(QAbstractItemModel* attachmentModel READ attachmentModel CONSTANT)
    Q_PROPERTY(bool editingInProgress READ editingInProgress WRITE setEditingInProgress)

public:
    typedef std::function<KJob*(const Domain::Task::Ptr &)> SaveFunction;

    explicit EditorModel(QObject *parent = nullptr);
    ~EditorModel();

    Domain::Task::Ptr task() const;
    void setTask(const Domain::Task::Ptr &task);

    bool hasSaveFunction() const;
    void setSaveFunction(const SaveFunction &function);

    QString text() const;
    QString title() const;
    bool isDone() const;
    QDate startDate() const;
    QDate dueDate() const;
    Domain::Task::Recurrence recurrence() const;
    QAbstractItemModel *attachmentModel() const;

    static int autoSaveDelay();
    static void setAutoSaveDelay(int delay);

    bool editingInProgress() const;

public Q_SLOTS:
    void setText(const QString &text);
    void setTitle(const QString &title);
    void setDone(bool done);
    void setStartDate(const QDate &start);
    void setDueDate(const QDate &due);
    void setRecurrence(Domain::Task::Recurrence recurrence);

    void addAttachment(const QString &fileName);
    void removeAttachment(const QModelIndex &index);
    void openAttachment(const QModelIndex &index);

    void setEditingInProgress(bool editingInProgress);

signals:
    void taskChanged(const Domain::Task::Ptr &task);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDate &date);
    void dueDateChanged(const QDate &due);
    void recurrenceChanged(Domain::Task::Recurrence recurrence);

private Q_SLOTS:
    void onTextChanged(const QString &text);
    void onTitleChanged(const QString &title);
    void onDoneChanged(bool done);
    void onStartDateChanged(const QDate &start);
    void onDueDateChanged(const QDate &due);
    void onRecurrenceChanged(Domain::Task::Recurrence recurrence);

    void save();

private:
    void setSaveNeeded(bool needed);
    bool isSaveNeeded() const;
    void applyNewText(const QString &text);
    void applyNewTitle(const QString &title);
    void applyNewDone(bool done);
    void applyNewStartDate(const QDate &start);
    void applyNewDueDate(const QDate &due);
    void applyNewRecurrence(Domain::Task::Recurrence recurrence);

    Domain::Task::Ptr m_task;
    SaveFunction m_saveFunction;

    QString m_text;
    QString m_title;
    bool m_done;
    QDate m_start;
    QDate m_due;
    Domain::Task::Recurrence m_recurrence;
    AttachmentModel *m_attachmentModel;

    QTimer *m_saveTimer;
    bool m_saveNeeded;
    bool m_editingInProgress;
};

}

#endif // PRESENTATION_EDITORMODEL_H
