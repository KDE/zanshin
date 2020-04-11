/*
  This file is part of libkdepim.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

//krazy:excludeall=qclasses as we want to subclass from QComboBox, not KComboBox

#include "kdateedit.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QMouseEvent>
#include <QValidator>

#include <KLocalizedString>

using namespace KPIM;

class DateFormat
{
public:
    DateFormat()
    {
        // Check if we can use the QLocale::ShortFormat
        if (!QLocale().toString(QDate(2015, 1, 1), QLocale::ShortFormat).contains(QStringLiteral("2015"))) {
            mFallbackFormat = QStringLiteral("dd/MM/yyyy");
        }
    }

    QDate toDate( const QString &string )
    {
        return mFallbackFormat.isEmpty() ? QLocale().toDate( string, QLocale::ShortFormat )
                                         : QLocale().toDate( string, mFallbackFormat );
    }

    QString toString( const QDate &date )
    {
        return mFallbackFormat.isEmpty() ? QLocale().toString( date, QLocale::ShortFormat )
                                         : QLocale().toString( date, mFallbackFormat );
    }

private:
    QString mFallbackFormat;
};
Q_GLOBAL_STATIC(DateFormat, sDateFormat);

class DateValidator : public QValidator
{
  public:
    DateValidator( const QStringList &keywords, QWidget *parent )
      : QValidator( parent ), mKeywords( keywords )
    {}

    virtual State validate( QString &str, int & ) const override
    {
      int length = str.length();

      // empty string is intermediate so one can clear the edit line and start from scratch
      if ( length <= 0 ) {
        return Intermediate;
      }

      if ( mKeywords.contains( str.toLower() ) ) {
        return Acceptable;
      }

      QDate result = sDateFormat->toDate( str );
      if ( result.isValid() ) {
        return Acceptable;
      } else {
        return Intermediate;
      }
    }

  private:
    QStringList mKeywords;
};

KDateEdit::KDateEdit( QWidget *parent )
  : QComboBox( parent ), mReadOnly( false ), mDiscardNextMousePress( false )
{
  // need at least one entry for popup to work
  setMaxCount( 1 );
  setEditable( true );

  mDate = QDate::currentDate();
  QString today = sDateFormat->toString( mDate );

  addItem( today );
  setCurrentIndex( 0 );

  connect( lineEdit(), SIGNAL(returnPressed()),
           this, SLOT(lineEnterPressed()) );
  connect( this, SIGNAL(editTextChanged(QString)),
           SLOT(slotTextChanged(QString)) );

  mPopup = new KDatePickerPopup( KDatePickerPopup::DatePicker | KDatePickerPopup::Words,
                                 QDate::currentDate(), this );
  mPopup->hide();
  mPopup->installEventFilter( this );

  connect( mPopup, SIGNAL(dateChanged(QDate)),
           SLOT(dateSelected(QDate)) );

  // handle keyword entry
  setupKeywords();
  lineEdit()->installEventFilter( this );

  setValidator( new DateValidator( mKeywordMap.keys(), this ) );

  mTextChanged = false;
}

KDateEdit::~KDateEdit()
{
}

void KDateEdit::setDate( const QDate &date )
{
  assignDate( date );
  updateView();
}

QDate KDateEdit::date() const
{
  return mDate;
}

void KDateEdit::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  lineEdit()->setReadOnly( readOnly );
}

bool KDateEdit::isReadOnly() const
{
  return mReadOnly;
}

void KDateEdit::showPopup()
{
  if ( mReadOnly ) {
    return;
  }

  QRect desk = QApplication::desktop()->screenGeometry( this );

  QPoint popupPoint = mapToGlobal( QPoint( 0, 0 ) );

  int dateFrameHeight = mPopup->sizeHint().height();
  if ( popupPoint.y() + height() + dateFrameHeight > desk.bottom() ) {
    popupPoint.setY( popupPoint.y() - dateFrameHeight );
  } else {
    popupPoint.setY( popupPoint.y() + height() );
  }

  int dateFrameWidth = mPopup->sizeHint().width();
  if ( popupPoint.x() + dateFrameWidth > desk.right() ) {
    popupPoint.setX( desk.right() - dateFrameWidth );
  }

  if ( popupPoint.x() < desk.left() ) {
    popupPoint.setX( desk.left() );
  }

  if ( popupPoint.y() < desk.top() ) {
    popupPoint.setY( desk.top() );
  }

  if ( mDate.isValid() ) {
    mPopup->setDate( mDate );
  } else {
    mPopup->setDate( QDate::currentDate() );
  }

  mPopup->popup( popupPoint );

  // The combo box is now shown pressed. Make it show not pressed again
  // by causing its (invisible) list box to emit a 'selected' signal.
  // First, ensure that the list box contains the date currently displayed.
  QDate date = parseDate();
  assignDate( date );
  updateView();

  // Now, simulate an Enter to unpress it
  QAbstractItemView *lb = view();
  if ( lb ) {
    lb->setCurrentIndex( lb->model()->index( 0, 0 ) );
    QKeyEvent *keyEvent =
      new QKeyEvent( QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier );
    QApplication::postEvent( lb, keyEvent );
  }
}

void KDateEdit::dateSelected( const QDate &date )
{
  if ( assignDate( date ) ) {
    updateView();
    emit dateChanged( date );
    emit dateEntered( date );

    if ( date.isValid() ) {
      mPopup->hide();
    }
  }
}

void KDateEdit::lineEnterPressed()
{
  bool replaced = false;

  QDate date = parseDate( &replaced );

  if ( assignDate( date ) ) {
    if ( replaced ) {
      updateView();
    }

    emit dateChanged( date );
    emit dateEntered( date );
  }
}

QDate KDateEdit::parseDate( bool *replaced ) const
{
  QString text = currentText();
  QDate result;

  if ( replaced ) {
    (*replaced) = false;
  }

  if ( text.isEmpty() ) {
    result = QDate();
  } else if ( mKeywordMap.contains( text.toLower() ) ) {
    QDate today = QDate::currentDate();
    int i = mKeywordMap.value( text.toLower() );
    if ( i == 30 ) {
      // Special case for "one month"
      result = today.addMonths( 1 );
    } else {
      if ( i >= 100 ) {
        /* A day name has been entered. Convert to offset from today.
         * This uses some math tricks to figure out the offset in days
         * to the next date the given day of the week occurs. There
         * are two cases, that the new day is >= the current day, which means
         * the new day has not occurred yet or that the new day < the current day,
         * which means the new day is already passed (so we need to find the
         * day in the next week).
         */
        i -= 100;
        int currentDay = today.dayOfWeek();
        if ( i >= currentDay ) {
          i -= currentDay;
        } else {
          i += 7 - currentDay;
        }
      }

      result = today.addDays( i );
    }
    if ( replaced ) {
        (*replaced) = true;
    }
  } else {
      result = sDateFormat->toDate( text );
  }

  return result;
}

