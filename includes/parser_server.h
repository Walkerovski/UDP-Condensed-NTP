#ifndef PARSER_SERVER_H
#define PARSER_SERVER_H

#include <string>
#include <argp.h>

// Struct to hold parsed arguments
struct server_arguments {
    int port;
    int drop_rate;
    bool condensed = false;
};

/* Verifies whether provided values can be parsed as numbers
 * On sucess returns True on fail False
 */
bool isNumber(const std::string& s);

/* Parse command-line options for the server. Depending on the key provided,
 * this function updates the server_arguments struct with the appropriate values:
 *   - 'p': sets the server port after validating that the argument is numeric
 *          and within the range [1025, 65535]. If invalid, an error is reported.
 *   - 'd': sets the drop rate value after validating that the argument is numeric.
 *   - ARGP_KEY_END: verifies that a port has been specified; otherwise reports an error.
 *
 * On success, returns 0. If the key is not recognized, returns ARGP_ERR_UNKNOWN.
 * The function uses argp_error to report invalid options or missing required arguments,
 * which will terminate the program with an error message.
 */
error_t server_parser(int key, char *arg, struct argp_state *state);

/* Parse server command-line options. Supports:
 *   -p / --port : required port number (validated range 1025–65535)
 *   -d / --drop_rate : optional drop rate (validated range 0-100)
 * Uses argp with server_parser for validation. On success, prints the
 * parsed values; on error, reports via argp_error or prints a message.
 */
void server_parseopt(server_arguments& args, int argc, char *argv[]);

#endif // PARSER_SERVER_H