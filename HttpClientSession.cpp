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


#include "JtaNetworkTestingHub.h"
#include "HttpClientSession.h"


using namespace guident;



HttpClientSession::HttpClientSession(net::io_context & ioc, int t) 
	: __resolver(net::make_strand(ioc)), __stream(net::make_strand(ioc)), httpMethodVerb(http::verb::get), type(t) {

	//printf("HttpClientSession::HttpClientSession(): Constructor.\n");

}


HttpClientSession::~HttpClientSession() {
	//printf("HttpClientSession::~HttpClientSession(): Destructor.\n");
}

void HttpClientSession::onRun() {

}


void HttpClientSession::sendMessage(const std::string msg) {

}


void HttpClientSession::run(char const * host, char const * port, char const * target, int version) {

	// Set up an HTTP  request message
        __req.version(version);
        __req.method(httpMethodVerb);
        __req.target(target);
        __req.set(http::field::host, host);
        __req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	__req.set(http::field::authorization, "Basic YWRtaW46V0EyMTE4UEEwMDA4ODY=");

        //__sessionHandler->decorateRequest(__req);

        // Look up the domain name
	__resolver.async_resolve(host, port, beast::bind_front_handler(&HttpClientSession::onResolve, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));

}



void HttpClientSession::onResolve(beast::error_code ec, tcp::resolver::results_type results) {

	if ( ec ) {
		printf("HttpClientSession::onResolve(): Oops, error <<%d,%s>>\n", ec.value(), ec.message().c_str());
		__stream.close();
		return;
	}

	// Set a timeout on the operation
	beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(5));

        // Make the connection on the IP address we get from a lookup
	beast::get_lowest_layer(__stream).async_connect(results, beast::bind_front_handler(&HttpClientSession::onConnect, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));

}


void HttpClientSession::onConnect(beast::error_code ec,  tcp::resolver::results_type::endpoint_type et) {

	if ( ec ) {
		printf("HttpClientSession::onConnect(): Oops, error <<%d,%s>>.\n", ec.value(), ec.message().c_str());
		__stream.close();
		return;
	}

	beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(5));

	http::async_write(__stream, __req, beast::bind_front_handler(&HttpClientSession::onWrite, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));
}




void HttpClientSession::onWrite(beast::error_code ec, std::size_t bytes_transferred) {

	boost::ignore_unused(bytes_transferred);

	if ( ec ) {
		printf("HttpClientSession::onWrite(): Oops, error <<%d,%s>>.\n", ec.value(), ec.message().c_str());
		__stream.close();
		return;
	}

	beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(5));

	http::async_read(__stream, __buffer, __res, beast::bind_front_handler(&HttpClientSession::onRead, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));
	//__stream.async_read_some(__buffer, __res, beast::bind_front_handler(&HttpClientSession::onRead, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));

}



void HttpClientSession::onRead(beast::error_code ec, std::size_t bytes_transferred) {

	boost::ignore_unused(bytes_transferred);

	if ( ec ) {
		printf("HttpClientSession::onRead(): Oops, error <<%d,%s>>.\n", ec.value(), ec.message().c_str());
		__stream.close();
		return;
	}

	int resultCode = __res.result_int();

	if ( resultCode >= 200 && resultCode <= 299 ) {

		std::string resp = __res.body();
		//printf("HttpClientSession::onRead(): Response: <<%s>>\n", resp.c_str());
		if ( type == 1 ) {
			JtaNetworkTestingHub::Instance()->processGpsResponse(resp.c_str(), resp.size());
		} else {
			JtaNetworkTestingHub::Instance()->processModemStatsResponse(resp.c_str(), resp.size());
		}
		return;
	} else {

		std::string reason(__res.reason());
		printf("HttpClientSession::onRead(): Oops, bad response: <<%d>> <<%s>>.\n", resultCode, reason.c_str());
	}

	__buffer.consume(__buffer.size());

	beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(5));

	http::async_read(__stream, __buffer, __res, beast::bind_front_handler(&HttpClientSession::onRead, std::dynamic_pointer_cast<HttpClientSession>(shared_from_this())));

}



void HttpClientSession::setMethodVerb(http::verb method) {
        httpMethodVerb = method;
}


/*
template <class THub, class THandler>
void HttpClientSession<THub, THandler>::onShutdown(beast::error_code ec) {

	if ( ec == net::error::eof ) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }

	if ( ec ) {
		Log::Inst().Logm(Log::LOGLEVELERROR | Log::LOGTYPENETWORK, GetId(), "HttpClientSession::onShutdown(): Oops, error <<%d,%s>>.", ec.value(), ec.message().c_str());
	}

	// if we get here, shutdown is nice.
	
	THub::Inst().onConnectionDisconnect(this->GetId());

	return;
}
*/




