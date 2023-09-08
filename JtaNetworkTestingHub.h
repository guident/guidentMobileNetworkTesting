
#pragma once

#include "NetStuff.h"
#include "boost/thread.hpp"

namespace guident {

class JtaNetworkTestingHub {


public:

	~JtaNetworkTestingHub();

	static JtaNetworkTestingHub * Instance();

	void run();

	net::io_service & GetIoc();

	void processPingResponse(const char *resp);

	void processGpsResponse(const char * resp, size_t len);
	void processModemStatsResponse(const char * resp, size_t len);

	unsigned long long now_ms();

private:

	JtaNetworkTestingHub();

	static JtaNetworkTestingHub * __instance;

	void init();

	void onTimerCallback(const boost::system::error_code & err, boost::asio::deadline_timer* t, int * count);

	void threadHandler();

	bool retrieveIndexAndRttFromPingResponse(const char * str, unsigned long & idx, double & rtt);

	void storePingResponseData(unsigned long idx, double rtt);

	net::io_service _ioc;

        boost::asio::deadline_timer statusTimer;

        int statusTimerCounter;

	boost::thread __workerThread;
	pid_t pid;

	boost::mutex __mutex;

	bool gpsFix;


	//ping stuff
	bool missedPing;
	std::list<std::pair<unsigned long, double>> lastTwentyIndexesAndRtts;	


	// gps stuff
	double latitude, longitude;


	// modem stats stuff
	double rssi, rsrq, rsrp, sinr;

	
};

}
