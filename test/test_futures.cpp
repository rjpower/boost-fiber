//  (C) Copyright 2008-10 Anthony Williams 
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <utility>
#include <memory>
#include <string>

#include <boost/chrono/system_clocks.hpp>
#include <boost/move/move.hpp>
#include <boost/fiber/all.hpp>
#include <boost/test/unit_test.hpp>

boost::fibers::mutex callback_mutex;
unsigned callback_called=0;
int global_ref_target=0;

int& return_ref()
{
    return global_ref_target;
}

void do_nothing_callback(boost::fibers::promise<int>& /*pi*/)
{
    boost::lock_guard<boost::fibers::mutex> lk(callback_mutex);
    ++callback_called;
}

void f_wait_callback(boost::fibers::promise<int>& pi)
{
    boost::lock_guard<boost::fibers::mutex> lk(callback_mutex);
    ++callback_called;
    try
    {
        pi.set_value(42);
    }
    catch(...)
    {
    }
}

void wait_callback_for_task(boost::fibers::packaged_task<int>& pt)
{
    boost::lock_guard<boost::fibers::mutex> lk(callback_mutex);
    ++callback_called;
    try
    {
        pt();
    }
    catch(...)
    {
    }
}

int make_int_slowly()
{
    boost::this_fiber::yield();
    boost::this_fiber::yield();
    boost::this_fiber::yield();
    return 42;
}

int echo( int i)
{
    boost::this_fiber::yield();
    return i;
}


void future_wait()
{
    boost::fibers::packaged_task<int> pt(boost::bind( echo, 42));
    boost::fibers::unique_future<int> f(pt.get_future());
    
    boost::fibers::fiber( boost::move(pt) ).detach();
    
    f.wait();
    int i = f.get();
    
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK_EQUAL( 42, i);
}


void do_nothing()
{}

struct X
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE( X)
    
public:
    int i;
    
    X():
        i(42)
    {}
#ifndef BOOST_NO_RVALUE_REFERENCES
    X(X&& other):
        i(other.i)
    {
        other.i=0;
    }
#else
    X( BOOST_RV_REF( X) other):
        i(other.i)
    {
        other.i=0;
    }
#endif
    ~X()
    {}
};

int make_int()
{ return 42; }

int throw_runtime_error()
{ throw std::runtime_error("42"); }

void set_promise_thread(boost::fibers::promise<int>* p)
{ p->set_value(42); }

struct my_exception
{};

void set_promise_exception_thread(boost::fibers::promise<int>* p)
{ p->set_exception(boost::copy_exception(my_exception())); }


void store_value_from_thread()
{
    boost::fibers::promise<int> pi2;
    boost::fibers::unique_future<int> fi2(pi2.get_future());
    boost::fibers::fiber(
        boost::bind( set_promise_thread,&pi2) ).detach();
    int j=fi2.get();
    BOOST_CHECK(j==42);
    BOOST_CHECK(fi2.is_ready());
    BOOST_CHECK(fi2.has_value());
    BOOST_CHECK(!fi2.has_exception());
    BOOST_CHECK(fi2.get_state()==boost::fibers::future_state::ready);
}


void store_exception()
{
    boost::fibers::promise<int> pi3;
    boost::fibers::unique_future<int> fi3=pi3.get_future();
    boost::fibers::fiber(
        boost::bind( set_promise_exception_thread,&pi3) ).detach();
    try
    {
        fi3.get();
        BOOST_CHECK(false);
    }
    catch(my_exception)
    {
        BOOST_CHECK(true);
    }
    
    BOOST_CHECK(fi3.is_ready());
    BOOST_CHECK(!fi3.has_value());
    BOOST_CHECK(fi3.has_exception());
    BOOST_CHECK(fi3.get_state()==boost::fibers::future_state::ready);
}

void initial_state()
{
    boost::fibers::unique_future<int> fi;
    BOOST_CHECK(!fi.is_ready());
    BOOST_CHECK(!fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::uninitialized);
    int i = 0;
    try
    {
        i=fi.get();
        BOOST_CHECK(0!=i);
        BOOST_CHECK(false);
    }
    catch(boost::fibers::future_uninitialized)
    {
        BOOST_CHECK(true);
    }
}

