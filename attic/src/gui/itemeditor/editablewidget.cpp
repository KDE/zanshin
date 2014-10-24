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


#include "editablewidget.h"

#include "kdateedit.h"
#include "utils/datestringbuilder.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QCheckBox>

#include <KDebug>
#include <klocalizedstring.h>
#include <KDE/KIconLoader>
#include <KSqueezedTextLabel>
#include <QLineEdit>



AbstractEditableWidget::AbstractEditableWidget(QWidget *contentWidget, QWidget* parent)
:   QWidget(parent),
    m_contentWidget(contentWidget)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);

    m_label = new KSqueezedTextLabel(this);
    m_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_label->setTextElideMode(Qt::ElideRight);
    m_layout->addWidget(m_label);

    m_layout->addWidget(contentWidget);

    m_button = new QToolButton(this);
    m_button->setCheckable(false);
    //m_button->setIconSize(QSize(10,10));
    //m_button->setI (Qt::ToolButtonIconOnly);
    connect(m_button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
    m_layout->addWidget(m_button);

    m_layout->addStretch();
    setLayout(m_layout);

    display();
}

void AbstractEditableWidget::clear()
{
    m_label->clear();
    display();
}



void AbstractEditableWidget::edit()
{
    if (m_editMode) {
        return;
    }
    m_label->hide();
    m_contentWidget->show();
    m_editMode = true;
    m_button->setIcon(QIcon(SmallIcon("document-save")));
}

void AbstractEditableWidget::display()
{
    m_contentWidget->hide();
    m_label->show();
    m_editMode = false;
    m_button->setIcon(QIcon(SmallIcon("document-edit")));
}

void AbstractEditableWidget::buttonPressed()
{
    if (!m_editMode) {
        edit();
    } else {
        display();
        emit valueChanged();
    }
}




EditableDate::EditableDate(QWidget* parent)
:   AbstractEditableWidget(new KPIM::KDateEdit(), parent),
    m_dateTimeWidget(static_cast<KPIM::KDateEdit*>(m_contentWidget))

{
    m_dateTimeWidget->setParent(this);
}

void EditableDate::clear()
{
    AbstractEditableWidget::clear();
    m_dateTimeWidget->setDate(QDate::currentDate());
}

void EditableDate::display()
{
    m_label->setText(DateStringBuilder::getFullDate(KDateTime(m_dateTimeWidget->date())));
    AbstractEditableWidget::display();
}

void EditableDate::buttonPressed()
{
    if (m_editMode) {
        emit dateChanged(KDateTime(m_dateTimeWidget->date()));
    }
    AbstractEditableWidget::buttonPressed();
}

KDateTime EditableDate::dateTime()
{
    return KDateTime( m_dateTimeWidget->date());
}

void EditableDate::setDate(const KDateTime& date)
{
    m_label->setText(DateStringBuilder::getFullDate(date));
    m_dateTimeWidget->setDate(date.date());
}




CheckableEditableDate::CheckableEditableDate(QWidget* parent)
:   EditableDate(parent)
{
    m_checkBox = new QCheckBox(this);
    m_layout->insertWidget(0, m_checkBox);
    connect(m_checkBox, SIGNAL(clicked(bool)), this, SLOT(checkStatusChanged(bool)));
}

void CheckableEditableDate::enable(bool enable)
{
    m_label->setEnabled(enable);
    m_dateTimeWidget->setEnabled(enable);
    m_button->setEnabled(enable);
    m_isEnabled = enable;
    m_checkBox->setChecked(enable);
    if (!enable) {
        m_checkBox->setText(i18n("&Enable"));
        m_button->hide();
        m_label->hide();
    } else {
        m_checkBox->setText("");
        m_button->show();
        m_label->show();
    }
}

bool CheckableEditableDate::isEnabled()
{
    return m_isEnabled;
}

void CheckableEditableDate::checkStatusChanged(bool status)
{
    if (status) {
        enable(true);
        edit();
    } else {
        display();
        enable(false);
        emit dateChanged(KDateTime());
    }

}

void CheckableEditableDate::buttonPressed()
{
    if (m_editMode) {
        emit dateChanged(KDateTime(m_dateTimeWidget->date()));
    }
    EditableDate::buttonPressed();
}



EditableString::EditableString(QWidget* parent)
:   AbstractEditableWidget(new QLineEdit(), parent),
    m_lineEdit(static_cast<QLineEdit*>(m_contentWidget))
{
    m_contentWidget->setParent(this);
    connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(buttonPressed()));

}

void EditableString::clear()
{
    AbstractEditableWidget::clear();
    m_lineEdit->clear();
}

void EditableString::display()
{
    m_label->setText(m_lineEdit->text());
    AbstractEditableWidget::display();
}

void EditableString::buttonPressed()
{
    if (m_editMode) {
        emit textChanged(m_lineEdit->text());
    }
    AbstractEditableWidget::buttonPressed();
}

QString EditableString::text()
{
    return m_lineEdit->text();
}

void EditableString::setText( const QString &text)
{
    m_label->setText(text);
    m_lineEdit->setText(text);
}

QLineEdit& EditableString::lineEdit()
{
    return *m_lineEdit;
}

void EditableString::setDisplayFont( const QFont &font)
{
    m_label->setFont(font);
}



