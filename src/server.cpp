#include "parser_server.h"
#include "requests.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <time.h>
#include <unordered_map>


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

string buildClientKey(sockaddr_in client_addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    uint16_t port = ntohs(client_addr.sin_port);
    string key = string(ip) + ":" + to_string(port);
    return key;
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

    unordered_map<string, pair<uint32_t, time_t>> client_map;
    const int EXPIRATION = 120;

    while (true) {
        struct sockaddr_in client_addr;
        TimeRequest req{};
        req.receive(sockfd, client_addr);
        time_t now = time(nullptr);
        string key = buildClientKey(client_addr);
        if (!client_map.count(key) || (now - client_map[key].second) > EXPIRATION) {
            client_map[key] = {req.sequence_number, now};
        } else {
            uint32_t prev_max = client_map[key].first;

            if (req.sequence_number < prev_max) {
                cout << key << " " << req.sequence_number << " " << prev_max << "\n";
            } else if (req.sequence_number > prev_max) {
                client_map[key].first = req.sequence_number;
            }
            client_map[key].second = now;
        }

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
        if (rand() % 100 >= args.drop_rate) {
            resp.sendTo(sockfd, client_addr);
        }
    }
}