void waiting_future()
{
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi;
    fi=pi.get_future();

    int i=0;
    BOOST_CHECK(!fi.is_ready());
    BOOST_CHECK(!fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::waiting);
    BOOST_CHECK(i==0);
}

void cannot_get_future_twice()
{
    boost::fibers::promise<int> pi;
    pi.get_future();

    try
    {
        pi.get_future();
        BOOST_CHECK(false);
    }
    catch(boost::fibers::future_already_retrieved&)
    {
        BOOST_CHECK(true);
    }
}

void set_value_updates_future_state()
{
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi;
    fi=pi.get_future();

    pi.set_value(42);
    
    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
}

void set_value_can_be_retrieved()
{
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi;
    fi=pi.get_future();

    pi.set_value(42);
    
    int i=0;
    BOOST_CHECK(i=fi.get());
    BOOST_CHECK(i==42);
    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
}

void set_value_can_be_moved()
{
//     boost::fibers::promise<int> pi;
//     boost::fibers::unique_future<int> fi;
//     fi=pi.get_future();

//     pi.set_value(42);
    
//     int i=0;
//     BOOST_CHECK(i=fi.get());
//     BOOST_CHECK(i==42);
//     BOOST_CHECK(fi.is_ready());
//     BOOST_CHECK(fi.has_value());
//     BOOST_CHECK(!fi.has_exception());
//     BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
}

void future_from_packaged_task_is_waiting()
{
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();
    int i=0;
    BOOST_CHECK(!fi.is_ready());
    BOOST_CHECK(!fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::waiting);
    BOOST_CHECK(i==0);
}

void invoking_a_packaged_task_populates_future()
{
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();

    pt();

    int i=0;
    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(i=fi.get());
    BOOST_CHECK(i==42);
}

void invoking_a_packaged_task_twice_throws()
{
    boost::fibers::packaged_task<int> pt(make_int);

    pt();
    try
    {
        pt();
        BOOST_CHECK(false);
    }
    catch(boost::fibers::task_already_started)
    {
        BOOST_CHECK(true);
    }
}


void cannot_get_future_twice_from_task()
{
    boost::fibers::packaged_task<int> pt(make_int);
    pt.get_future();
    try
    {
        pt.get_future();
        BOOST_CHECK(false);
    }
    catch(boost::fibers::future_already_retrieved)
    {
        BOOST_CHECK(true);
    }
}

