/*
   SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
   SPDX-FileCopyrightText: 2002 David Jarvie <software@astrojar.org.uk>
   SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEPIM_KDATEEDIT_H
#define KDEPIM_KDATEEDIT_H

#include "kdatepickerpopup.h"
#include "kdepim_export.h"

#include <QComboBox>
#include <QDateTime>
#include <QEvent>
#include <QMap>
#include <QMouseEvent>

class QEvent;

namespace KPIM {

/**
  A date editing widget that consists of an editable combo box.
  The combo box contains the date in text form, and clicking the combo
  box arrow will display a 'popup' style date picker.

  This widget also supports advanced features like allowing the user
  to type in the day name to get the date. The following keywords
  are supported (in the native language): tomorrow, yesterday, today,
  monday, tuesday, wednesday, thursday, friday, saturday, sunday.

  @image html kdateedit.png "This is how it looks"

  @author Cornelius Schumacher <schumacher@kde.org>
  @author Mike Pilone <mpilone@slac.com>
  @author David Jarvie <software@astrojar.org.uk>
  @author Tobias Koenig <tokoe@kde.org>
*/
class KDEPIM_EXPORT KDateEdit : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY( QDate date READ date WRITE setDate )

  public:
    explicit KDateEdit( QWidget *parent = 0 );
    virtual ~KDateEdit();

    /**
      @return The date entered. This date could be invalid,
              you have to check validity yourself.
     */
    QDate date() const;

    /**
      Sets whether the widget is read-only for the user. If read-only, the
      date pop-up is inactive, and the displayed date cannot be edited.

      @param readOnly True to set the widget read-only, false to set it read-write.
     */
    void setReadOnly( bool readOnly );

    /**
      @return True if the widget is read-only, false if read-write.
     */
    bool isReadOnly() const;

    void showPopup() override;

  Q_SIGNALS:
    /**
      This signal is emitted whenever the user has entered a new date.
      When the user changes the date by editing the line edit field,
      the signal is not emitted until focus leaves the line edit field.
      The passed date can be invalid.
     */
    void dateEntered( const QDate &date );

    /**
      This signal is emitted whenever the user modifies the date.
      The passed date can be invalid.
     */
    void dateChanged( const QDate &date );

  public Q_SLOTS:
    /**
      Sets the date.

      @param date The new date to display. This date must be valid or
                  it will not be set
     */
    void setDate( const QDate &date );

  protected Q_SLOTS:
    void lineEnterPressed();
    void slotTextChanged( const QString & );
    void dateSelected( const QDate & );

  protected:
    bool eventFilter( QObject *, QEvent * ) override;
    void mousePressEvent( QMouseEvent * ) override;
    void focusOutEvent( QFocusEvent * ) override;
    void wheelEvent( QWheelEvent * ) override;
    void keyPressEvent( QKeyEvent * ) override;

    /**
      Sets the date, without altering the display.
      This method is used internally to set the widget's date value.
      As a virtual method, it allows derived classes to perform additional
      validation on the date value before it is set. Derived classes should
      return true if QDate::isValid(@p date) returns false.

      @param date The new date to set.
      @return True if the date was set, false if it was considered invalid and
              remains unchanged.
     */
    virtual bool assignDate( const QDate &date );

    /**
      Fills the keyword map. Reimplement it if you want additional keywords.
     */
    void setupKeywords();

  private:
    QDate parseDate( bool *replaced = 0 ) const;
    void updateView();

    KDatePickerPopup *mPopup;

    QDate mDate;
    bool mReadOnly;
    bool mTextChanged;
    bool mDiscardNextMousePress;

    QMap<QString, int> mKeywordMap;
};

}

#endif
