
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_CONFIG_H
#define BOOST_FIBERS_DETAIL_CONFIG_H

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#ifdef BOOST_FIBERS_DECL
# undef BOOST_FIBERS_DECL
#endif

#if defined(BOOST_HAS_DECLSPEC)
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_FIBERS_DYN_LINK)
#  if ! defined(BOOST_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  if defined(BOOST_FIBERS_SOURCE)
#   define BOOST_FIBERS_DECL BOOST_SYMBOL_EXPORT
#  else 
#   define BOOST_FIBERS_DECL BOOST_SYMBOL_IMPORT
#  endif
# endif
#endif

#if ! defined(BOOST_FIBERS_DECL)
# define BOOST_FIBERS_DECL
#endif

#if ! defined(BOOST_FIBERS_SOURCE) && ! defined(BOOST_ALL_NO_LIB) && ! defined(BOOST_FIBERS_NO_LIB)
# define BOOST_LIB_NAME boost_fiber
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_FIBERS_DYN_LINK)
#  define BOOST_DYN_LINK
# endif
# include <boost/config/auto_link.hpp>
#endif

// FUTURE_INVALID_AFTER_GET
#if ! defined BOOST_FIBERS_PROVIDES_FUTURE_INVALID_AFTER_GET \
 && ! defined BOOST_FIBERS_DONT_PROVIDE_FUTURE_INVALID_AFTER_GET
# define BOOST_FIBERS_PROVIDES_FUTURE_INVALID_AFTER_GET
#endif

// INTERRUPTIONS
#if ! defined BOOST_FIBERS_PROVIDES_INTERRUPTIONS \
 && ! defined BOOST_FIBERS_DONT_PROVIDE_INTERRUPTIONS
# define BOOST_FIBERS_PROVIDES_INTERRUPTIONS
#endif

// SIGNATURE_PACKAGED_TASK
#if ! defined BOOST_FIBERS_PROVIDES_SIGNATURE_PACKAGED_TASK \
 && ! defined BOOST_FIBERS_DONT_PROVIDE_SIGNATURE_PACKAGED_TASK
# define BOOST_FIBERS_PROVIDES_SIGNATURE_PACKAGED_TASK
#endif

// FUTURE_CTOR_ALLOCATORS
#if ! defined BOOST_FIBERS_PROVIDES_FUTURE_CTOR_ALLOCATORS \
 && ! defined BOOST_FIBERS_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#define BOOST_FIBERS_PROVIDES_FUTURE_CTOR_ALLOCATORS
#endif

#endif // BOOST_FIBERS_DETAIL_CONFIG_H
