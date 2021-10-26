/*
 * SPDX-FileCopyrightText: 2015 René J.V. Bertin <rjvbertin@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
