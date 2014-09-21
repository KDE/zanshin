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

#include <QDateTime>

class QAbstractButton;
class QPlainTextEdit;

namespace KPIM {
    class KDateEdit;
}

namespace Widgets {

class EditorView : public QWidget
{
    Q_OBJECT
public:
    explicit EditorView(QWidget *parent = 0);

    QObject *model() const;

public slots:
    void setModel(QObject *model);

signals:
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void startDateChanged(const QDateTime &start);
    void dueDateChanged(const QDateTime &due);
    void doneChanged(bool done);

private slots:
    void onArtifactChanged();
    void onHasTaskPropertiesChanged();
    void onTextOrTitleChanged();
    void onStartDateChanged();
    void onDueDateChanged();
    void onDoneChanged();

    void onTextEditChanged();
    void onStartEditEntered(const QDate &start);
    void onDueEditEntered(const QDate &due);
    void onDoneButtonChanged(bool checked);
    void onStartTodayClicked();

private:
    QObject *m_model;

    QPlainTextEdit *m_textEdit;
    QWidget *m_taskGroup;
    KPIM::KDateEdit *m_startDateEdit;
    KPIM::KDateEdit *m_dueDateEdit;
    QAbstractButton *m_startTodayButton;
    QAbstractButton *m_doneButton;
};

}

#endif // WIDGETS_EDITORVIEW_H
