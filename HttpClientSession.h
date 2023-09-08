#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>


#include "NetStuff.h"
#include "BeastStuff.h"


namespace guident {

class HttpClientSession : public std::enable_shared_from_this<HttpClientSession> {

public:

	HttpClientSession(net::io_context & ioc, int t);
	~HttpClientSession();

	void onRun();
	void sendMessage(const std::string msg);

	void run(char const *host, char const * port, char const *target, int version);
        void onResolve(beast::error_code ec, tcp::resolver::results_type results);
	void onConnect(beast::error_code ec,  tcp::resolver::results_type::endpoint_type et);
        void onRead(beast::error_code ec, std::size_t bytes_transferred);
        void onWrite(beast::error_code ec, std::size_t bytes_transferred);
        void onShutdown(beast::error_code ec);

	void fail(beast::error_code ec, const char * what);

	void setMethodVerb(http::verb method);

private:

	tcp::resolver __resolver;
	beast::tcp_stream __stream;
	beast::flat_buffer __buffer; // (Must persist between reads)
	//http::request<http::empty_body> __req;
	http::request<http::string_body> __req;
	http::response<http::string_body> __res;

	http::verb httpMethodVerb;

	int type;
};


}
