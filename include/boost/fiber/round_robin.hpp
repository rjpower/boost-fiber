//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DEFAULT_SCHEDULER_H
#define BOOST_FIBERS_DEFAULT_SCHEDULER_H

#include <cstddef>
#include <deque>
#include <set>
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/locks.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/notify.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/algorithm.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

namespace boost {
namespace fibers {

class BOOST_FIBERS_DECL round_robin : public algorithm
{
private:
    typedef std::deque< detail::fiber_base::ptr_t >     wqueue_t;
    typedef std::deque< detail::fiber_base::ptr_t >     rqueue_t;

    detail::fiber_base::ptr_t   active_fiber_;
    detail::notify::ptr_t       notifier_;
    wqueue_t                    wqueue_;
    detail::spinlock            rqueue_mtx_;
    rqueue_t                    rqueue_;

public:
    round_robin() BOOST_NOEXCEPT;

    ~round_robin() BOOST_NOEXCEPT;

    void spawn( detail::fiber_base::ptr_t const&);

    void priority( detail::fiber_base::ptr_t const&, int);

    void join( detail::fiber_base::ptr_t const&);

    detail::fiber_base::ptr_t active() BOOST_NOEXCEPT
    { return active_fiber_; }

    bool run();

    void wait( unique_lock< detail::spinlock > &);

    void yield();

    detail::notify::ptr_t notifier();

    void migrate_to( fiber const&);

    fiber steel_from();
};

}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DEFAULT_SCHEDULER_H
