/*!
* \file
* \brief configuration.h foo
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <can-dbcparser/header/attribute.hpp>
#include "canmessage.h"
#include "cansignal.h"
#include "value.h"
#include <exception>
#include <jansson.h>
#include <map>
#include <set>
#include <string>
#include <map>

class ConfigurationException : public std::exception
{
  public:

  virtual const char *what() const throw();
};

class Configuration
{
public:
    Configuration(const std::string &cfg, const std::string &dbc, bool suppressDefaults = false, bool ignoreDirections=false);
    ~Configuration();

    std::string getCfgVersion() const;
    std::string getDBCVersion() const;
    CANSignal *getSignal(const std::string &key);
    CANMessage *getMessage(const std::string &key);
    CANMessage *getMessage(const std::uint32_t &msgId);
    bool isVariableSupported(const std::string &key);
    void setDefaultValues();
    Value getDefaultValue(const std::string &key);
    Value getValue(const std::string &key);
    bool setValue(const std::string &key, Value value);
    bool setValue(const std::string &key, std::string value);
    bool getMessageId(const std::string &key, std::uint32_t &msgId);
    const std::set<std::uint32_t> &getSendIDs() const;
    const std::set<std::uint32_t> &getReceiveIDs() const;
    const std::set<std::string> &getVariables() const;
    const Attribute *getAttribute(const std::string &name) const;
    void setAttribute(const Attribute &attr);
    const std::map<std::string, Attribute> &getAttributes() const {
        return m_attributes;
    }
    const std::map<std::uint32_t, CANMessage> &getMessages() const { return m_messages; }
    bool createFilterList(std::map<std::uint32_t, bool> &list);

private:
    std::string m_cfgFile;
    std::string m_dbcFile;
    json_t *m_canMapping;
    bool m_suppressDefaults;
    bool m_ignoreDirections;
    std::set<std::uint32_t> m_sendIDs;
    std::set<std::uint32_t> m_receiveIDs;
    std::set<std::string> m_variables;
    std::map<std::uint32_t, CANMessage> m_messages;
    std::map<std::string, Attribute> m_attributes;

    void initSignalSettings();
    void initMessageDirections();
    bool getSignalName(const std::string &key, std::string &SignalName);
    void initCANMessageDirections();
};

#endif // CONFIGURATION_H
