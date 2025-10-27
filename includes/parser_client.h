#ifndef PARSER_CLIENT_H
#define PARSER_CLIENT_H

#include <netinet/in.h>
#include <string>
#include <stdio.h>
#include <argp.h>

// Struct to hold parsed arguments
struct client_arguments {
    struct sockaddr_in addr;
    int reqnum = -1;
    int timeout = -1;
    bool condensed = false;
};

/* Verifies whether provided string can be parsed as numbers
 * On sucess returns True on fail False
 */
bool isNumber(const std::string& s);

/* Parse client command-line options. Supports:
 *   -a / --addr    : required IPv4 address (validated with inet_pton)
 *   -p / --port    : required port number (range 1025–65535)
 *   -n / --number  : required number of time requests (>= 0)
 *   -t / --timeout : optional time before timeout (seconds, 0 -> indefinitely)
 *
 * Called by argp for each option. Performs validation and fills
 * a client_arguments struct. On invalid or missing options, reports
 * errors via argp_error and terminates execution.
 */
error_t client_parser(int key, char *arg, struct argp_state *state);

/* Parse all client command-line arguments using argp.
 * Defines supported options (addr, port, number, timeout),
 * delegates validation to client_parser, and fills a client_arguments struct.
 * On parse failure, prints an error; on success, prints the parsed values.
 */
void client_parseopt(client_arguments& args, int argc, char *argv[]);

#endif // PARSER_CLIENT_H