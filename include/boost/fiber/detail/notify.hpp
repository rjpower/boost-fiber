
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_NOTIFY_H
#define BOOST_FIBERS_DETAIL_NOTIFY_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

class notify : private noncopyable
{
protected:
    virtual void add_ref() BOOST_NOEXCEPT = 0;
    virtual void release_ref() = 0;

public:
    typedef intrusive_ptr< notify >     ptr_t;

    virtual ~notify() {};

    virtual bool is_ready() const BOOST_NOEXCEPT = 0;

    virtual void set_ready() BOOST_NOEXCEPT = 0;

    friend inline void intrusive_ptr_add_ref( notify * p) BOOST_NOEXCEPT
    { p->add_ref(); }

    friend inline void intrusive_ptr_release( notify * p)
    { p->release_ref(); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_NOTIFY_H
