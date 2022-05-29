/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_APPLICATIONSELECTEDATTRIBUTE_H
#define AKONADI_APPLICATIONSELECTEDATTRIBUTE_H

#include <QByteArray>

#include <Akonadi/Attribute>

namespace Akonadi {

class ApplicationSelectedAttribute : public Attribute
{
public:
    ApplicationSelectedAttribute();
    ~ApplicationSelectedAttribute();

    void setSelected(bool selected);
    bool isSelected() const;

    QByteArray type() const override;
    ApplicationSelectedAttribute *clone() const override;

    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    bool m_selected;
};

}

#endif
