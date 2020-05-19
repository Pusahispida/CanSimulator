/*!
* \file
* \brief test_CLI_commandline_parser.cpp foo
*/

#include "../cli/commandlineparser.cpp"
#include "../lib/logger.cpp"
#include <gtest/gtest.h>


void cleanup_testcase() {
    params.asc = "";
    params.cfg = "";
    params.dbc = "";
    params.filters = "";
    params.filterExclude = false;
    params.ignoreDirections = false;
    params.interface = "";
    params.native = false;
    params.runTime = -1;
    params.sendTime = true;
    params.utcTime = false;
    params.suppressDefaults = false;
    params.verbosity = 4;
    params.command = "";
    params.commandParameters.clear();
    optind=0;
}

TEST(CLI_commandline_parser, defaults) {
    Logger::getLogger().setVerbosity(0);
    char* argv[] = {strdup("test"), NULL};
    bool ret = parseCommandLineArguments(1, argv);

    ASSERT_EQ("", params.asc);
    ASSERT_EQ("", params.cfg);
    ASSERT_EQ("", params.dbc);
    ASSERT_EQ("", params.filters);
    ASSERT_FALSE(params.filterExclude);
    ASSERT_EQ(false, params.ignoreDirections);
    ASSERT_EQ("can0", params.interface);
    ASSERT_EQ(false, params.native);
    ASSERT_EQ(-1, params.runTime);
    ASSERT_EQ(true, params.sendTime);
    ASSERT_EQ(false, params.utcTime);
    ASSERT_EQ(false, params.suppressDefaults);
    ASSERT_EQ(4, params.verbosity);
    ASSERT_EQ("", params.command);
    ASSERT_EQ(0, params.metricsSeparator);
    ASSERT_EQ("", params.metrics);
    ASSERT_TRUE(params.commandParameters.empty());
    ASSERT_FALSE(ret);
    cleanup_testcase();
}

TEST(CLI_commandline_parser, all_long_set) {
    Logger::getLogger().setVerbosity(0);
    char* argv[] = {strdup("test"),
        strdup("--asc=barfoo.asc"),
        strdup("--cfg=barfoo/cfg"),
        strdup("--dbc=barfoo.dbc"),
        strdup("--metrics=output"),
        strdup("--metricsSeparator=:"),
        strdup("--filterExclude=11,12,13"),
        strdup("--ignoreDirections"),
        strdup("--interface=bar0"),
        strdup("--native"),
        strdup("--no-send-time"),
        strdup("--utc"),
        strdup("--run-time=842"),
        strdup("--suppress-defaults"),
        strdup("--verbosity=2"),
        strdup("Command"), strdup("Argument1"), strdup("Argument2"), strdup("Argument3"),
        NULL};
    bool ret = parseCommandLineArguments(19, argv);

    ASSERT_EQ("barfoo.asc", params.asc);
    ASSERT_EQ("barfoo/cfg", params.cfg);
    ASSERT_EQ("barfoo.dbc", params.dbc);
    ASSERT_TRUE(params.filterExclude);
    ASSERT_EQ("11,12,13", params.filters);
    ASSERT_EQ(true, params.ignoreDirections);
    ASSERT_EQ("bar0", params.interface);
    ASSERT_EQ("output", params.metrics);
    ASSERT_EQ(':', params.metricsSeparator);
    ASSERT_EQ(true, params.native);
    ASSERT_EQ(842, params.runTime);
    ASSERT_EQ(false, params.sendTime);
    ASSERT_EQ(true, params.utcTime);
    ASSERT_EQ(true, params.suppressDefaults);
    ASSERT_EQ(2, params.verbosity);
    ASSERT_EQ("Command", params.command);
    ASSERT_EQ(3, params.commandParameters.size());
    ASSERT_EQ("Argument1", params.commandParameters[0]);
    ASSERT_EQ("Argument2", params.commandParameters[1]);
    ASSERT_EQ("Argument3", params.commandParameters[2]);
    ASSERT_TRUE(ret);
    cleanup_testcase();
}