void task_stores_exception_if_function_throws()
{
    boost::fibers::packaged_task<int> pt(throw_runtime_error);
    boost::fibers::unique_future<int> fi=pt.get_future();

    pt();

    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(!fi.has_value());
    BOOST_CHECK(fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
    try
    {
        fi.get();
        BOOST_CHECK(false);
    }
    catch(std::exception&)
    {
        BOOST_CHECK(true);
    }
    catch(...)
    {
        BOOST_CHECK(!"Unknown exception thrown");
    }
    
}

void void_promise()
{
    boost::fibers::promise<void> p;
    boost::fibers::unique_future<void> f=p.get_future();
    p.set_value();
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK(f.has_value());
    BOOST_CHECK(!f.has_exception());
    BOOST_CHECK(f.get_state()==boost::fibers::future_state::ready);
    f.get();
}

void reference_promise()
{
    boost::fibers::promise<int&> p;
    boost::fibers::unique_future<int&> f=p.get_future();
    int i=42;
    p.set_value(i);
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK(f.has_value());
    BOOST_CHECK(!f.has_exception());
    BOOST_CHECK(f.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(&f.get()==&i);
}

void task_returning_void()
{
    boost::fibers::packaged_task<void> pt(do_nothing);
    boost::fibers::unique_future<void> fi=pt.get_future();

    pt();

    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
}

void task_returning_reference()
{
    boost::fibers::packaged_task<int&> pt(return_ref);
    boost::fibers::unique_future<int&> fi=pt.get_future();

    pt();

    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(fi.has_value());
    BOOST_CHECK(!fi.has_exception());
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::ready);
    int& i=fi.get();
    BOOST_CHECK(&i==&global_ref_target);
}

void shared_future()
{
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();

    boost::fibers::shared_future<int> sf(boost::move(fi));
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::uninitialized);

    pt();

    int i=0;
    BOOST_CHECK(sf.is_ready());
    BOOST_CHECK(sf.has_value());
    BOOST_CHECK(!sf.has_exception());
    BOOST_CHECK(sf.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(i=sf.get());
    BOOST_CHECK(i==42);
}

void copies_of_shared_future_become_ready_together()
{
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();

    boost::fibers::shared_future<int> sf(boost::move(fi));
    boost::fibers::shared_future<int> sf2(sf);
    boost::fibers::shared_future<int> sf3;
    sf3=sf;
    BOOST_CHECK(sf.get_state()==boost::fibers::future_state::waiting);
    BOOST_CHECK(sf2.get_state()==boost::fibers::future_state::waiting);
    BOOST_CHECK(sf3.get_state()==boost::fibers::future_state::waiting);

    pt();

    int i=0;
    BOOST_CHECK(sf.is_ready());
    BOOST_CHECK(sf.has_value());
    BOOST_CHECK(!sf.has_exception());
    BOOST_CHECK(sf.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(i=sf.get());
    BOOST_CHECK(i==42);
    i=0;
    BOOST_CHECK(sf2.is_ready());
    BOOST_CHECK(sf2.has_value());
    BOOST_CHECK(!sf2.has_exception());
    BOOST_CHECK(sf2.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(i=sf2.get());
    BOOST_CHECK(i==42);
    i=0;
    BOOST_CHECK(sf3.is_ready());
    BOOST_CHECK(sf3.has_value());
    BOOST_CHECK(!sf3.has_exception());
    BOOST_CHECK(sf3.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(i=sf3.get());
    BOOST_CHECK(i==42);
}

void shared_future_can_be_move_assigned_from_unique_future()
{
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();

    boost::fibers::shared_future<int> sf;
    sf=boost::move(fi);
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::uninitialized);

    BOOST_CHECK(!sf.is_ready());
    BOOST_CHECK(!sf.has_value());
    BOOST_CHECK(!sf.has_exception());
    BOOST_CHECK(sf.get_state()==boost::fibers::future_state::waiting);
}

void shared_future_void()
{
    boost::fibers::packaged_task<void> pt(do_nothing);
    boost::fibers::unique_future<void> fi=pt.get_future();

    boost::fibers::shared_future<void> sf(boost::move(fi));
    BOOST_CHECK(fi.get_state()==boost::fibers::future_state::uninitialized);

    pt();

    BOOST_CHECK(sf.is_ready());
    BOOST_CHECK(sf.has_value());
    BOOST_CHECK(!sf.has_exception());
    BOOST_CHECK(sf.get_state()==boost::fibers::future_state::ready);
    sf.get();
}

void shared_future_ref()
{
    boost::fibers::promise<int&> p;
    boost::fibers::shared_future<int&> f(p.get_future());
    int i=42;
    p.set_value(i);
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK(f.has_value());
    BOOST_CHECK(!f.has_exception());
    BOOST_CHECK(f.get_state()==boost::fibers::future_state::ready);
    BOOST_CHECK(&f.get()==&i);
}

void can_get_a_second_future_from_a_moved_promise()
{
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi=pi.get_future();
    
    boost::fibers::promise<int> pi2(boost::move(pi));
    boost::fibers::unique_future<int> fi2=pi.get_future();

    pi2.set_value(3);
    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(!fi2.is_ready());
    BOOST_CHECK(fi.get()==3);
    pi.set_value(42);
    BOOST_CHECK(fi2.is_ready());
    BOOST_CHECK(fi2.get()==42);
}

void can_get_a_second_future_from_a_moved_void_promise()
{
    boost::fibers::promise<void> pi;
    boost::fibers::unique_future<void> fi=pi.get_future();
    
    boost::fibers::promise<void> pi2(boost::move(pi));
    boost::fibers::unique_future<void> fi2=pi.get_future();

    pi2.set_value();
    BOOST_CHECK(fi.is_ready());
    BOOST_CHECK(!fi2.is_ready());
    pi.set_value();
    BOOST_CHECK(fi2.is_ready());
}
#if 0
void unique_future_for_move_only_udt()
{
    boost::fibers::promise<X> pt;
    boost::fibers::unique_future<X> fi=pt.get_future();

    X tmp;
    pt.set_value(boost::move(tmp));
    X res(boost::move(fi.get()));
    BOOST_CHECK(res.i==42);
}
#endif
void unique_future_for_string()
{
    boost::fibers::promise<std::string> pt;
    boost::fibers::unique_future<std::string> fi=pt.get_future();

    pt.set_value(std::string("hello"));
    std::string res(fi.get());
    BOOST_CHECK(res=="hello");

    boost::fibers::promise<std::string> pt2;
    fi=pt2.get_future();

    std::string const s="goodbye";
    
    pt2.set_value(s);
    res=fi.get();
    BOOST_CHECK(res=="goodbye");

    boost::fibers::promise<std::string> pt3;
    fi=pt3.get_future();

    std::string s2="foo";
    
    pt3.set_value(s2);
    res=fi.get();
    BOOST_CHECK(res=="foo");
}

void wait_callback()
{
    callback_called=0;
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi=pi.get_future();
    pi.set_wait_callback(f_wait_callback);
    fi.wait();
    BOOST_CHECK(callback_called);
    BOOST_CHECK(fi.get()==42);
    fi.wait();
    fi.wait();
    BOOST_CHECK(callback_called==1);
}
#if 0
void wait_callback_with_timed_wait()
{
    callback_called=0;
    boost::fibers::promise<int> pi;
    boost::fibers::unique_future<int> fi=pi.get_future();
    pi.set_wait_callback(do_nothing_callback);
    bool success=fi.timed_wait(boost::chrono::milliseconds(10));
    BOOST_CHECK(callback_called);
    BOOST_CHECK(!success);
    success=fi.timed_wait(boost::chrono::milliseconds(10));
    BOOST_CHECK(!success);
    success=fi.timed_wait(boost::chrono::milliseconds(10));
    BOOST_CHECK(!success);
    BOOST_CHECK(callback_called==3);
    pi.set_value(42);
    success=fi.timed_wait(boost::chrono::milliseconds(10));
    BOOST_CHECK(success);
    BOOST_CHECK(callback_called==3);
    BOOST_CHECK(fi.get()==42);
    BOOST_CHECK(callback_called==3);
}
#endif
void wait_callback_for_packaged_task()
{
    callback_called=0;
    boost::fibers::packaged_task<int> pt(make_int);
    boost::fibers::unique_future<int> fi=pt.get_future();
    pt.set_wait_callback(wait_callback_for_task);
    fi.wait();
    BOOST_CHECK(callback_called);
    BOOST_CHECK(fi.get()==42);
    fi.wait();
    fi.wait();
    BOOST_CHECK(callback_called==1);
}

void packaged_task_can_be_moved()
{
    boost::fibers::packaged_task<int> pt(make_int);

    boost::fibers::unique_future<int> fi=pt.get_future();

    BOOST_CHECK(!fi.is_ready());
    
    boost::fibers::packaged_task<int> pt2(boost::move(pt));

    BOOST_CHECK(!fi.is_ready());
    try
    {
        pt();
        BOOST_CHECK(!"Can invoke moved task!");
    }
    catch(boost::fibers::task_moved&)
    {
    }

    BOOST_CHECK(!fi.is_ready());

    pt2();
    
    BOOST_CHECK(fi.is_ready());
}

void destroying_a_promise_stores_broken_promise()
{
    boost::fibers::unique_future<int> f;
    
    {
        boost::fibers::promise<int> p;
        f=p.get_future();
    }
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK(f.has_exception());
    try
    {
        f.get();
    }
    catch(boost::fibers::broken_promise&)
    {
    }
}

void destroying_a_packaged_task_stores_broken_promise()
{
    boost::fibers::unique_future<int> f;
    
    {
        boost::fibers::packaged_task<int> p(make_int);
        f=p.get_future();
    }
    BOOST_CHECK(f.is_ready());
    BOOST_CHECK(f.has_exception());
    try
    {
        f.get();
    }
    catch(boost::fibers::broken_promise&)
    {
    }
}

void wait_for_either_of_two_futures_1()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    
    boost::fibers::fiber( boost::move(pt) ).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2);
    
    BOOST_CHECK(future==0);
    BOOST_CHECK(f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(f1.get()==42);
}

void wait_for_either_of_two_futures_2()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    
    boost::fibers::fiber( boost::move(pt2) ).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2);
    
    BOOST_CHECK(future==1);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(f2.is_ready());
    BOOST_CHECK(f2.get()==42);
}

