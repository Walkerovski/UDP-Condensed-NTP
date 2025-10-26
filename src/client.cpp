#include "parser_client.h"
#include "requests.h"

#include <iostream>
#include <random>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <time.h>

using namespace std;

timespec gettime() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        perror("clock_gettime");
    return ts;
}

int main(int argc, char *argv[]) {
    try {
        client_arguments args{};
        client_parseopt(args, argc, argv);
        struct timespec ts;

        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            cerr << "socket() failed: " << strerror(errno) << "\n";
            return 1;
        }

        struct timeval tv{};
        tv.tv_sec = args.timeout;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            cerr << "setsockopt(SO_RCVTIMEO) failed: " << strerror(errno) << "\n";
            return 1;
        }


        for (int i = 1; i <= args.reqnum; ++i) {

            TimeRequest req{};

            ts = gettime();
            req.setValues(
                static_cast<uint32_t>(i),
                static_cast<uint64_t>(ts.tv_sec),
                static_cast<uint64_t>(ts.tv_nsec),
                static_cast<uint32_t>(7)
            );
            req.sendTo(sockfd, args.addr);

            TimeResponse resp{};
            resp.receive(sockfd, args.addr);
            if (resp.version == 0) {
                cout << i << ": Droppped\n";
                continue;
            }

            ts = gettime();
            int64_t T0 = static_cast<int64_t>(resp.client_seconds);
            int64_t T1 = static_cast<int64_t>(resp.server_seconds);
            int64_t T2 = static_cast<int64_t>(ts.tv_sec);
            int64_t tetha = ((T1 - T0) + (T1 - T2))/2;
            int64_t delta = T2 - T0;
            cout << resp.sequence_number << ": " << tetha << " " << delta << "\n";
        }
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        cerr << "Unknown error occurred\n";
        return 1;
    }
}