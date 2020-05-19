/*!
* \file
* \brief logger.cpp foo
*/

#include "logger.h"

/*!
 * \brief Logger::getLogger
 * Get the reference to the Logger object
 * \return reference to the Logger object
 */
Logger &Logger::getLogger()
{
    static Logger instance;
    return instance;
}

/*!
 * \brief Logger::setVerbosity
 * Set logging verbosity
 * \param level: New log level
 */
void Logger::setVerbosity(unsigned int level)
{
    logLevel = level;
}

/*!
 * \brief Logger::log
 * Log a message in C style
 * \param level: log level
 * \param format: format string
 */
void Logger::log(unsigned int level, const char *format, ...) const
{
    if (level > logLevel) {
        return;
    }
    va_list args;
    va_start(args, format);
    if (level == LOG_DBG) {
#ifdef DEBUG
        vfprintf(stderr, format, args);
#endif
    } else if (level == LOG_INFO) {
        vfprintf(stderr, format, args);
    } else {
        vfprintf(stdout, format, args);
    }
    va_end(args);
}

/*!
 * \brief Logger::log
 * Log a message in C++ style
 * \param level: log level
 * \param message: string to log
 */
void Logger::log(unsigned int level, const std::string& message) const
{
    if (level > logLevel) {
        return;
    }
    if (level == LOG_DBG) {
#ifdef DEBUG
        std::cerr << message;
#endif
    } else if (level == LOG_INFO) {
        std::cerr << message;
    } else {
        std::cout << message;
    }
}
