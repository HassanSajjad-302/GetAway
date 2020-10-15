//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include <memory>
#include <string>
#include <unordered_set>


class tcp_session;

// Represents the shared server state
class shared_state
{
    // This simple method of tracking
    // sessions only works with an implicit
    // strand (i.e. a single-threaded server)
    std::unordered_set<tcp_session*> sessions_;

public:
    explicit
    shared_state();

    void join  (tcp_session& session);
    void leave (tcp_session& session);
    int number_of_sessions();
    void state_send  (std::string message, tcp_session* session);
};

#endif
