/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#ifndef EDITABLEWIDGET_H
#define EDITABLEWIDGET_H

#include <QWidget>

#include <KDateTime>

class KSqueezedTextLabel;
class QCheckBox;
class QToolButton;
class KDateTimeWidget;
class QHBoxLayout;
class QLineEdit;


class AbstractEditableWidget: public QWidget
{
    Q_OBJECT
public:
    explicit AbstractEditableWidget(QWidget *contentWidget, QWidget* parent = 0);
    virtual ~AbstractEditableWidget(){};
    ///Clear label and edit widget
    virtual void clear();
signals:
    void valueChanged();

public slots:
    virtual void buttonPressed();
    ///update edit widget content
    virtual void edit();
    ///update display widget content
    virtual void display();

protected:
    QWidget *m_contentWidget;
    KSqueezedTextLabel *m_label;
    QToolButton *m_button;
    QHBoxLayout *m_layout;
    bool m_editMode;
};

/**
 * A Widget which shows the date as label
 * It can switch to editmode via button
 */
class EditableDate: public AbstractEditableWidget
{
    Q_OBJECT
public:
    explicit EditableDate(QWidget* parent = 0);
    virtual ~EditableDate(){};

    void clear();
    KDateTime dateTime();
signals:
    void dateChanged(KDateTime);
public slots:
    void setDate(const KDateTime &date);
public slots:
    virtual void buttonPressed();
    void display();
protected:
    KDateTimeWidget *m_dateTimeWidget;
};

/**
 * Offers additionally a checkbox to enable/disable the date
 */
class CheckableEditableDate: public EditableDate
{
    Q_OBJECT
public:
    explicit CheckableEditableDate(QWidget* parent = 0);
    virtual ~CheckableEditableDate(){};

    bool isEnabled();
signals:
    void dateChanged(KDateTime, bool);
public slots:
    void enable(bool enable);
public slots:
    virtual void buttonPressed();
private slots:
    void checkStatusChanged(bool);
private:
    QCheckBox *m_checkBox;
    bool m_editMode;
    bool m_isEnabled;
};

/**
 * An Editable which is displayed as label
 */

class EditableString: public AbstractEditableWidget
{
    Q_OBJECT
public:
    explicit EditableString(QWidget* parent = 0);
    virtual ~EditableString(){};

    void clear();
    QString text();
    QLineEdit &lineEdit();
    void setDisplayFont(const QFont &);
signals:
    void textChanged(QString);
public slots:
    void setText(const QString &);
protected slots:
    virtual void buttonPressed();
    void display();
protected:
    QLineEdit *m_lineEdit;
};

#endif // EDITABLEWIDGET_H
