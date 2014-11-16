/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef UTILS_MOCKOBJECT_H
#define UTILS_MOCKOBJECT_H

#include <mockitopp/mockitopp.hpp>

#include <QSharedPointer>

namespace Utils {
    template<typename T>
    class MockObject
    {
        typedef mockitopp::mock_object<T> InternalMockType;

        class InternalDeleter
        {
        public:
            explicit InternalDeleter(InternalMockType *mockObject)
                : m_mockObject(mockObject)
            {
            }

            void operator ()(T *)
            {
                delete m_mockObject;
            }

        private:
            InternalMockType *m_mockObject;
        };

    public:
        MockObject()
            : m_mockObject(new InternalMockType),
              m_mockPtr(&m_mockObject->getInstance(), InternalDeleter(m_mockObject))
        {
        }

        QSharedPointer<T> getInstance()
        {
            return m_mockPtr;
        }

        template <typename M>
        mockitopp::detail::dynamic_vfunction<typename mockitopp::detail::remove_member_function_pointer_cv<M>::type>& operator() (M ptr2member)
        {
            return (*m_mockObject)(ptr2member);
        }

    private:
        InternalMockType *m_mockObject;
        QSharedPointer<T> m_mockPtr;
    };
}

#endif // UTILS_MOCKOBJECT_H
