/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef ITEMCONTEXT_H
#define ITEMCONTEXT_H

#include <QWidget>

#include <Nepomuk/Resource>

namespace Nepomuk {
    namespace Utils {
        class SimpleResourceModel;
    }
}
class NepomukContextModel;
class NepomukContextView;
class QLabel;
/**
 * A Widget for showing/adding related items (documents, etc)
 */
class ItemContext : public QWidget
{
    Q_OBJECT
public:
    explicit ItemContext(QWidget* parent = 0);

    /**
     * Sets the resource for which we want the context
     * Actually we set the thing here normally
     */
    void setResource(const Nepomuk::Resource &resource);
    /**
     * Set the uri related to the current item
     */
    void addContext(const KUrl &uri);
protected:
    virtual void dragEnterEvent(QDragEnterEvent* );
    virtual void dropEvent(QDropEvent* );
signals:
    void itemActivated(const Nepomuk::Resource &);
private slots:
    void removeSelection();
    void openSelection();

private:
    Nepomuk::Resource m_resource;
    void updateContext();
    NepomukContextModel *m_model;
    NepomukContextView *m_contextView;
};

#endif // ITEMCONTEXT_H
