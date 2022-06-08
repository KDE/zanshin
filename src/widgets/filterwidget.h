/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_FILTERWIDGET_H
#define WIDGETS_FILTERWIDGET_H

#include <QWidget>

class QComboBox;
class QLineEdit;

namespace Presentation
{
    class TaskFilterProxyModel;
}

namespace Ui {
    class FilterWidget;
}

namespace Widgets {

class FilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterWidget(QWidget *parent = nullptr);
    ~FilterWidget();

    Presentation::TaskFilterProxyModel *proxyModel() const;

public Q_SLOTS:
    void clear();
    void setShowDoneTasks(bool show);
    void setShowFutureTasks(bool show);

private Q_SLOTS:
    void onTextChanged(const QString &text);
    void onSortTypeChanged(int index);
    void onAscendingClicked();
    void onDescendingClicked();

private:
    Ui::FilterWidget *ui;
    Presentation::TaskFilterProxyModel *m_model;
};

}

#endif // WIDGETS_FILTERWIDGET_H
