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

template <typename ReqType>
void sendRequest(
    int sockfd,
    int i,
    client_arguments& args
) {
    ReqType req{};
    struct timespec ts = gettime();
    req.setValues(
        static_cast<uint32_t>(i),
        static_cast<uint64_t>(ts.tv_sec),
        static_cast<uint64_t>(ts.tv_nsec),
        static_cast<uint32_t>(7)
    );
    req.sendTo(sockfd, args.addr);
}

template <typename RespType>
bool handleResponse(
    int sockfd,
    client_arguments& args,
    time_t& last_response_time,
    vector<bool>& received,
    vector<int64_t>& received_theta,
    vector<int64_t>& received_delta
) {
    RespType resp{};
    resp.receive(sockfd, args.addr);

    if (args.timeout > 0 && difftime(time(nullptr), last_response_time) >= args.timeout)
        return true;

    if (resp.sequence_number <= 0) {
        return false;
    }

    last_response_time = time(nullptr);
    struct timespec ts = gettime();
    int64_t T0 = static_cast<int64_t>(resp.client_seconds);
    int64_t T1 = static_cast<int64_t>(resp.server_seconds);
    int64_t T2 = static_cast<int64_t>(ts.tv_sec);

    int64_t theta = ((T1 - T0) + (T1 - T2))/2;
    int64_t delta = T2 - T0;

    received[resp.sequence_number] = true;
    received_theta[resp.sequence_number] = theta;
    received_delta[resp.sequence_number] = delta;

    return false;
}

int main(int argc, char *argv[]) {
    try {
        client_arguments args{};
        client_parseopt(args, argc, argv);

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

        vector<bool> received(args.reqnum + 1, false);
        vector<int64_t> received_theta(args.reqnum + 1, static_cast<int64_t>(0));
        vector<int64_t> received_delta(args.reqnum + 1, static_cast<int64_t>(0));

        for (int i = 1; i <= args.reqnum; ++i) {
            args.condensed
                ? sendRequest<CondensedTimeRequest>(sockfd, i, args)
                : sendRequest<TimeRequest>(sockfd, i, args);
        }

        time_t last_response_time = time(nullptr);
        for (int i = 1; i <= args.reqnum; ++i) {
            const bool timeout = args.condensed
                ? handleResponse<CondensedTimeResponse>(sockfd, args, last_response_time, received, received_theta, received_delta)
                : handleResponse<TimeResponse>(sockfd, args, last_response_time, received, received_theta, received_delta);

            if (timeout) break;
        }

        for (int i = 1; i <= args.reqnum; ++i) {
            received[i]
                ? cout << i << ": " << received_theta[i] << " " << received_delta[i] << "\n"
                : cout << i << ": Droppped\n";
        }
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        cerr << "Unknown error occurred\n";
        return 1;
    }
}