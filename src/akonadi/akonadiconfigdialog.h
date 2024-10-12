/*
 * SPDX-FileCopyrightText: 2011-2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_CONFIGDIALOG_H
#define AKONADI_CONFIGDIALOG_H

#include <QDialog>

#include "akonadistorageinterface.h"

namespace Akonadi
{

class AgentInstanceFilterProxyModel;
class AgentInstanceWidget;
class AgentFilterProxyModel;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget *parent = nullptr);

private slots:
    void onAddTriggered();
    void onRemoveTriggered();
    void onConfigureTriggered();

private:
    void applyContentTypes(Akonadi::AgentInstanceFilterProxyModel *model);
    void applyContentTypes(AgentFilterProxyModel *model);

    Akonadi::AgentInstanceWidget *m_agentInstanceWidget;
};

}

#endif // AKONADI_CONFIGDIALOG_H

