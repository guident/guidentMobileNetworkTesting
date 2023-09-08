//============================================================================================
//
// Copyright (c) 2020 Guident, Ltd. 
//
// Guident Remote Monitoring  -- Guident, Ltd -- Michael Trank  -- mtrank at guident dot com
//
//============================================================================================


#pragma once

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
//namespace ssl = boost::asio::ssl;       	// from <boost/asio/ssl.hpp>
