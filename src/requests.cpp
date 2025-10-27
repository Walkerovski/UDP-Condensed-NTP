#include "requests.h"

#include <arpa/inet.h>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <iostream> 
#include <cstring>
#include <unistd.h>

#define ERR_SEND(cls) ("Sending " #cls " failed!")
#define ERR_RECV(cls) ("Receiving " #cls " failed!")

using namespace std;

uint64_t htonll(uint64_t value) {
    return ((uint64_t)htonl(value & 0xFFFFFFFFULL) << 32) | htonl(value >> 32);
}

uint64_t ntohll(uint64_t value) {
    return ((uint64_t)ntohl(value & 0xFFFFFFFFULL) << 32) | ntohl(value >> 32);
}


void TimeRequest::setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec, uint32_t ver) {
    sequence_number = htonl(seq_num);
    version = htonl(ver);
    client_seconds = htonll(client_sec);
    client_nanoseconds = htonll(client_nanosec);
}

void TimeRequest::sendTo(int sockfd, sockaddr_in& addr) const {
    ssize_t sent = sendto(sockfd, this, sizeof(*this), 0,
                    (struct sockaddr*)&addr, sizeof(addr));
    if (sent < 0) {
        cerr << "sendto() failed: " << strerror(errno) << "\n";
        close(sockfd);
        return;
    }
}

void TimeRequest::receive(int sockfd, sockaddr_in& addr) {
    socklen_t addr_len = sizeof(addr);
    ssize_t recv_len = recvfrom(sockfd, this, sizeof(*this), 0,
                                (struct sockaddr*)&addr, &addr_len);
    if (recv_len < 0) {
        if (errno != EWOULDBLOCK || errno != EAGAIN) {
            cerr << "recvfrom() failed: " << strerror(errno) << "\n";
            return;
        }
    }
    this->sequence_number = ntohl(this->sequence_number);
    this->version = ntohl(this->version);
    this->client_seconds = ntohll(this->client_seconds);
    this->client_nanoseconds = ntohll(this->client_nanoseconds);
}

void TimeResponse::setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec,
                             uint64_t server_sec, uint64_t sever_nanosec, uint32_t ver) {
    sequence_number = htonl(seq_num);
    version = htonl(ver);
    client_seconds = htonll(client_sec);
    client_nanoseconds = htonll(client_nanosec);
    server_seconds = htonll(server_sec);
    server_nanoseconds = htonll(sever_nanosec);
}

void TimeResponse::sendTo(int sockfd, sockaddr_in& addr) const {
    ssize_t sent = sendto(sockfd, this, sizeof(*this), 0,
                    (struct sockaddr*)&addr, sizeof(addr));
    if (sent < 0) {
        cerr << "sendto() failed: " << strerror(errno) << "\n";
        close(sockfd);
        return;
    }
}

void TimeResponse::receive(int sockfd, sockaddr_in& addr) {
    socklen_t addr_len = sizeof(addr);
    ssize_t recv_len = recvfrom(sockfd, this, sizeof(*this), 0,
                                (struct sockaddr*)&addr, &addr_len);
    if (recv_len < 0) {
        if (errno != EWOULDBLOCK || errno != EAGAIN) {
            cerr << "recvfrom() failed: " << strerror(errno) << "\n";
            return;
        }
    }
    this->sequence_number = ntohl(this->sequence_number);
    this->version = ntohl(this->version);
    this->client_seconds = ntohll(this->client_seconds);
    this->client_nanoseconds = ntohll(this->client_nanoseconds);
    this->server_seconds = ntohll(this->server_seconds);
    this->server_nanoseconds = ntohll(this->server_nanoseconds);
}

void CondensedTimeRequest::setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec, uint16_t ver) {
    version = htons(ver);
    sequence_number = htonl(seq_num);
    client_seconds = htonll(client_sec);
    client_nanoseconds = htonll(client_nanosec);
}

void CondensedTimeRequest::sendTo(int sockfd, sockaddr_in& addr) const {
    ssize_t sent = sendto(sockfd, this, sizeof(*this), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (sent < 0) {
        cerr << "sendto() failed: " << strerror(errno) << "\n";
        close(sockfd);
    }
}

void CondensedTimeRequest::receive(int sockfd, sockaddr_in& addr) {
    socklen_t addr_len = sizeof(addr);
    ssize_t recv_len = recvfrom(sockfd, this, sizeof(*this), 0, (struct sockaddr*)&addr, &addr_len);
    if (recv_len < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            cerr << "recvfrom() failed: " << strerror(errno) << "\n";
        return;
    }

    version = ntohs(version);
    sequence_number = ntohl(sequence_number);
    client_seconds = ntohll(client_seconds);
    client_nanoseconds = ntohll(client_nanoseconds);
}

void CondensedTimeResponse::setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec,
                                      uint64_t server_sec, uint64_t server_nanosec, uint16_t ver) {
    sequence_number = htonl(seq_num);
    version = htons(ver);
    client_seconds = htonll(client_sec);
    client_nanoseconds = htonll(client_nanosec);
    server_seconds = htonll(server_sec);
    server_nanoseconds = htonll(server_nanosec);
}

void CondensedTimeResponse::sendTo(int sockfd, sockaddr_in& addr) const {
    ssize_t sent = sendto(sockfd, this, sizeof(*this), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (sent < 0) {
        cerr << "sendto() failed: " << strerror(errno) << "\n";
        close(sockfd);
    }
}

void CondensedTimeResponse::receive(int sockfd, sockaddr_in& addr) {
    socklen_t addr_len = sizeof(addr);
    ssize_t recv_len = recvfrom(sockfd, this, sizeof(*this), 0, (struct sockaddr*)&addr, &addr_len);
    if (recv_len < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            cerr << "recvfrom() failed: " << strerror(errno) << "\n";
        return;
    }

    sequence_number = ntohl(sequence_number);
    version = ntohs(version);
    client_seconds = ntohll(client_seconds);
    client_nanoseconds = ntohll(client_nanoseconds);
    server_seconds = ntohll(server_seconds);
    server_nanoseconds = ntohll(server_nanoseconds);
}
