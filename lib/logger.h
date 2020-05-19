/*!
* \file
* \brief logger.h foo
*/

#ifndef CAN_LOGGER_H
#define CAN_LOGGER_H
#include <iostream>
#include <cstdarg>
#include <string>

// Log levels
#define LOG_SILENT  (0)
#define LOG_OUT     (1)
#define LOG_ERR     (2)
#define LOG_WARN    (3)
#define LOG_INFO    (4)
#define LOG_DBG     (5)

#define LOG Logger::getLogger().log

/*!
 * Singleton Logger Class.
 */
class Logger
{
public:
    void log(unsigned int level, const std::string& sMessage) const;
    void log(unsigned int level, const char *format, ...) const;
    static Logger& getLogger();
    void setVerbosity(unsigned int level);
private:
    Logger(): logLevel(-1) {};
    ~Logger() {};
    Logger(const Logger&) {};
    Logger& operator=(const Logger &logger) { logLevel = logger.logLevel; return *this; };
    unsigned int logLevel;
};
#endif
