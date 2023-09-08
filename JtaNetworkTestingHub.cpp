#include <cstdio>
#include <chrono>
#include <iostream>
#include <string>
#include <strstream>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#include "NetStuff.h"
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <boost/foreach.hpp>

#include "HttpClientSession.h"
#include "JtaNetworkTestingHub.h"


using namespace guident;


JtaNetworkTestingHub * JtaNetworkTestingHub::__instance = NULL;




JtaNetworkTestingHub::JtaNetworkTestingHub() : statusTimer(_ioc, boost::posix_time::milliseconds(2000)), __workerThread(boost::bind(&JtaNetworkTestingHub::threadHandler, this)), pid(0), latitude(0.0), longitude(0.0), rssi(-125.00), rsrq(-199.99), rsrp(-199.99), sinr(-199.99) {

}



JtaNetworkTestingHub::~JtaNetworkTestingHub() {

}



JtaNetworkTestingHub * JtaNetworkTestingHub::Instance() {

	if ( __instance == NULL ) {
		__instance = new JtaNetworkTestingHub();
	}
	return(__instance);
}



void JtaNetworkTestingHub::run() {
	init();
	_ioc.run();
}




void JtaNetworkTestingHub::init() {

	statusTimer.async_wait(boost::bind(&JtaNetworkTestingHub::onTimerCallback, this, boost::asio::placeholders::error, &statusTimer, &statusTimerCounter));

}




net::io_service & JtaNetworkTestingHub::GetIoc() {
        return(_ioc);
}




void JtaNetworkTestingHub::onTimerCallback(const boost::system::error_code & err, boost::asio::deadline_timer* t, int * count) {

        try {

                //printf("JtaNetworkTestingHub::onTimerCallback(): The timer fireed!!\n");

		std::shared_ptr<HttpClientSession> h1 = std::make_shared<HttpClientSession>(JtaNetworkTestingHub::Instance()->GetIoc(), 1);
        	h1->run("192.168.0.1", "80", "/api/status/gps", 11);

		std::shared_ptr<HttpClientSession> h2 = std::make_shared<HttpClientSession>(JtaNetworkTestingHub::Instance()->GetIoc(), 2);
        	h2->run("192.168.0.1", "80", "/api/status/stats/signal_history/mdm-12359c2b9/", 11);

                t->expires_at(t->expires_at() + boost::posix_time::milliseconds(2000));
                t->async_wait(boost::bind(&JtaNetworkTestingHub::onTimerCallback, this, boost::asio::placeholders::error, t, count));

        } catch(...) {

                printf("JtaNetworkTestingHub::onTimerCallback(): Oops, exception thrown!!\n");

        }
}


void JtaNetworkTestingHub::threadHandler() {


	int filedes[2];
	if ( pipe(filedes) == -1 ) {
		perror("pipe");
		exit(1);
	}

	pid_t pid = fork();

	//printf("Attempting fork!! %d\n", getpid());
	if ( pid == -1) {
  		perror("oops fork");
		printf("huh????\n");
  		exit(1);
	}
	
	//printf("fork!! %d\n", getpid());

	if ( pid == 0 ) {

		while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
		close(filedes[1]);
		close(filedes[0]);
		//execl("/bin/ping", "/bin/ping", "-n", "-i", "0.2", "-s", "512", "172.16.11.83", NULL);
		execl("/bin/ping", "/bin/ping", "-n", "-i", "0.2", "-s", "512", "10.10.116.129", NULL);
		//printf("JtaNetworkTestingHub::threadHandler(): Exiting Child process! %d", getpid());
		perror("execl");
		_exit(1);
	} 

	close(filedes[1]);
	
	char buffer[4096];
	while ( true ) {
		memset(buffer, 0, 4096);
		ssize_t count = read(filedes[0], buffer, sizeof(buffer)-1);
		if ( count == -1 ) {

		} else if ( count == 0 ) {
			break;
		} else {
			//printf("from child process: <<%s>>\n", buffer);
			this->processPingResponse(buffer);
		}
	}
	close(filedes[0]);
	wait(0);
}




void JtaNetworkTestingHub::processPingResponse(const char * resp) {

	unsigned long index = 0L;
	double rtt = 0.0;

	if ( retrieveIndexAndRttFromPingResponse(resp, index, rtt) ) {
		printf("Ping response: <<%llu>> <<%lu>> <<%5.3f>>\n", now_ms(), index, rtt);
		storePingResponseData(index, rtt);
	}

	{
		boost::mutex::scoped_lock lock(__mutex);
		int count = 0;
		double avg = 0.0;
		bool miss = false;
		std::list<std::pair<unsigned long, double>>::iterator iter = lastTwentyIndexesAndRtts.begin();
		if ( iter != lastTwentyIndexesAndRtts.end() ) {
			unsigned long lastidx = iter->first - 1;
			while (iter != lastTwentyIndexesAndRtts.end() ) {
				if ( iter->first != lastidx + 1 ) {
					miss = true;
				}
				avg += iter->second;
				++iter;
				lastidx = iter->first;
				count++;
			}
		}
		if ( count > 0 ) avg /= count; else avg = 0.0;
	}
}