void wait_for_either_of_three_futures_1()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    
    boost::fibers::fiber( boost::move(pt) ).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3);
    
    BOOST_CHECK(future==0);
    BOOST_CHECK(f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(f1.get()==42);
}

void wait_for_either_of_three_futures_2()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    
    boost::fibers::fiber( boost::move(pt2) ).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3);
    
    BOOST_CHECK(future==1);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(f2.get()==42);
}

void wait_for_either_of_three_futures_3()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    
    boost::fibers::fiber( boost::move(pt3) ).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3);
    
    BOOST_CHECK(future==2);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(f3.is_ready());
    BOOST_CHECK(f3.get()==42);
}

void wait_for_either_of_four_futures_1()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    
    boost::fibers::fiber(boost::move(pt)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4);
    
    BOOST_CHECK(future==0);
    BOOST_CHECK(f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(f1.get()==42);
}

void wait_for_either_of_four_futures_2()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    
    boost::fibers::fiber(boost::move(pt2)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4);
    
    BOOST_CHECK(future==1);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(f2.get()==42);
}

void wait_for_either_of_four_futures_3()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    
    boost::fibers::fiber(boost::move(pt3)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4);
    
    BOOST_CHECK(future==2);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(f3.get()==42);
}

void wait_for_either_of_four_futures_4()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    
    boost::fibers::fiber(boost::move(pt4)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4);
    
    BOOST_CHECK(future==3);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(f4.is_ready());
    BOOST_CHECK(f4.get()==42);
}

