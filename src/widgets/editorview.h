/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_EDITORVIEW_H
#define WIDGETS_EDITORVIEW_H

#include <QWidget>

#include <QDate>

#include <functional>

#include "domain/task.h"

class QAbstractButton;
class QLabel;
class QPlainTextEdit;

class KDateComboBox;
class KLineEdit;

namespace Ui {
    class EditorView;
}

namespace Widgets {

class EditorView : public QWidget
{
    Q_OBJECT
public:
    typedef std::function<QString(QWidget*)> RequestFileNameFunction;

    explicit EditorView(QWidget *parent = nullptr);
    ~EditorView();

    QObject *model() const;
    RequestFileNameFunction requestFileNameFunction() const;

public slots:
    void setModel(QObject *model);
    void setRequestFileNameFunction(const RequestFileNameFunction &function);

signals:
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void startDateChanged(const QDate &start);
    void dueDateChanged(const QDate &due);
    void doneChanged(bool done);
    void recurrenceChanged(Domain::Task::Recurrence recurrence);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTaskChanged();
    void onTextOrTitleChanged();
    void onStartDateChanged();
    void onDueDateChanged();
    void onDoneChanged();
    void onRecurrenceChanged();

    void onTextEditChanged();
    void onStartEditEntered(const QDate &start);
    void onDueEditEntered(const QDate &due);
    void onDoneButtonChanged(bool checked);
    void onStartTodayClicked();
    void onRecurrenceComboChanged(int index);

    void onAttachmentSelectionChanged();
    void onAddAttachmentClicked();
    void onRemoveAttachmentClicked();
    void onAttachmentDoubleClicked(const QModelIndex &index);

private:
    QObject *m_model;
    RequestFileNameFunction m_requestFileNameFunction;

    Ui::EditorView *ui;
};

}

#endif // WIDGETS_EDITORVIEW_H
