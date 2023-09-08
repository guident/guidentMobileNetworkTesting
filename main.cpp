

#include "JtaNetworkTestingHub.h"
#include "HttpClientSession.h"


using namespace guident;

int main() {


	//std::shared_ptr<HttpClientSession> h = std::make_shared<HttpClientSession>(JtaNetworkTestingHub::Instance()->GetIoc());

	//h->run("192.168.0.1", "80", "/api/status/gps", 11);
	//h->run("192.168.0.1", "80", "/api/status/stats/signal_history/mdm-12359c2b9/", 11);

	JtaNetworkTestingHub::Instance()->run();

	return(0);
}






	//

	/*
	pid_t pid = fork();

        printf("Attempting fork!! %d\n", getpid());
        if ( pid == -1) {
                perror("oops fork");
                printf("huh????\n");
                exit(1);
        }

        printf("fork!! %d %d\n", pid, getpid());

        if ( pid == 0 ) {

                printf("JtaNetworkTestingHub::threadHandler(): Child process forked! %d", getpid());

                const char *argv[] = { "/usr/bin/ping", "-i", "0.2", "-s", "1024", 0 };
                //execv(argv[0], &argv[0]);
                execl("/bin/ping", "/bin/ping", "-i", "0.2", "-s", "512", "8.8.8.8", NULL);
                printf("JtaNetworkTestingHub::threadHandler(): Exiting Child process! %d", getpid());
                perror("execl");
                _exit(1);
        }


        while ( true ) {
                printf("loop in the thread %d\n", getpid());
                sleep(1);
        }

	*/

