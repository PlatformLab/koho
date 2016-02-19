/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <algorithm>
#include "epoller.hh"
#include "exception.hh"

using namespace std;
using namespace EpollerShortNames;

Epoller::Epoller( )
    : epoll_fd_(-1),
    actions_()
{
    epoll_fd_ = SystemCall( "epoll_create", ::epoll_create( 1 ) );
}

void Epoller::add_action( Epoller::Action && action )
{
    assert( actions_.count( action.fd.fd_num() ) == 0 );
    actions_.emplace( action.fd.fd_num(), action );

    epoll_event ev;
    ev.events = action.direction == Direction::In ? EPOLLIN : EPOLLOUT;
    ev.data.fd = action.fd.fd_num();

    SystemCall( "epoll_ctl", ::epoll_ctl( epoll_fd_, EPOLL_CTL_ADD, action.fd.fd_num(), &ev ) );
}

void Epoller::remove_action( int file_descriptor )
{
    // Note: "Applications that need to be portable to kernels before 2.6.9 should specify a non-null pointer in event"
    SystemCall( "epoll_ctl", ::epoll_ctl( epoll_fd_, EPOLL_CTL_DEL, file_descriptor, NULL ) );

    size_t num_erased = actions_.erase( file_descriptor );
    if ( num_erased != 1 ) {
        cerr << "problem erasing fd " << file_descriptor << endl;
    }
    cerr << "epoll erased fd " << file_descriptor << endl;
}

unsigned int Epoller::Action::service_count( void ) const
{
    return direction == Direction::In ? fd.read_count() : fd.write_count();
}

Epoller::Result Epoller::poll( const int & timeout_ms )
{
    const int max_events = 1000;
    struct epoll_event events[max_events];

    int num_fds = SystemCall( "epoll_ctl", ::epoll_wait( epoll_fd_, events, max_events, timeout_ms ) );
   
    for (int n = 0; n < num_fds; n++) {
        auto action_it = actions_.find( events[n].data.fd );
        assert( action_it != actions_.end() );
        const auto service_count_before_callback = action_it->second.service_count();
        auto callback_result = action_it->second.callback();

        if ( callback_result.result == ResultType::Exit) {
            return Result( Result::Type::Exit, callback_result.exit_status );
        }

        if ( action_it->second.service_count() == service_count_before_callback ) {
            throw runtime_error( "Epoller: busy wait detected: callback did not read/write fd" );
        }
    }
    return Result::Type::Success;
}