void KDateEdit::focusOutEvent( QFocusEvent *e )
{
  if ( mTextChanged ) {
    lineEnterPressed();
    mTextChanged = false;
  }
  QComboBox::focusOutEvent( e );
}

void KDateEdit::wheelEvent( QWheelEvent *e )
{
  if ( mReadOnly || e->delta() == 0 ) {
    return;
  }
  QDate date = parseDate();
  if ( !date.isValid() ) {
    return;
  }
  // QWheelEvent::delta reports +/-120 per step on most mice, but according to the documentation some
  // mice may send smaller deltas. We're just interested in the direction, though...
  date = date.addDays( e->delta() > 0 ? 1 : -1 );
  if ( assignDate( date ) ) {
    e->accept();
    updateView();
    emit dateChanged( date );
    emit dateEntered( date );
    return;
  }
  QComboBox::wheelEvent( e );
}

void KDateEdit::keyPressEvent(QKeyEvent* e)
{
  QDate date;

  if ( !mReadOnly ) {
    switch ( e->key() ) {
    case Qt::Key_Up:
      date = parseDate();
      if (!date.isValid()) break;
      date = date.addDays( 1 );
      break;
    case Qt::Key_Down:
      date = parseDate();
      if (!date.isValid()) break;
      date = date.addDays( -1 );
      break;
    case Qt::Key_PageUp:
      date = parseDate();
      if (!date.isValid()) break;
      date = date.addMonths( 1 );
      break;
    case Qt::Key_PageDown:
      date = parseDate();
      if (!date.isValid()) break;
      date = date.addMonths( -1 );
      break;
    case Qt::Key_Equal:
      date = QDate::currentDate();
      break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      // When the date is selected and the Key_Return/Key_Enter pressed, the value in the
      // QLineEdit is set to the value of the first item in the QCompleter, here "friday".
      lineEdit()->deselect();
      break;
    }

    if ( date.isValid() && assignDate( date ) ) {
      e->accept();
      updateView();
      emit dateChanged( date );
      emit dateEntered( date );
      return;
    }
  }

  QComboBox::keyPressEvent( e );
}

