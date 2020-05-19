/*!
* \file
* \brief main.cpp foo
*/

#include "cansimulatorcore.h"
#include "commandlineparser.h"
#include "flood.h"
#include "logger.h"
#include "metrics.h"
#include "stringtools.h"
#include <inttypes.h>
#include <signal.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <vector>

// Global CANSimulatorCore
CANSimulatorCore *canSimulator;

// MetricsCollector
MetricsCollector *metrics = NULL;

// Main loop status
bool running = true;

/*!
 * \brief printHelp
 * Print program help
 */
void printHelp() {
    LOG(LOG_OUT,
"Usage: can-simulator-ng [options] ((-c FILE -d FILE) | -a FILE) <command> [parameters]\n\
  -a, --asc=FILE                ASC file\n\
  -c, --cfg=FILE                cfg file\n\
  -d, --dbc=FILE                dbc file\n\
[options]\n\
  -f, --filterExclude=ID,ID     List of all message ID's that will be excluded from sending, each separated by ,\n\
  -F, --filterInclude=ID,ID     List of all message ID's that will only be included in sending, each separated by ,\n\
  -I, --ignoreDirections        Ignore message directions defined in configuration\n\
  -i, --interface               CAN interface name (default: can0)\n\
  -m, --metrics=FILE            Metrics output file (without file extension)\n\
  -M, --metricsSeparator=CHAR   Metrics output file value separator character (default ;)\n\
  -n, --native                  Use native units instead of SI units\n\
  -r, --run-time=NUM            Run only for NUM seconds, use with automatic simulation\n\
  -s, --suppress-defaults       Suppress reporting incoming initial default values\n\
  -t, --no-send-time            Do not send time automatically\n\
  -u, --utc                     Use UTC time for automatic time sending\n\
  -v, --verbosity=NUM           Output verbosity (0: silent, 1: output, 2: errors, 3: warnings,\n\
                                                  4: additional info, 5: debug) (default: 4)\n\
<command>\n\
  flood                         Send random messages at given intervals\n\
    [parameters]\n\
    delay=VAL                   Set flood repeat interval VAL as usec\n\
    rate=VAL                    Set flood congestion percentage VAL to use calculated intervals based on CAN bitrate and message size\n\
    burst-len=VAL               Set and enable flood bursting time VAL in usec (if not set, will use burst-delay)\n\
    burst-delay=VAL             Set flood bursting delay time VAL in usec (if not set, will use burst-len)\n\
    include=VAR,VAR             List of messages that will be sent through flooding, each separated by ,\n\
    exclude=VAR,VAR             List of messages that will not be sent through flooding, each separated by ,\n\
  list [variable]               List all supported variables, or a single variable if defined\n\
  monitor                       Only listen to CAN bus\n\
  prompt                        Listen to command parameters from stdin\n\
    [parameters]\n\
      reset                     Reset to default values\n\
      VAR=VAL                   Set variable VAR to value VAL\n\
      exit/quit                 Exit simulator\n\
  send                          Send only command parameters\n\
    [parameters]\n\
      reset                     Reset to default values\n\
      VAR=VAL                   Set variable VAR to value VAL\n\
  simulate                      Use automatic simulation\n");
}

/*!
 * brief sigHandler
 * SIGINT/SIGTERM handler. Sets the main loop exit condition
 * \param signo: Signal number (irrelevant)
 */
void sigHandler(int signo)
{
    (void)signo;
    running = false;
}

/*!
 * \brief printSignalInfo
 * Print signal information
 * \param name: Variable name
 * \param details: Add also value descriptions
 */
void printSignalInfo(const std::string &name, bool details)
{
    const CANMessage *message = canSimulator->getMessage(name);
    if (message) {
        const CANSignal *signal = canSimulator->getSignal(name);
        if (signal) {
            LOG(LOG_OUT, name+"\n");
            LOG(LOG_INFO, signal->toString(details)+message->toString(details)+"\n");
        }
    }
}

/*!
 * \brief printMessageSignalInfo
 * Print signal information for all signals in a message
 * \param message: Pointer to message
 */
