/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef EPOLLER_HH
#define EPOLLER_HH

#include <functional>
#include <vector>
#include <map>
#include <list>
#include <cassert>

#include "file_descriptor.hh"

#include <sys/epoll.h>

class Epoller
{
    int epoll_fd_;
public:
    struct Action
    {
        struct Result
        {
            enum class Type { Continue, Exit, Cancel } result;
            unsigned int exit_status;
            Result( const Type & s_result = Type::Continue, const unsigned int & s_status = EXIT_SUCCESS )
                : result( s_result ), exit_status( s_status ) {}
        };

        typedef std::function<Result(void)> CallbackType;

        FileDescriptor & fd;
        enum EpollDirection : short { In = EPOLLIN, Out = EPOLLOUT } direction;
        CallbackType callback;

        Action( FileDescriptor & s_fd,
                const EpollDirection & s_direction,
                const CallbackType & s_callback)
                
            : fd( s_fd ), direction( s_direction ), callback( s_callback ) {}

        unsigned int service_count( void ) const;
    };

private:
    std::map< int, Action > actions_; /* map from fd to action */

public:
    struct Result
    {
        enum class Type { Success, Timeout, Exit } result;
        unsigned int exit_status;
        Result( const Type & s_result, const unsigned int & s_status = EXIT_SUCCESS )
            : result( s_result ), exit_status( s_status ) {}
    };

    Epoller();
    void add_action( Action && action );
    void remove_action( int file_descriptor );

    Result poll( const int & timeout_ms );
};

namespace EpollerShortNames {
    typedef Epoller::Action::Result Result;
    typedef Epoller::Action::Result::Type ResultType;
    typedef Epoller::Action::EpollDirection Direction;
}

#endif