bool KDateEdit::eventFilter( QObject *object, QEvent *event )
{
  if ( object == lineEdit() ) {
    // We only process the focus out event if the text has changed
    // since we got focus
    if ( ( event->type() == QEvent::FocusOut ) && mTextChanged ) {
      lineEnterPressed();
      mTextChanged = false;
    } else if ( event->type() == QEvent::KeyPress ) {
      // Up and down arrow keys step the date
      QKeyEvent *keyEvent = (QKeyEvent *)event;

      if ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ) {
        lineEnterPressed();
        return true;
      }
    }
  } else {
    // It's a date picker event
    switch ( event->type() ) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    {
      QMouseEvent *mouseEvent = (QMouseEvent*)event;
      if ( !mPopup->rect().contains( mouseEvent->pos() ) ) {
        QPoint globalPos = mPopup->mapToGlobal( mouseEvent->pos() );
        if ( QApplication::widgetAt( globalPos ) == this ) {
          // The date picker is being closed by a click on the
          // KDateEdit widget. Avoid popping it up again immediately.
          mDiscardNextMousePress = true;
        }
      }

      break;
    }
    default:
      break;
    }
  }

  return false;
}

void KDateEdit::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && mDiscardNextMousePress ) {
    mDiscardNextMousePress = false;
    return;
  }

  QComboBox::mousePressEvent( event );
}

void KDateEdit::slotTextChanged( const QString & )
{
  QDate date = parseDate();

  if ( assignDate( date ) ) {
    emit dateChanged( date );
  }

  mTextChanged = true;
}

void KDateEdit::setupKeywords()
{
  // Create the keyword list. This will be used to match against when the user
  // enters information.
  mKeywordMap.insert( i18nc( "the day after today", "tomorrow" ), 1 );
  mKeywordMap.insert( i18nc( "this day", "today" ), 0 );
  mKeywordMap.insert( i18nc( "the day before today", "yesterday" ), -1 );
  mKeywordMap.insert( i18nc( "the week after this week", "next week" ), 7 );
  mKeywordMap.insert( i18nc( "the month after this month", "next month" ), 30 );

  QString dayName;
  for ( int i = 1; i <= 7; ++i ) {
    dayName = QLocale().standaloneDayName( i ).toLower();
    mKeywordMap.insert( dayName, i + 100 );
  }

  QCompleter *comp = new QCompleter( mKeywordMap.keys(), this );
  comp->setCaseSensitivity( Qt::CaseInsensitive );
  comp->setCompletionMode( QCompleter::InlineCompletion );
  setCompleter( comp );
}

bool KDateEdit::assignDate( const QDate &date )
{
  mDate = date;
  mTextChanged = false;
  return true;
}

void KDateEdit::updateView()
{
  QString dateString;
  if ( mDate.isValid() ) {
    dateString = sDateFormat->toString( mDate );
  }

  // We do not want to generate a signal here,
  // since we explicitly setting the date
  bool blocked = signalsBlocked();
  blockSignals( true );
  removeItem( 0 );
  insertItem( 0, dateString );
  blockSignals( blocked );
}
