/*!
* \file
* \brief commandlineparser.cpp foo
*/

#include "commandlineparser.h"
#include "logger.h"
#include <getopt.h>
#include <stdexcept>

parameters params;

/*!
 * \brief parseCommandLineArguments
 * Parse commandline parameters
 * \param argc: number of command line parameters
 * \param argv: command line parameters
 * \return True if successful, false on failure or when help was requested.
 */
bool parseCommandLineArguments(int argc, char *argv[])
{
    static struct option long_options[] =
    {
        {"asc",               required_argument, 0, 'a'},
        {"cfg",               required_argument, 0, 'c'},
        {"dbc",               required_argument, 0, 'd'},
        {"filterExclude",     required_argument, 0, 'f'},
        {"filterInclude",     required_argument, 0, 'F'},
        {"help",              no_argument,       0, 'h'},
        {"ignoreDirections",  no_argument,       0, 'I'},
        {"interface",         required_argument, 0, 'i'},
        {"metrics",           required_argument, 0, 'm'},
        {"metricsSeparator",  required_argument, 0, 'M'},
        {"native",            no_argument,       0, 'n'},
        {"run-time",          required_argument, 0, 'r'},
        {"suppress-defaults", no_argument,       0, 's'},
        {"no-send-time",      no_argument,       0, 't'},
        {"utc",               no_argument,       0, 'u'},
        {"verbosity",         required_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    // Initialize defaults
    params.filterExclude = false;
    params.ignoreDirections = false;
    params.interface = "can0";
    params.metrics = "";
    params.metricsSeparator = 0;
    params.native = false;
    params.runTime = -1;
    params.suppressDefaults = false;
    params.sendTime = true;
    params.utcTime = false;
    params.verbosity = LOG_INFO;

    while (1) {
        // Current option index.
        int option_index = 0;

        int c = getopt_long(argc, argv, "a:c:d:f:F:m:M:Ii:hmnr:stuv:",
                        long_options, &option_index);
        // Detect the end of the options.
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'a':
                params.asc = optarg;
                break;
            case 'c':
                params.cfg = optarg;
                break;
            case 'd':
                params.dbc = optarg;
                break;
            case 'f':
                params.filterExclude = true;
                params.filters = optarg;
                break;
            case 'F':
                params.filterExclude = false;
                params.filters = optarg;
                break;
            case 'h':
                return false;
                break;
            case 'i':
                params.interface = optarg;
                break;
            case 'I':
                params.ignoreDirections = true;
                break;
            case 'm':
                params.metrics = optarg;
                break;
            case 'M':
                params.metricsSeparator = optarg[0];
                break;
            case 'n':
                params.native = true;
                break;
            case 'r':
                try {
                    params.runTime = std::stoi(optarg, 0);
                }
                catch (std::invalid_argument) {
                    LOG(LOG_ERR, "error=1 Invalid value for runTime.\n");
                    return false;
                }
                break;
            case 's':
                params.suppressDefaults = true;
                break;
            case 't':
                params.sendTime = false;
                break;
            case 'u':
                params.utcTime = true;
                break;
            case 'v':
                try {
                    params.verbosity = std::stoi(optarg, 0);
                }
                catch (std::invalid_argument) {
                    LOG(LOG_ERR, "error=1 Invalid value for verbosity.\n");
                    return false;
                }
                break;
            default:
                return false;
        }
    }
    // Check that required parameters are set
    if (params.asc.empty() && (params.cfg.empty() || params.dbc.empty())) {
        LOG(LOG_ERR, "error=1 'asc' or both 'dbc' and 'cfg' options are required.\n");
        return false;
    }

    /* Get command and any command parameters */
    if (optind < argc) {
        // The first parameter is the command
        params.command = argv[optind++];
        // Remaining parameters are for the command
        while (optind < argc) {
            params.commandParameters.insert(params.commandParameters.end(), argv[optind++]);
        }
    } else {
        return false;
    }

    return true;
}
