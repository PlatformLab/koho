/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <algorithm>
#include "poller.hh"
#include "exception.hh"

using namespace std;
using namespace PollerShortNames;

void Poller::add_action( Poller::Action action )
{
    actions_.push_back( action );
    pollfds_.push_back( { action.fd.fd_num(), 0, 0 } );
}

unsigned int Poller::Action::service_count( void ) const
{
    return direction == Direction::In ? fd.read_count() : fd.write_count();
}

Poller::Result Poller::poll( const int & timeout_ms )
{
    assert( pollfds_.size() == actions_.size() );

    /* remove finished actions */
    auto action_it = actions_.begin();
    auto pollfd_it = pollfds_.begin();
    while ( action_it != actions_.end() and pollfd_it != pollfds_.end() ) {
        assert( pollfd_it->fd == action_it->fd.fd_num() );
        if ( action_it->when_remove() ) {
            action_it = actions_.erase( action_it );
            pollfd_it = pollfds_.erase( pollfd_it );
        } else {
            action_it++;
            pollfd_it++;
        }
    }

    /* tell poll whether we care about each fd */
    action_it = actions_.begin();
    for ( unsigned int i = 0; i < pollfds_.size(); i++, action_it++ ) {
        assert( pollfds_.at( i ).fd == action_it->fd.fd_num() );
        pollfds_.at( i ).events = (action_it->active and action_it->when_interested())
            ? action_it->direction : 0;

        /* don't poll in on fds that have had EOF */
        if ( action_it->direction == Direction::In
             and action_it->fd.eof() ) {
            pollfds_.at( i ).events = 0;
        }
    }

    /* Quit if no member in pollfds_ has a non-zero direction */
    if ( not accumulate( pollfds_.begin(), pollfds_.end(), false,
                         [] ( bool acc, pollfd x ) { return acc or x.events; } ) ) {
        return Result::Type::Exit;
    }

    if ( 0 == SystemCall( "poll", ::poll( &pollfds_[ 0 ], pollfds_.size(), timeout_ms ) ) ) {
        return Result::Type::Timeout;
    }

    action_it = actions_.begin();
    for ( unsigned int i = 0; i < pollfds_.size(); i++, action_it++ ) {
        if ( pollfds_[ i ].revents & (POLLERR | POLLHUP | POLLNVAL) ) {
            //            throw Exception( "poll fd error" );
            return Result::Type::Exit;
        }

        if ( pollfds_[ i ].revents & pollfds_[ i ].events ) {
            /* we only want to call callback if revents includes
               the event we asked for */
            const auto count_before = action_it->service_count();
            auto result = action_it->callback();

            switch ( result.result ) {
            case ResultType::Exit:
                return Result( Result::Type::Exit, result.exit_status );
            case ResultType::Cancel:
                action_it->active = false;
                break;
            case ResultType::Continue:
                break;
            }

            if ( count_before == action_it->service_count() ) {
                throw runtime_error( "Poller: busy wait detected: callback did not read/write fd" );
            }
        }
    }

    return Result::Type::Success;
}
