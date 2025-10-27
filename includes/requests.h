#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

struct checksum_ctx;

/**
 * @brief Send a buffer over a socket.
 *
 * Attempts to send exactly @p size bytes from @p data using the socket
 * file descriptor @p sockfd. If the number of bytes sent does not match
 * the requested size, a std::runtime_error is thrown.
 *
 * @param sockfd   Socket file descriptor.
 * @param data     Pointer to the buffer to send.
 * @param size     Number of bytes to send.
 * @param errMsg   Error message included in the exception if sending fails.
 *
 * @throws std::runtime_error if sending fails or fewer than @p size bytes are sent.
 */
void sendAny(int sockfd, const void* data, ssize_t size, const char* errMsg);

/**
 * @brief Receive a fixed-size buffer from a socket.
 *
 * Reads exactly @p size bytes into @p buffer from the socket @p sockfd.
 * The function will loop until all bytes are received or an error occurs.
 *
 * @param sockfd   Socket file descriptor.
 * @param buffer   Pointer to the buffer where received data will be stored.
 * @param size     Number of bytes to receive.
 * @param errMsg   Error message included in the exception if receiving fails.
 *
 * @throws std::runtime_error if the socket is closed or an error occurs.
 */
void receiveAny(int sockfd, void* buffer, ssize_t size, const char* errMsg);

/* Protocol Structures */
struct TimeRequest {
    uint32_t sequence_number;
    uint32_t version;
    uint64_t client_seconds;
    uint64_t client_nanoseconds;

    void setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec, uint32_t ver=7);
    void sendTo(int sockfd, sockaddr_in& client_addr) const;
    void receive(int sockfd, sockaddr_in& client_addr);
};

struct TimeResponse {
    uint32_t sequence_number;
    uint32_t version;
    uint64_t client_seconds;
    uint64_t client_nanoseconds;
    uint64_t server_seconds;
    uint64_t server_nanoseconds;

    void setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec, 
                   uint64_t server_sec, uint64_t sever_nanosec, uint32_t ver=7);
    void sendTo(int sockfd, sockaddr_in& client_addr) const;
    void receive(int sockfd, sockaddr_in& client_addr);
};

#pragma pack(push, 1)
struct CondensedTimeRequest {
    uint32_t sequence_number;
    uint16_t version;
    uint64_t client_seconds;
    uint64_t client_nanoseconds;

    void setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec, uint16_t ver = 7);
    void sendTo(int sockfd, sockaddr_in& client_addr) const;
    void receive(int sockfd, sockaddr_in& client_addr);
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CondensedTimeResponse {
    uint32_t sequence_number;
    uint16_t version;
    uint64_t client_seconds;
    uint64_t client_nanoseconds;
    uint64_t server_seconds;
    uint64_t server_nanoseconds;

    void setValues(uint32_t seq_num, uint64_t client_sec, uint64_t client_nanosec,
                   uint64_t server_sec, uint64_t server_nanosec, uint16_t ver = 7);
    void sendTo(int sockfd, sockaddr_in& client_addr) const;
    void receive(int sockfd, sockaddr_in& client_addr);
};
#pragma pack(pop)


#endif // REQUESTS_H