void wait_for_either_of_five_futures_1()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    boost::fibers::packaged_task<int> pt5(make_int_slowly);
    boost::fibers::unique_future<int> f5(pt5.get_future());
    
    boost::fibers::fiber(boost::move(pt)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4,f5);
    
    BOOST_CHECK(future==0);
    BOOST_CHECK(f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(!f5.is_ready());
    BOOST_CHECK(f1.get()==42);
}

void wait_for_either_of_five_futures_2()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    boost::fibers::packaged_task<int> pt5(make_int_slowly);
    boost::fibers::unique_future<int> f5(pt5.get_future());
    
    boost::fibers::fiber(boost::move(pt2)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4,f5);
    
    BOOST_CHECK(future==1);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(!f5.is_ready());
    BOOST_CHECK(f2.get()==42);
}
void wait_for_either_of_five_futures_3()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    boost::fibers::packaged_task<int> pt5(make_int_slowly);
    boost::fibers::unique_future<int> f5(pt5.get_future());
    
    boost::fibers::fiber(boost::move(pt3)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4,f5);
    
    BOOST_CHECK(future==2);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(!f5.is_ready());
    BOOST_CHECK(f3.get()==42);
}
void wait_for_either_of_five_futures_4()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    boost::fibers::packaged_task<int> pt5(make_int_slowly);
    boost::fibers::unique_future<int> f5(pt5.get_future());
    
    boost::fibers::fiber(boost::move(pt4)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4,f5);
    
    BOOST_CHECK(future==3);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(f4.is_ready());
    BOOST_CHECK(!f5.is_ready());
    BOOST_CHECK(f4.get()==42);
}
void wait_for_either_of_five_futures_5()
{
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> f1(pt.get_future());
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> f2(pt2.get_future());
    boost::fibers::packaged_task<int> pt3(make_int_slowly);
    boost::fibers::unique_future<int> f3(pt3.get_future());
    boost::fibers::packaged_task<int> pt4(make_int_slowly);
    boost::fibers::unique_future<int> f4(pt4.get_future());
    boost::fibers::packaged_task<int> pt5(make_int_slowly);
    boost::fibers::unique_future<int> f5(pt5.get_future());
    
    boost::fibers::fiber(boost::move(pt5)).detach();
    
    unsigned const future=boost::fibers::waitfor_any(f1,f2,f3,f4,f5);
    
    BOOST_CHECK(future==4);
    BOOST_CHECK(!f1.is_ready());
    BOOST_CHECK(!f2.is_ready());
    BOOST_CHECK(!f3.is_ready());
    BOOST_CHECK(!f4.is_ready());
    BOOST_CHECK(f5.is_ready());
    BOOST_CHECK(f5.get()==42);
}

