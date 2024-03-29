/*
 * SPDX-FileCopyrightText: 2008 Trevor Pounds
 * SPDX-License-Identifier: MIT
 */

#ifndef __MOCKITOPP_ACTION_HPP__
#define __MOCKITOPP_ACTION_HPP__

namespace mockitopp
{
   namespace detail
   {
      template <typename R>
      struct action
      {
         virtual R invoke() = 0;

         virtual ~action() {}
      };

      template <typename R>
      struct returnable_action : public action<R>
      {
         R _returnable;

         returnable_action(const R& returnable)
            : _returnable(returnable)
            {}

         R invoke() override { return _returnable; }
      };

      template <>
      struct returnable_action<void> : public action<void>
      {
         void invoke() override {}
      };

      template <typename R, typename T>
      struct throwable_action : public action<R>
      {
         T _throwable;

         throwable_action(const T& throwable)
            : _throwable(throwable)
            {}

         R invoke() override { throw _throwable; }
      };
   } // namespace detail
} // namespace mockitopp

#endif //__MOCKITOPP_ACTION_HPP__
