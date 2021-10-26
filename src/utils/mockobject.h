/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