void wait_for_either_invokes_callbacks()
{
    callback_called=0;
    boost::fibers::packaged_task<int> pt(make_int_slowly);
    boost::fibers::unique_future<int> fi=pt.get_future();
    boost::fibers::packaged_task<int> pt2(make_int_slowly);
    boost::fibers::unique_future<int> fi2=pt2.get_future();
    pt.set_wait_callback(wait_callback_for_task);

    boost::fibers::fiber(boost::move(pt)).detach();
    boost::fibers::waitfor_any(fi,fi2);
    
    BOOST_CHECK(fi.get()==42);
    BOOST_CHECK(callback_called==1);
}

void wait_for_any_from_range()
{
    unsigned const count=10;
    for(unsigned i=0;i<count;++i)
    {
        boost::fibers::packaged_task<int> tasks[count];
        boost::fibers::unique_future<int> futures[count];
        for(unsigned j=0;j<count;++j)
        {
            tasks[j]=boost::fibers::packaged_task<int>(make_int_slowly);
            futures[j]=tasks[j].get_future();
        }
        boost::fibers::fiber(boost::move(tasks[i])).detach();
    
        BOOST_CHECK(boost::fibers::waitfor_any(futures,futures)==futures);
        
        boost::fibers::unique_future<int>* const future=boost::fibers::waitfor_any(futures,futures+count);
    
        BOOST_CHECK(future==(futures+i));
        for(unsigned j=0;j<count;++j)
        {
            if(j!=i)
            {
                BOOST_CHECK(!futures[j].is_ready());
            }
            else
            {
                BOOST_CHECK(futures[j].is_ready());
            }
        }
        BOOST_CHECK(futures[i].get()==42);
    }
}

void wait_for_all_from_range()
{
    unsigned const count=10;
    boost::fibers::unique_future<int> futures[count];
    for(unsigned j=0;j<count;++j)
    {
        boost::fibers::packaged_task<int> task(make_int_slowly);
        futures[j]=task.get_future();
        boost::fibers::fiber(boost::move(task)).detach();
    }
    
    boost::fibers::waitfor_all(futures,futures+count);
    
    for(unsigned j=0;j<count;++j)
    {
        BOOST_CHECK(futures[j].is_ready());
    }
}

void wait_for_all_two_futures()
{
    unsigned const count=2;
    boost::fibers::unique_future<int> futures[count];
    for(unsigned j=0;j<count;++j)
    {
        boost::fibers::packaged_task<int> task(make_int_slowly);
        futures[j]=task.get_future();
        boost::fibers::fiber(boost::move(task)).detach();
    }
    
    boost::fibers::waitfor_all(futures[0],futures[1]);
    
    for(unsigned j=0;j<count;++j)
    {
        BOOST_CHECK(futures[j].is_ready());
    }
}

void wait_for_all_three_futures()
{
    unsigned const count=3;
    boost::fibers::unique_future<int> futures[count];
    for(unsigned j=0;j<count;++j)
    {
        boost::fibers::packaged_task<int> task(make_int_slowly);
        futures[j]=task.get_future();
        boost::fibers::fiber(boost::move(task)).detach();
    }
    
    boost::fibers::waitfor_all(futures[0],futures[1],futures[2]);
    
    for(unsigned j=0;j<count;++j)
    {
        BOOST_CHECK(futures[j].is_ready());
    }
}

void wait_for_all_four_futures()
{
    unsigned const count=4;
    boost::fibers::unique_future<int> futures[count];
    for(unsigned j=0;j<count;++j)
    {
        boost::fibers::packaged_task<int> task(make_int_slowly);
        futures[j]=task.get_future();
        boost::fibers::fiber(boost::move(task)).detach();
    }
    
    boost::fibers::waitfor_all(futures[0],futures[1],futures[2],futures[3]);
    
    for(unsigned j=0;j<count;++j)
    {
        BOOST_CHECK(futures[j].is_ready());
    }
}