void printMessageSignalInfo(CANMessage *message)
{
    if (message) {
        std::map<std::string, CANSignal> signals = message->getSignals();
        for (auto it = signals.begin(); it != signals.end(); ++it) {
            if (it->second.isModified()) {
                if (!it->second.getVariableName().empty()) {
                    LOG(LOG_OUT, it->second.getVariableName() + "=" + it->second.getValue().toString() + "\n");
                }
                LOG(LOG_INFO, it->second.toString(false));
            }
        }
    }
}

/*!
 * \brief printSignalInfo
 * Print signal information
 * \param input: Signal list
 * \param printVersions: Print cfg and dbc versions before signal information
 */
void printSignalInfo(std::vector<std::string> &input, bool printVersions)
{
    if (!input.empty()) {
        for (std::vector<std::string>::iterator it = input.begin(); it != input.end(); ++it) {
            printSignalInfo(*it, true);
        }
    } else {
        if (printVersions) {
            LOG(LOG_OUT, "cfg version: " + canSimulator->getCfgVersion() +"\n");
            LOG(LOG_OUT, "dbc version: " + canSimulator->getDBCVersion() +"\n\n");
        }
        const std::set<std::string> &variables = canSimulator->getVariables();
        for (std::set<std::string>::iterator it = variables.begin(); it != variables.end(); ++it) {
            printSignalInfo(*it, false);
        }
    }
}

/*!
 * \brief waitInput
 * Wait for input from sockets
 * \param input_set: fd_set of monitored file descriptors
 * \param nfds: number of file descriptors to monitor
 * \return True if input is available, false otherwise.
 */
bool waitInput(fd_set &input_set, int nfds)
{
    // Wait 100 milliseconds for input
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    return select(nfds, &input_set, NULL, NULL, &timeout) > 0;
}

/*!
 * \brief processUserInput
 * Process user input from vector of strings
 * \param input: user input
 */
void processUserInput(std::vector<std::string> &input)
{
    if (input.empty()) {
        return;
    }
    if (input.front() == "reset") {
        // Reset default values
        canSimulator->setDefaultValues(true);
    } else if (input.front() == "quit" || input.front() == "exit") {
        // Exit main loop
        running = false;
    } else if (input.front() == "list") {
        // Remove command from vector
        input.erase(input.begin());
        // Print signal information
        printSignalInfo(input, false);
    } else {
        // Set values
        canSimulator->setValues(input);
    }
}

/*!
 * \brief mainLoop
 * Main loop for monitor, prompt and simulator modes
 * \param send: listen to user input from stdin
 * \return Exit code
 */
int mainLoop(bool send)
{
    fd_set input_set;
    int messageQueueFd = canSimulator->getMessageQueue()->getEventFd();
    while (running) {
        int nfds = 0;
        // Empty the FD set
        FD_ZERO(&input_set);
        if (send) {
            // Listen to stdin
            FD_SET(STDIN_FILENO, &input_set);
            nfds = STDIN_FILENO + 1;
        }
        if (messageQueueFd >= 0) {
            // Listen to incoming messages
            FD_SET(messageQueueFd, &input_set);
            nfds = ((messageQueueFd+1) > nfds) ? messageQueueFd+1 : nfds;
        }
        // Wait for input
        if (waitInput(input_set, nfds) && running) {
            // Check for data from stdin
            if (FD_ISSET(STDIN_FILENO, &input_set)) {
                std::string line;
                std::getline(std::cin, line);
                std::vector<std::string> userInput = split(line, ' ');
                processUserInput(userInput);
            }
            if (FD_ISSET(messageQueueFd, &input_set)) {
                //Print changed signals
                uint64_t val ;
                read(messageQueueFd, &val, sizeof(uint64_t));
                Queue<std::shared_ptr<CANMessage>> *messageQueue = canSimulator->getMessageQueue();
                while (!messageQueue->empty()) {
                    std::shared_ptr<CANMessage> message = messageQueue->pop();
                    printMessageSignalInfo(message.get());
                }
            }
        }
    }
    return 0;
}

/*!
 * \brief flooderLoop
 * CAN flooder loop
 * \param input: flood setting parameters
 */
