

jta-network-test: 	main.o JtaNetworkTestingHub.o HttpClientSession.o
	g++ -o jta-network-test main.o JtaNetworkTestingHub.o HttpClientSession.o -lboost_system -lboost_thread -lpthread -lcrypto -lssl

main.o: main.cpp JtaNetworkTestingHub.h
	g++ -c -o main.o main.cpp

JtaNetworkTestingHub.o: JtaNetworkTestingHub.h JtaNetworkTestingHub.cpp
	g++ -c -o JtaNetworkTestingHub.o JtaNetworkTestingHub.cpp

HttpClientSession.o: HttpClientSession.h HttpClientSession.cpp JtaNetworkTestingHub.h
	g++ -c -o HttpClientSession.o HttpClientSession.cpp

clean:
	rm -f jta-network-test *.o >/dev/null 2>&1
