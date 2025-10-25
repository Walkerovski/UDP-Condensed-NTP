#include "parser_client.h"

#include <iostream>
#include <cstring>
#include <cstdlib> 
#include <arpa/inet.h>

using namespace std;

bool isNumber(const string& s) {
    for (char it:s) {
        if (!isdigit(it)) return false;
    }
    return true;
}

error_t client_parser(int key, char *arg, struct argp_state *state) {
	struct client_arguments *args = (client_arguments *) state->input;
	error_t ret = 0;
	switch(key) {
	case 'a':
        if (inet_pton(AF_INET, arg, &args->addr.sin_addr) <= 0)
            argp_error(state, "Invalid address");

		args->addr.sin_family = AF_INET;

		break;
	case 'p':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for a port, must be a number!");

		if (atoi(arg) < 1025 || atoi(arg) > 65535)
			argp_error(state, "Port is supposed to be a value in between 1025 and 65535!");

		args->addr.sin_port = htons(atoi(arg));
		break;
	case 'n':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for the number of time requests (-n --number), must be a number!");

		args->reqnum = atoi(arg);
		if (args->reqnum < 0)
			argp_error(state, "The number of time requests (-n --number), must be >= 0");

		break;
	case 't':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for the lenght of timeout (-t --timeout), must be a number!");

		args->timeout = atoi(arg);
		if (args->timeout < 0)
			argp_error(state, "The lenght of timeout (-t --timeout), must be >= 0");

		break;
    case ARGP_KEY_END:
        if (args->addr.sin_addr.s_addr == INADDR_ANY)
            argp_error(state, "Option -a (--addr) is required!");
        if (args->addr.sin_port == 0)
            argp_error(state, "Option -p (--port) is required!");
        if (args->reqnum == -1)
            argp_error(state, "Option -n (--number) is required!");
        if (args->timeout == -1)
            argp_error(state, "Option -t (--timeout) is required!");
        break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

void client_parseopt(client_arguments& args, int argc, char *argv[]) {
	struct argp_option options[] = {
		{"addr", 'a', "addr", 0, "The IP address the server is listening at", 0},
		{"port", 'p', "port", 0, "The port that is being used at the server", 0},
		{"number", 'n', "num", 0, "The number of requests to send to the server", 0},
		{"timeout", 't', "timeout", 0, "The timeout for the client", 0},
		{0, 0, 0, 0, 0, 0}
	};

	struct argp argp_settings = { options, client_parser, 0, 0, 0, 0, 0};

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0)
		cout << "Got an error condition when parsing\n";

	cout << "Address: " << inet_ntoa(args.addr.sin_addr) << ":" << ntohs(args.addr.sin_port) << ", n="
        << args.reqnum << ", t=" <<  args.timeout << "\n";
}