bool JtaNetworkTestingHub::retrieveIndexAndRttFromPingResponse(const char * str, unsigned long & index, double & rtt) {

	if ( str == NULL ) return(false);
	if ( strlen(str) < 10 ) return(false);

	unsigned long idx = 0L;
	double tt = 0.0;

	std::istringstream f(str);
	std::string token;
	while (getline(f, token, ' ')) {

		if ( token.substr(0, 9) == "icmp_seq=" ) {
			idx = atol(token.substr(9).c_str());
		}

		if ( token.substr(0, 5) == "time=" ) {
			tt = atof(token.substr(5).c_str());
		}
	}

	if ( idx > 0 ) index = idx; else idx = 0L;
	if ( tt > 0.0 && tt < 1000.0 ) rtt = tt; else tt = 0.0;

	if ( idx == 0L || tt == 0.0 ) return(false);

	index = idx;
	rtt = tt;
	return(true);

}


void JtaNetworkTestingHub::storePingResponseData(unsigned long idx, double rtt) {

	if ( idx == 0 ) return;

	boost::mutex::scoped_lock lock(__mutex);

	std::pair<unsigned long, double> pingData(idx, rtt);

	lastTwentyIndexesAndRtts.push_back(pingData);

	while ( lastTwentyIndexesAndRtts.size() > 20 ) {
		lastTwentyIndexesAndRtts.pop_front();
	}

	return;
}





unsigned long long JtaNetworkTestingHub::now_ms() {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        unsigned long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return(millis);
}




void JtaNetworkTestingHub::processGpsResponse(const char * resp, size_t len) {

	if ( resp == NULL ) return;
	if ( len <= 3 ) return;


	try {

		std::istrstream is(resp);

		boost::property_tree::ptree pt;

		boost::property_tree::json_parser::read_json(is, pt);
	
		double latdeg = pt.get<double>("data.fix.latitude.degree");
		double latmin = pt.get<double>("data.fix.latitude.minute");
		double latsec = pt.get<double>("data.fix.latitude.second");

		double londeg = pt.get<double>("data.fix.longitude.degree");
		double lonmin = pt.get<double>("data.fix.longitude.minute");
		double lonsec = pt.get<double>("data.fix.longitude.second");

		double lat = latdeg + ( latmin / 60.0 ) + ( latsec / 3600.0);
		double lon = londeg - ( lonmin / 60.0 ) - ( lonsec / 3600.0);

		//printf("JtaNetworkTestingHub::processGpsResponse(): Latitude, Longitude is <<%10.7f,%10.7f>> \n", lat, lon);

		boost::mutex::scoped_lock _lock(__mutex);

		latitude = lat;
		longitude = lon;

		printf("Location: <<%llu>>, Latitude/Longitude: <<%10.7f,%10.7f\n", now_ms(), latitude, longitude);
	
	} catch(...) {

		boost::mutex::scoped_lock _lock(__mutex);

		latitude = 0.0;
		longitude = 0.0;

		printf("Location: <<%llu>>, Latitude/Longitude: <<%10.7f,%10.7f\n", now_ms(), latitude, longitude);
	}

	//printf("Location: <<%llu>>, Latitude/Longitude: <<%10.7f,%10.7f\n", now_ms(), latitude, longitude);

}



void JtaNetworkTestingHub::processModemStatsResponse(const char * resp, size_t len) {

	if ( resp == NULL ) return;
	if ( len <= 3 ) return;
	
	try {

		std::istrstream is(resp);

		boost::property_tree::ptree pt;

		boost::property_tree::json_parser::read_json(is, pt);

		boost::mutex::scoped_lock _lock(__mutex);

		unsigned long len = pt.get_child("data").size();

		int I = 0;
		BOOST_FOREACH(boost::property_tree::ptree::value_type & v, pt.get_child("data")) {
			//std::cout <<  I << " <<" << v.second.get<double>("RSSI") << ">>" << std::endl;
			rssi = v.second.get<double>("RSSI");
			if ( rssi > -125.00 ) {
				rsrq = v.second.get<double>("RSRQ");
				rsrp = v.second.get<double>("RSRP");
				sinr = v.second.get<double>("SINR");
			}
			I++;
		}

		printf("Modem stats: <<%llu>>, rssi: %6.3f, rsrq: %6.3f, rsrp: %6.3f, sinr: %6.3f\n", now_ms(), rssi, rsrq, rsrp, sinr);

	} catch(...) {

		boost::mutex::scoped_lock _lock(__mutex);

		printf("HUH???????\n");
		rssi = -125.00;
		rsrq = -199.99;
		rsrp = -199.99;
		sinr = -199.99;
		printf("Modem stats: <<%llu>>, rssi: %6.3f, rsrq: %6.3f, rsrp: %6.3f, sinr: %6.3f\n", now_ms(), rssi, rsrq, rsrp, sinr);
	}

}
