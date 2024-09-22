/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef DOMAIN_DATASOURCE_H
#define DOMAIN_DATASOURCE_H

#include <QDebug>
#include <QMetaType>
#include <QObject>
#include <QSharedPointer>
#include <QString>

namespace Domain {

// cppcheck somehow doesn't see the ctor in here
// cppcheck-suppress noConstructor
class DataSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(Domain::DataSource::ContentTypes contentTypes READ contentTypes WRITE setContentTypes NOTIFY contentTypesChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
public:
    typedef QSharedPointer<DataSource> Ptr;
    typedef QList<DataSource::Ptr> List;

    enum ContentType {
        NoContent = 0,
        Tasks,
    };
    Q_ENUM(ContentType)
    Q_DECLARE_FLAGS(ContentTypes, ContentType)

    explicit DataSource(QObject *parent = nullptr);
    virtual ~DataSource();

    QString name() const;
    QString iconName() const;
    ContentTypes contentTypes() const;
    bool isSelected() const;

public slots:
    void setName(const QString &name);
    void setIconName(const QString &iconName);
    void setContentTypes(Domain::DataSource::ContentTypes types);
    void setSelected(bool selected);

signals:
    void nameChanged(const QString &name);
    void iconNameChanged(const QString &iconName);
    void contentTypesChanged(Domain::DataSource::ContentTypes types);
    void selectedChanged(bool selected);

private:
    QString m_name;
    QString m_iconName;
    ContentTypes m_contentTypes;
    bool m_selected;
};

}

Q_DECLARE_METATYPE(Domain::DataSource::Ptr)
Q_DECLARE_METATYPE(Domain::DataSource::ContentTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Domain::DataSource::ContentTypes)
QDebug operator<<(QDebug dbg, const Domain::DataSource &dataSource);
QDebug operator<<(QDebug dbg, const Domain::DataSource::Ptr &dataSource);

#endif // DOMAIN_DATASOURCE_H
