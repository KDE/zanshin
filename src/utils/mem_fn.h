/* This file is part of Zanshin

   Copyright 2015 René J.V. Bertin <rjvbertin@gmail.com>
   with thanks to "sehe" @ http://stackoverflow.com/questions/32255435

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


#ifndef UTILS_MEMFN_H
#define UTILS_MEMFN_H

#ifdef ZANSHIN_USE_BOOST_MEM_FN

namespace boost {
    // this template teaches get_pointer about QSharedPointer
    template<class T, template <class> class Q > T* get_pointer(const Q<T> &pointer)
    {
        return pointer.data();
    }
}

#include <boost/mem_fn.hpp>

#else // ZANSHIN_USE_BOOST_MEM_FN

#include <functional>

#endif // ZANSHIN_USE_BOOST_MEM_FN

namespace Utils {
#ifdef ZANSHIN_USE_BOOST_MEM_FN

    template <typename PTMorPTMF>
    auto mem_fn(PTMorPTMF const& ptm) -> decltype(boost::mem_fn(ptm)) {
        return boost::mem_fn(ptm);
    }

#else

    template <typename PTMorPTMF>
    auto mem_fn(PTMorPTMF const& ptm) -> decltype(std::mem_fn(ptm)) {
        return std::mem_fn(ptm);
    }

#endif
}

#endif // UTILS_MEMFN_H