TEST(CLI_commandline_parser, all_short_set) {
    Logger::getLogger().setVerbosity(0);
    char* argv[] = {strdup("test"),
        strdup("-a"), strdup("foobar.asc"),
        strdup("-c"), strdup("foobar/cfg"),
        strdup("-d"), strdup("foobar.dbc"),
        strdup("-m"), strdup("output2"),
        strdup("-M"), strdup(";-"),
        strdup("-F"), strdup("13,15,17"),
        strdup("-I"),
        strdup("-i"), strdup("foo0"),
        strdup("-n"),
        strdup("-r64738"),
        strdup("-s"),
        strdup("-t"),
        strdup("-u"),
        strdup("-v1"),
        strdup("command"), strdup("argument1"), strdup("argument2"), strdup("argument3"),
        NULL};
    bool ret = parseCommandLineArguments(26, argv);

    ASSERT_EQ("foobar.asc", params.asc);
    ASSERT_EQ("foobar/cfg", params.cfg);
    ASSERT_EQ("foobar.dbc", params.dbc);
    ASSERT_FALSE(params.filterExclude);
    ASSERT_EQ("13,15,17", params.filters);
    ASSERT_EQ(true, params.ignoreDirections);
    ASSERT_EQ("foo0", params.interface);
    ASSERT_EQ("output2", params.metrics);
    ASSERT_EQ(';', params.metricsSeparator);
    ASSERT_EQ(true, params.native);
    ASSERT_EQ(64738, params.runTime);
    ASSERT_EQ(false, params.sendTime);
    ASSERT_EQ(true, params.utcTime);
    ASSERT_EQ(true, params.suppressDefaults);
    ASSERT_EQ(1, params.verbosity);
    ASSERT_EQ("command", params.command);
    ASSERT_EQ(3, params.commandParameters.size());
    ASSERT_EQ("argument1", params.commandParameters[0]);
    ASSERT_EQ("argument2", params.commandParameters[1]);
    ASSERT_EQ("argument3", params.commandParameters[2]);
    ASSERT_TRUE(ret);
    cleanup_testcase();
}

TEST(CLI_commandline_parser, filterModes) {
    Logger::getLogger().setVerbosity(0);
    char* argv[] = {strdup("test"),
        strdup("--asc=barfoo.asc"),
        strdup("--cfg=barfoo/cfg"),
        strdup("--dbc=barfoo.dbc"),
        strdup("--filterInclude=10,120,13"),
        NULL};
    parseCommandLineArguments(5, argv);
    ASSERT_EQ("10,120,13", params.filters);
    ASSERT_FALSE(params.filterExclude);
    cleanup_testcase();

    char* argv2[] = {strdup("test"),
        strdup("--asc=barfoo.asc"),
        strdup("--cfg=barfoo/cfg"),
        strdup("--dbc=barfoo.dbc"),
        strdup("--filterExclude=101,125,213"),
        NULL};
    parseCommandLineArguments(5, argv2);
    ASSERT_EQ("101,125,213", params.filters);
    ASSERT_TRUE(params.filterExclude);
    cleanup_testcase();

    char* argv3[] = {strdup("test"),
        strdup("-a"), strdup("foobar.asc"),
        strdup("-c"), strdup("foobar/cfg"),
        strdup("-d"), strdup("foobar.dbc"),
        strdup("-F"), strdup("13,15,17"),
        NULL};
    parseCommandLineArguments(9, argv3);
    ASSERT_EQ("13,15,17", params.filters);
    ASSERT_FALSE(params.filterExclude);
    cleanup_testcase();

    char* argv4[] = {strdup("test"),
        strdup("-a"), strdup("foobar.asc"),
        strdup("-c"), strdup("foobar/cfg"),
        strdup("-d"), strdup("foobar.dbc"),
        strdup("-f"), strdup("13,15,17"),
        NULL};
    parseCommandLineArguments(9, argv4);
    ASSERT_EQ("13,15,17", params.filters);
    ASSERT_TRUE(params.filterExclude);
    cleanup_testcase();
}