void wait_for_all_five_futures()
{
    unsigned const count=5;
    boost::fibers::unique_future<int> futures[count];
    for(unsigned j=0;j<count;++j)
    {
        boost::fibers::packaged_task<int> task(make_int_slowly);
        futures[j]=task.get_future();
        boost::fibers::fiber(boost::move(task)).detach();
    }
    
    boost::fibers::waitfor_all(futures[0],futures[1],futures[2],futures[3],futures[4]);
    
    for(unsigned j=0;j<count;++j)
    {
        BOOST_CHECK(futures[j].is_ready());
    }
}


void test_store_value_from_thread()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    store_value_from_thread();
}

void test_store_exception()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    store_exception();
}

void test_initial_state()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    initial_state();
}

void test_waiting_future()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    waiting_future();
}

void test_cannot_get_future_twice()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    cannot_get_future_twice();
}

void test_set_value_updates_future_state()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    set_value_updates_future_state();
}

void test_set_value_can_be_retrieved()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    set_value_can_be_retrieved();
}

void test_set_value_can_be_moved()
{
}

void test_future_from_packaged_task_is_waiting()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    future_from_packaged_task_is_waiting();
}

void test_invoking_a_packaged_task_populates_future()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    invoking_a_packaged_task_populates_future();
}

void test_invoking_a_packaged_task_twice_throws()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    invoking_a_packaged_task_twice_throws();
}

void test_cannot_get_future_twice_from_task()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    cannot_get_future_twice_from_task();
}

void test_task_stores_exception_if_function_throws()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    task_stores_exception_if_function_throws();
}

void test_void_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    void_promise();
}

void test_reference_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    reference_promise();
}

void test_task_returning_void()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    task_returning_void();
}

void test_task_returning_reference()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    task_returning_reference();
}

void test_shared_future()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    shared_future();
}

void test_copies_of_shared_future_become_ready_together()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    copies_of_shared_future_become_ready_together();
}

void test_shared_future_can_be_move_assigned_from_unique_future()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    shared_future_can_be_move_assigned_from_unique_future();
}

void test_shared_future_void()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    shared_future_void();
}

void test_shared_future_ref()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    shared_future_ref();
}

void test_can_get_a_second_future_from_a_moved_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    can_get_a_second_future_from_a_moved_promise();
}

void test_can_get_a_second_future_from_a_moved_void_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    can_get_a_second_future_from_a_moved_void_promise();
}
#if 0
void test_unique_future_for_move_only_udt()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    unique_future_for_move_only_udt();
}
#endif
void test_unique_future_for_string()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    unique_future_for_string();
}

void test_wait_callback()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_callback();
}
#if 0
void test_wait_callback_with_timed_wait()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_callback_with_timed_wait();
}
#endif
void test_wait_callback_for_packaged_task()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_callback_for_packaged_task();
}

void test_packaged_task_can_be_moved()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    packaged_task_can_be_moved();
}

void test_destroying_a_promise_stores_broken_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    destroying_a_promise_stores_broken_promise();
}

void test_destroying_a_packaged_task_stores_broken_promise()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    destroying_a_packaged_task_stores_broken_promise();
}

void test_wait_for_either_of_two_futures_1()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_two_futures_1();
}

void test_wait_for_either_of_two_futures_2()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_two_futures_2();
}

void test_wait_for_either_of_three_futures_1()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_three_futures_1();
}

void test_wait_for_either_of_three_futures_2()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_three_futures_2();
}

void test_wait_for_either_of_three_futures_3()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_three_futures_3();
}

void test_wait_for_either_of_four_futures_1()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_four_futures_1();
}

void test_wait_for_either_of_four_futures_2()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_four_futures_2();
}

void test_wait_for_either_of_four_futures_3()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_four_futures_3();
}

void test_wait_for_either_of_four_futures_4()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_four_futures_4();
}

void test_wait_for_either_of_five_futures_1()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_five_futures_1();
}

void test_wait_for_either_of_five_futures_2()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_five_futures_2();
}

void test_wait_for_either_of_five_futures_3()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_five_futures_3();
}

void test_wait_for_either_of_five_futures_4()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_five_futures_4();
}

void test_wait_for_either_of_five_futures_5()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_of_five_futures_5();
}

void test_wait_for_either_invokes_callbacks()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_either_invokes_callbacks();
}

void test_wait_for_any_from_range()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_any_from_range();
}

void test_wait_for_all_from_range()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_all_from_range();
}

void test_wait_for_all_two_futures()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_all_two_futures();
}