int flooderLoop(std::vector<std::string> &input)
{
    CANSimulatorFloodMode *canFlooder;
    try {
        canFlooder = new CANSimulatorFloodMode(canSimulator, &input);
    }
    catch (CANSimulatorFloodException&) {
        LOG(LOG_ERR, "error=1 Floodmode failed to initialize!\n");
        return 1;
    }
    uint64_t sent = 0;
    canFlooder->initMetrics(metrics);
    while(running) {
        sent += (canFlooder->floodSignal()) ? 1 : 0;
    }
    LOG(LOG_OUT, "Sent %" PRIu64 " messages\n", sent);
    delete canFlooder;
    return 0;
}

/*!
 * \brief initializeMetrics
 * Inititalize MetricsCollector class to gather metrics, if metrics are enabled
 */
void initializeMetrics()
{
    if (!params.metrics.empty()) {
        try {
            metrics = new MetricsCollector(canSimulator, params.metrics);
            if (params.metricsSeparator) {
                metrics->setValueSeparator(params.metricsSeparator);
            }
        }
        catch (MetricsCollectorException&) {
            LOG(LOG_ERR, "error=1 Couldn't initialize metrics collection to '%s'\n", params.metrics.c_str());
        }
    }
}

/*!
 * \brief main
 * Main function of commandline interface for CAN simulator
 * \param argc: number of command line parameters
 * \param argv: command line parameters
 * \return Exit code
 */
int main(int argc, char *argv[])
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    setlinebuf(stdout);
    // Parse command line arguments
    if (!parseCommandLineArguments(argc, argv)) {
        printHelp();
        return 1;
    }
    // Set logging verbosity
    Logger::getLogger().setVerbosity(params.verbosity);

    if (params.command.compare("simulate") && !params.asc.empty()) {
        printHelp();
        return 1;
    }
    // Do not set CAN interface in list mode
    if (!params.command.compare("list")) {
        params.interface = "";
    }

    // Initialize CANSimulatorCore
    try {
        canSimulator = new CANSimulatorCore(params.cfg, params.dbc, params.asc, params.interface, params.suppressDefaults, params.ignoreDirections);
    }
    catch (CANSimulatorCoreException&) {
        return 2;
    }

    initializeMetrics();

    // Set parameters
    if (params.native) {
        canSimulator->setUseNativeUnits(params.native);
    }
    if (params.runTime >= 0) {
        canSimulator->setRunTime(params.runTime);
    }
    if (!params.sendTime) {
        canSimulator->setSendTime(params.sendTime);
    }
    if (params.utcTime) {
        canSimulator->setUseUTCTime(params.utcTime);
    }
    if (!params.filters.empty()) {
        std::vector<std::string> filters = split(params.filters, ',');
        if (!canSimulator->initializeMessageFilterList(&filters, params.filterExclude)) {
            LOG(LOG_ERR, "error=2 All messages have been filtered out! Aborting!\n");
            delete canSimulator;
            return 1;
        }
    }

    int retval = 0;
    // Handle commands
    if (!params.command.compare("flood")) {
        retval = flooderLoop(params.commandParameters);
    } else if (!params.command.compare("list")) {
        printSignalInfo(params.commandParameters, true);
        retval = 0;
    } else if (!params.command.compare("monitor")) {
        canSimulator->startCANReaderThread();
        retval = mainLoop(false);
    } else if (!params.command.compare("prompt")) {
        canSimulator->startCANReaderThread();
        canSimulator->startCANSenderThread();
        retval = mainLoop(true);
    } else if (!params.command.compare("send")) {
        processUserInput(params.commandParameters);
        canSimulator->sendCANMessages();
        retval = 0;
    } else if (!params.command.compare("simulate")) {
        canSimulator->startDataSimulator();
        retval = 0;
    } else {
        LOG(LOG_ERR, "error=1 Unknown command.\n");
        printHelp();
        retval = 1;
    }

    if (metrics != NULL) {
        metrics->writeToFile();
        delete metrics;
    }
    delete canSimulator;
    return retval;
}
