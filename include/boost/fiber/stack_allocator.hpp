
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_STACK_ALLOCATOR_H
#define BOOST_FIBERS_STACK_ALLOCATOR_H

#include <boost/config.hpp>

#if defined (BOOST_WINDOWS)
#include <boost/fiber/detail/stack_allocator_windows.hpp>
#else
#include <boost/fiber/detail/stack_allocator_posix.hpp>
#endif

namespace boost {
namespace fibers {
using detail::stack_allocator;
}}

#endif // BOOST_FIBERS_STACK_ALLOCATOR_H
