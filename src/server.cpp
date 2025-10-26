#include "parser_server.h"
#include "requests.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <time.h>


using namespace std;

timespec gettime() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        perror("clock_gettime");
    return ts;
}

bool initializeSocket(server_arguments& args, int sockfd) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(args.port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "bind() failed on port " << args.port << ": " << strerror(errno) << "\n";
        return true;
    }

    return false;
}


int main(int argc, char *argv[]) {
    server_arguments args{};
    server_parseopt(args, argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(initializeSocket(args, sockfd)) {
        cerr << "Server setup failed. Exiting cleanly.\n";
        close(sockfd);
        return 1;
    }

    while (true) {
        struct sockaddr_in client_addr;
        TimeRequest req{};
        req.receive(sockfd, client_addr);

        TimeResponse resp{};
        struct timespec ts;
        ts = gettime();
        resp.setValues(
            req.sequence_number,
            req.client_seconds,
            req.client_nanoseconds,
            static_cast<uint64_t>(ts.tv_sec),
            static_cast<uint64_t>(ts.tv_nsec)
        );
        resp.sendTo(sockfd, client_addr);
    }
}