void test_wait_for_all_three_futures()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_all_three_futures();
}

void test_wait_for_all_four_futures()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_all_four_futures();
}

void test_wait_for_all_five_futures()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    wait_for_all_five_futures();
}

void test_future_wait()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

    future_wait();
}

boost::unit_test_framework::test_suite* init_unit_test_suite(int, char*[])
{
    boost::unit_test_framework::test_suite* test =
        BOOST_TEST_SUITE("Boost.Fiber: futures test suite");

    test->add(BOOST_TEST_CASE(test_initial_state));
    test->add(BOOST_TEST_CASE(test_waiting_future));
    test->add(BOOST_TEST_CASE(test_cannot_get_future_twice));
    test->add(BOOST_TEST_CASE(test_set_value_updates_future_state));
    test->add(BOOST_TEST_CASE(test_set_value_can_be_retrieved));
    test->add(BOOST_TEST_CASE(test_set_value_can_be_moved));
    test->add(BOOST_TEST_CASE(test_store_value_from_thread));
    test->add(BOOST_TEST_CASE(test_store_exception));
    test->add(BOOST_TEST_CASE(test_future_from_packaged_task_is_waiting));
    test->add(BOOST_TEST_CASE(test_invoking_a_packaged_task_populates_future));
    test->add(BOOST_TEST_CASE(test_invoking_a_packaged_task_twice_throws));
    test->add(BOOST_TEST_CASE(test_cannot_get_future_twice_from_task));
    test->add(BOOST_TEST_CASE(test_task_stores_exception_if_function_throws));
    test->add(BOOST_TEST_CASE(test_void_promise));
    test->add(BOOST_TEST_CASE(test_reference_promise));
    test->add(BOOST_TEST_CASE(test_task_returning_void));
    test->add(BOOST_TEST_CASE(test_task_returning_reference));
    test->add(BOOST_TEST_CASE(test_shared_future));
    test->add(BOOST_TEST_CASE(test_copies_of_shared_future_become_ready_together));
    test->add(BOOST_TEST_CASE(test_shared_future_can_be_move_assigned_from_unique_future));
    test->add(BOOST_TEST_CASE(test_shared_future_void));
    test->add(BOOST_TEST_CASE(test_shared_future_ref));
    test->add(BOOST_TEST_CASE(test_can_get_a_second_future_from_a_moved_promise));
    test->add(BOOST_TEST_CASE(test_can_get_a_second_future_from_a_moved_void_promise));
//  test->add(BOOST_TEST_CASE(test_unique_future_for_move_only_udt));
    test->add(BOOST_TEST_CASE(test_unique_future_for_string));
    test->add(BOOST_TEST_CASE(test_wait_callback));
#if 0
    test->add(BOOST_TEST_CASE(test_wait_callback_with_timed_wait));
#endif
    test->add(BOOST_TEST_CASE(test_wait_callback_for_packaged_task));
    test->add(BOOST_TEST_CASE(test_packaged_task_can_be_moved));
    test->add(BOOST_TEST_CASE(test_destroying_a_promise_stores_broken_promise));
    test->add(BOOST_TEST_CASE(test_destroying_a_packaged_task_stores_broken_promise));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_two_futures_1));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_two_futures_2));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_three_futures_1));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_three_futures_2));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_three_futures_3));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_four_futures_1));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_four_futures_2));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_four_futures_3));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_four_futures_4));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_five_futures_1));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_five_futures_2));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_five_futures_3));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_five_futures_4));
    test->add(BOOST_TEST_CASE(test_wait_for_either_of_five_futures_5));
    test->add(BOOST_TEST_CASE(test_wait_for_either_invokes_callbacks));
    test->add(BOOST_TEST_CASE(test_wait_for_any_from_range));
    test->add(BOOST_TEST_CASE(test_wait_for_all_from_range));
    test->add(BOOST_TEST_CASE(test_wait_for_all_two_futures));
    test->add(BOOST_TEST_CASE(test_wait_for_all_three_futures));
    test->add(BOOST_TEST_CASE(test_wait_for_all_four_futures));
    test->add(BOOST_TEST_CASE(test_wait_for_all_five_futures));
    test->add(BOOST_TEST_CASE(test_future_wait));

    return test;
}
