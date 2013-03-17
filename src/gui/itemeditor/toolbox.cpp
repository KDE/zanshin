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


#include "toolbox.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QAbstractButton>

#include <qapplication.h>
#include <qeventloop.h>
#include <qlist.h>
#include <qpainter.h>
#include <qscrollarea.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <KDebug>

//This code is almost entierly copied from QToolBoxButton (qtoolbox.cpp), but modified a bit
class Header: public QAbstractButton
{
    Q_OBJECT
public:
    Header(const QString &name, QWidget *parent)
    :   QAbstractButton(parent),
        selected(false)
    {
        setText(name);
        setBackgroundRole(QPalette::Window);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        setFocusPolicy(Qt::NoFocus);
        /*
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::red);
        setPalette(p);
        setAutoFillBackground(true);
        */
    }

    inline void setSelected(bool b) { selected = b; update(); }

    QSize sizeHint() const
    {
        QSize iconSize(8, 8);
        if (!icon().isNull()) {
            int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, parentWidget()->parentWidget() /* QToolBox */);
            iconSize += QSize(icone + 2, icone);
        }
        QSize textSize = fontMetrics().size(Qt::TextShowMnemonic, text()) + QSize(0, 8);

        QSize total(iconSize.width() + textSize.width(), qMax(iconSize.height(), textSize.height()));
        return total.expandedTo(QApplication::globalStrut());
    }

    QSize minimumSizeHint() const
    {
        if (icon().isNull())
            return QSize();
        int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, parentWidget()->parentWidget() /* QToolBox */);
        return QSize(icone + 8, icone + 8);
    }
    
protected:
    void initStyleOption(QStyleOptionToolBox *opt) const;
    void paintEvent(QPaintEvent *);
    virtual void enterEvent(QEvent* )
    {
        update(); //This shouldn't be needed (it is not in qtoolbox), but otherwise highlighting doesn't work
    }
    virtual void leaveEvent(QEvent* )
    {
        update(); //This shouldn't be needed (it is not in qtoolbox), but otherwise highlighting doesn't work
    }
private:
    bool selected;
};




void Header::initStyleOption(QStyleOptionToolBox *option) const
{
    if (!option)
        return;
    option->initFrom(this);
    if (selected)
        option->state |= QStyle::State_Selected;
    if (isDown())
        option->state |= QStyle::State_Sunken;
    option->text = text();
    option->icon = icon();

    if (QStyleOptionToolBoxV2 *optionV2 = qstyleoption_cast<QStyleOptionToolBoxV2 *>(option)) {
        Toolbox *toolBox = static_cast<Toolbox *>(parentWidget()->parentWidget()); // I know I'm in a tool box.
        int widgetCount = toolBox->count();
        int currIndex = toolBox->currentIndex();
        int indexInPage = toolBox->layout()->indexOf(parentWidget());

        //kDebug() << widgetCount << currIndex << indexInPage;
        if (widgetCount == 1) {
            optionV2->position = QStyleOptionToolBoxV2::OnlyOneTab;
        } else if (indexInPage == 0) {
            optionV2->position = QStyleOptionToolBoxV2::Beginning;
        } else if (indexInPage == widgetCount - 1) {
            optionV2->position = QStyleOptionToolBoxV2::End;
        } else {
            optionV2->position = QStyleOptionToolBoxV2::Middle;
        }
        if (currIndex == indexInPage - 1) {
            optionV2->selectedPosition = QStyleOptionToolBoxV2::PreviousIsSelected;
        } else if (currIndex == indexInPage + 1) {
            optionV2->selectedPosition = QStyleOptionToolBoxV2::NextIsSelected;
        } else {
            optionV2->selectedPosition = QStyleOptionToolBoxV2::NotAdjacent;
        }
    }
}

void Header::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    QString text = QAbstractButton::text();
    QPainter *p = &paint;
    QStyleOptionToolBoxV2 opt;
    initStyleOption(&opt);
    style()->drawControl(QStyle::CE_ToolBoxTab, &opt, p, parentWidget());
}


class Container: public QWidget
{
     Q_OBJECT
public:
    Container(const QString &title, QWidget *widget, QWidget *parent)
    :   QWidget(parent),
        widget(widget),
        header(new Header(title, this)),
        m_enabled(false)
    {
        QVBoxLayout *l = new QVBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        setLayout(l);
        connect(header, SIGNAL(clicked()), this, SLOT(toggleVisibility()));
        layout()->addWidget(header);
        layout()->addWidget(widget);
        widget->hide();
        /*
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::green);
        setPalette(p);
        setAutoFillBackground(true);
       */
    }
    
    bool enabled()
    {
        return m_enabled;
    }
signals:
    void activated();
public slots:
    void toggleVisibility()
    {
        if (widget->isVisible()) {
            widget->hide();
            m_enabled = false;
            header->setSelected(false);
        } else {
            static_cast<Toolbox*>(parentWidget())->collapseAll();
            widget->show();
            m_enabled = true;
            header->setSelected(true);
        }
    }
    
private:
    QWidget *widget;
    Header *header;
    bool m_enabled;
};


Toolbox::Toolbox(QWidget* parent, Qt::WindowFlags f)
:   QWidget(parent, f)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    /*
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::yellow);
    setPalette(p);
    setAutoFillBackground(true);
    */
}

Toolbox::~Toolbox()
{

}


int Toolbox::count()
{
    return layout()->count();
}

int Toolbox::currentIndex()
{
    for (int i = 0; i < layout()->count(); i++) {
        Container *widget = static_cast<Container*>(layout()->itemAt(i)->widget());
        Q_ASSERT(widget);
        if (widget->enabled()) {
            return i;
        }
    }
    return -1;
}

void Toolbox::activateWidget(int index)
{
    if (index < currentIndex() || index >= layout()->count())
        return;
    if (index <  0) {
        collapseAll();
    } else {
        static_cast<Container*>(layout()->itemAt(index)->widget())->toggleVisibility();
    }
}


void Toolbox::addWidget(QWidget* widget, const QString& title)
{
    Container *container = new Container(title, widget, this);
    widget->setContentsMargins(8, 8, 4, 4);
    static_cast<QVBoxLayout*>(layout())->addWidget(container);
}

void Toolbox::collapseAll()
{
    for (int i = 0; i < layout()->count(); i++) {
        Container *widget = static_cast<Container*>(layout()->itemAt(i)->widget());
        Q_ASSERT(widget);
        if (widget->enabled()) {
            widget->toggleVisibility();
        }
    }
}


#include "toolbox.moc"
#include "moc_toolbox.cpp"
