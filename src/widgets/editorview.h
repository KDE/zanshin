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


#ifndef WIDGETS_EDITORVIEW_H
#define WIDGETS_EDITORVIEW_H

#include <QWidget>

#include <QDate>

#include <functional>

#include "domain/task.h"

class QAbstractButton;
class QLabel;
class QPlainTextEdit;

class KLineEdit;

namespace KPIM {
    class KDateEdit;
}

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
