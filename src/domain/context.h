/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef DOMAIN_CONTEXT_H
#define DOMAIN_CONTEXT_H

#include <QMetaType>
#include <QSharedPointer>
#include <QString>

namespace Domain {

class Context : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    typedef QSharedPointer<Context> Ptr;
    typedef QList<Context::Ptr> List;

    explicit Context(QObject *parent = nullptr);
    virtual ~Context();

    QString name() const;

public slots:
    void setName(const QString &name);

signals:
    void nameChanged(const QString &name);

private:
    QString m_name;
};

}

Q_DECLARE_METATYPE(Domain::Context::Ptr)

#endif // DOMAIN_CONTEXT_H
