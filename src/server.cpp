#include "parser_server.h"
#include "requests.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <unordered_map>


using namespace std;

timespec getTime() {
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

string buildClientKey(sockaddr_in client_addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    uint16_t port = ntohs(client_addr.sin_port);
    string key = string(ip) + ":" + to_string(port);
    return key;
}

template <typename ReqType, typename RespType>
void handleRequest(
    int sockfd,
    unordered_map<string, pair<uint32_t, time_t>>& client_map,
    server_arguments args,
    const int EXPIRATION
) {
    struct sockaddr_in client_addr;
    ReqType req{};
    req.receive(sockfd, client_addr);
    
    time_t now = time(nullptr);
    string key = buildClientKey(client_addr);

    auto& entry = client_map[key];
    if (entry.second == 0 || (now - entry.second) > EXPIRATION) {
        entry = {req.sequence_number, now};
    } else {
        uint32_t prev_max = entry.first;

        if (req.sequence_number < prev_max) {
            cout << key << " " << req.sequence_number << " " << prev_max << "\n";
        } else if (req.sequence_number > prev_max) {
            entry.first = req.sequence_number;
        }
        entry.second = now;
    }
    RespType resp{};
    struct timespec ts;
    ts = getTime();
    resp.setValues(
        req.sequence_number,
        req.client_seconds,
        req.client_nanoseconds,
        static_cast<uint64_t>(ts.tv_sec),
        static_cast<uint64_t>(ts.tv_nsec)
    );
    if (rand() % 100 >= args.drop_rate) {
        resp.sendTo(sockfd, client_addr);
    }
}

int main(int argc, char *argv[]) {
    server_arguments args{};
    server_parseopt(args, argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    if(initializeSocket(args, sockfd)) {
        cerr << "Server setup failed. Exiting cleanly.\n";
        close(sockfd);
        return 1;
    }

    unordered_map<string, pair<uint32_t, time_t>> client_map;
    const int EXPIRATION = 120;

    srand(time(nullptr));

    while (true) {
        if (args.condensed) {
            handleRequest<CondensedTimeRequest, CondensedTimeResponse>(
                sockfd, client_map, args, EXPIRATION
            );
        } else {
            handleRequest<TimeRequest, TimeResponse>(
                sockfd, client_map, args, EXPIRATION
            );
        }
    }
}