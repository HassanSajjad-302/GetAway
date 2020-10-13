//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "shared_state.hpp"
#include "tcp_session.hpp"


shared_state::
shared_state(){

}
void
shared_state::
join(tcp_session& session)
{
    sessions_.insert(&session);
}

void
shared_state::
leave(tcp_session& session)
{
    sessions_.erase(&session);
}

int shared_state::number_of_sessions()
{
    return sessions_.size();
}

void
shared_state::
state_send(std::string message,tcp_session* session)
{
    auto const ss = std::make_shared<std::string const>(std::move(message));

    for(auto sess: sessions_)
    {
        if(sess->id_!= session->id_){
            sess->session_send(ss);
        }
    }
}
