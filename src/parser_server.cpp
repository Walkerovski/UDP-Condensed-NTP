#include "parser_server.h"

#include <iostream>
#include <cstring>
#include <cstdlib> 

using namespace std;

bool isNumber(const string& s) {
    for (char it:s) {
        if (!isdigit(it))
			return false;
    }
    return true;
}

error_t server_parser(int key, char *arg, struct argp_state *state) {
	struct server_arguments *args = (server_arguments* )(state->input);
	error_t ret = 0;
	switch(key) {
	case 'p':
		if (!isNumber(arg))
			argp_error(state, "Invalid option for a port, must be a number!");

		if (atoi(arg) < 1025 || atoi(arg) > 65535)
			argp_error(state, "Port is supposed to be a value in between 1025 and 65535!");

		args->port = atoi(arg);

		break;
	case 'd':		
		if (!isNumber(arg))
			argp_error(state, "Invalid option for drop rate, must be a number!");

		if (atoi(arg) < 0 || atoi(arg) > 100)
			argp_error(state, "Droprate is supposed to be a value in between 0 and 100!");

		args->drop_rate = atoi(arg);

		break;
	case 'c':
		args->condensed = true;

		break;
    case ARGP_KEY_END:
        if (args->port == 0)
            argp_error(state, "Option -p (--port) is required!");

        break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

void server_parseopt(server_arguments& args, int argc, char *argv[]) {
		struct argp_option options[] = {
		{ "port", 'p', "port", 0, "The port to be used for the server", 0},
		{ "drop_rate", 'd', "drop_rate", 0, "The rate of dropping packets by the server. Zero by default", 0},
		{"condensed", 'c', 0, 0, "Use the condensed version of the message", 0},
		{ 0, 0, 0, 0, 0, 0}
	};

	struct argp argp_settings = { options, server_parser, 0, 0, 0, 0, 0 };

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0)
		cout << "Got an error condition when parsing\n";

    cout << "Port set for: " << args.port << "\n";
	cout << "Drop rate set for: " << args.drop_rate << "\n";
	cout << "Condensed mode: " << args.condensed << "\n";
}