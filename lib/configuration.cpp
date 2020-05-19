/*!
* \file
* \brief configuration.cpp foo
*/

#include "configuration.h"
#include "logger.h"
#include <can-dbcparser/header/dbciterator.hpp>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <utility>

const char *ConfigurationException::what() const throw()
{
    return "ConfigurationException";
}

/*!
 * \brief Configuration::Configuration
 * Constructor
 * \param cfg: path to configuration file
 * \param dbc: path to CAN specification dbc file
 * \param suppressDefaults: suppress reporting incoming initial default values
 * \param ignoreDirections: ignore message directions defined in configuration
 */
Configuration::Configuration(const std::string &cfg, const std::string &dbc, bool suppressDefaults, bool ignoreDirections) :
    m_cfgFile(cfg),
    m_dbcFile(dbc),
    m_canMapping(NULL),
    m_suppressDefaults(suppressDefaults),
    m_ignoreDirections(ignoreDirections)
{
    DBCIterator *m_dbcIterator = NULL;
    try {
        m_dbcIterator = new DBCIterator(m_dbcFile.c_str());
    }
    catch (const std::invalid_argument &argument) {
        LOG(LOG_ERR, "error=1 Cannot read CAN specification file.\n");
        throw ConfigurationException();
    }

    json_error_t error;
    m_canMapping = json_load_file(m_cfgFile.c_str(), 0, &error);
    if (!m_canMapping) {
        LOG(LOG_ERR, "error=1 Failed to parse CAN message mapping file on line %d: %s\n", error.line, error.text);
        delete m_dbcIterator;
        throw ConfigurationException();
    }

    initMessageDirections();
    for (auto it = m_dbcIterator->begin(); it != m_dbcIterator->end(); ++it) {
        if (m_sendIDs.count(it->second.getId()) || m_receiveIDs.count(it->second.getId())) {
            m_messages.insert({it->second.getId(), CANMessage(it->second)});
        }
    }
    // Copy global attributes
    DBCIterator::attributesMap attributes = m_dbcIterator->getAttributes();
    for (const auto &attribute : attributes) {
        if (attribute.second.getType().empty()) {
            m_attributes.insert({attribute.first, attribute.second});
        }
    }
    initSignalSettings();
    setDefaultValues();
    initCANMessageDirections();

    delete m_dbcIterator;
}

/*!
 * \brief Configuration::~Configuration
 * Destructor
 */
Configuration::~Configuration()
{
    if (m_canMapping) {
        json_decref(m_canMapping);
    }
}

/*!
 * \brief Configuration::getCfgVersion
 * Get cfg file date and revision
 * \return cfg file date and revision string
 */
std::string Configuration::getCfgVersion() const
{
    std::stringstream sstream;
    json_t *version = json_object_get(m_canMapping, "version");
    if (version) {
        if (json_object_get(version, "year") && json_object_get(version, "month") &&
            json_object_get(version, "day") && json_object_get(version, "revision")) {
            sstream << json_integer_value(json_object_get(version, "year")) << "-"
                    << std::setfill('0') << std::setw(2) << json_integer_value(json_object_get(version, "month")) << "-"
                    << std::setfill('0') << std::setw(2) << json_integer_value(json_object_get(version, "day"))
                    << " rev. " << json_integer_value(json_object_get(version, "revision"));
        }
    }
    return sstream.str();
}

/*!
 * \brief Configuration::getDBCVersion
 * Get DBC file date and revision
 * \return DBC file date and revision string
 */
std::string Configuration::getDBCVersion() const
{
    std::stringstream sstream;
    const Attribute *yearAttr = getAttribute("VersionYear");
    const Attribute *monthAttr = getAttribute("VersionMonth");
    const Attribute *dayAttr = getAttribute("VersionDay");
    const Attribute *numberAttr = getAttribute("VersionNumber");
    if (yearAttr && monthAttr && dayAttr && numberAttr) {
        sstream << yearAttr->getValue() << "-"
                << std::setfill('0') << std::setw(2) << monthAttr->getValue() << "-"
                << std::setfill('0') << std::setw(2) << dayAttr->getValue()
                << " rev. "<< numberAttr->getValue();
    }
    return sstream.str();
}

/*!
 * \brief Configuration::initSignalSettings
 * Initialize signal settings from configuration file
 */
void Configuration::initSignalSettings()
{
    json_t *signals = json_object_get(m_canMapping, "signals");
    if (signals) {
        const char *key;
        json_t *value;
        json_object_foreach(signals, key, value) {
            if (json_object_get(value, "id")) {
                CANSignal *sig = getSignal(key);
                if (sig) {
                    bool isDefaultValueSet = false;
                    if (json_t *obj = json_object_get(value, "type")) {
                        sig->setValueType(json_string_value(obj));
                    }
                    if (json_t *obj = json_object_get(value, "default")) {
                        isDefaultValueSet = sig->setDefaultValue(json_string_value(obj));
                        if (!isDefaultValueSet) {
                            LOG(LOG_WARN, "warning=1 Incorrect configured default value for signal '%s'\n", key);
                        }
                    }
                    if (!isDefaultValueSet && !sig->setDefaultValue("0")) {
                        sig->setDefaultValue(std::to_string(sig->getMinimum()));
                    }
                    sig->setVariableName(key);
                    m_variables.insert(key);
                }
            }
        }
    }
}

/*!
 * \brief Configuration::initMessageDirections
 * Initialize lists of message directions
 */
void Configuration::initMessageDirections()
{
    json_t *signals = json_object_get(m_canMapping, "signals");
    if (signals) {
        const char *key;
        json_t *value;
        json_object_foreach(signals, key, value) {
            if (json_t *id = json_object_get(value, "id")) {
                if (m_ignoreDirections) {
                    m_sendIDs.insert(json_integer_value(id));
                    m_receiveIDs.insert(json_integer_value(id));
                } else if (!json_object_get(value, "direction") || strcmp(json_string_value(json_object_get(value, "direction")), "out") == 0) {
                    m_sendIDs.insert(json_integer_value(id));
                } else if (strcmp(json_string_value(json_object_get(value, "direction")), "in") == 0) {
                    m_receiveIDs.insert(json_integer_value(id));
                } else {
                    LOG(LOG_WARN, "warning=1 Incorrect direction for variable '%s'\n", key);
                }
            }
        }
    }
}

/*!
 * \brief Configuration::getValue
 * Get the value for a variable name
 * \param key: Variable name
 * \return Current Value corresponding to a variable name
 */
Value Configuration::getValue(const std::string &key)
{
    if (const CANSignal *signal = getSignal(key)) {
        return signal->getValue();
    }
    return Value();
}

/*!
 * \brief Configuration::setValue
 * Set a value for a variable name
 * \param key: Variable name
 * \param value: Value for variable name
 * \return True if successful, false otherwise.
 */
bool Configuration::setValue(const std::string &key, Value value)
{
    if (CANMessage *message = getMessage(key)) {
        std::string signalName;
        if (getSignalName(key, signalName)) {
            std::uint32_t msgId;
            if (!getMessageId(key, msgId) || !m_sendIDs.count(msgId)) {
                LOG(LOG_WARN, "warning=1 Not setting variable '%s'. Variable not defined as outgoing\n", key.c_str());
            } else if (message->setValue(signalName, value)) {
                return true;
            }
        }
    }
    return false;
}

/*!
 * \brief Configuration::setValue
 * Set a value for a variable name from string
 * \param key: Variable name
 * \param value: Value as a string
 * \return True if successful, false otherwise.
 */
bool Configuration::setValue(const std::string &key, std::string value)
{
    if (CANMessage *message = getMessage(key)) {
        std::string signalName;
        if (getSignalName(key, signalName)) {
            std::uint32_t msgId;
            if (!getMessageId(key, msgId) || !m_sendIDs.count(msgId)) {
                LOG(LOG_WARN, "warning=1 Not setting variable '%s'. Variable not defined as outgoing\n", key.c_str());
            } else if (message->setValue(signalName, value)) {
                return true;
            }
        }
    }
    return false;
}

/*!
 * \brief Configuration::getDefaultValue
 * Get the default value for a variable name
 * \param key: Variable name
 * \return Default value corresponding to a variable name
 */
Value Configuration::getDefaultValue(const std::string &key)
{
    if (const CANSignal *signal = getSignal(key)) {
        return signal->getDefaultValue();
    }
    return Value();
}

/*!
 * \brief Configuration::setDefaultValues
 * Set all values to defaults
 */
void Configuration::setDefaultValues()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        it->second.resetValues(m_suppressDefaults);
    }
}

/*!
 * \brief Configuration::isVariableSupported
 * Check if variable name is supported by CAN configuration
 * \param key: Variable name
 * \return True if key supported, false otherwise
 */
bool Configuration::isVariableSupported(const std::string &key)
{
    json_t *signals = json_object_get(m_canMapping, "signals");
    return (signals && json_object_get(signals, key.c_str()));
}

/*!
 * \brief Configuration::getMessageId
 * Get message ID for variable
 * \param key: Variable name
 * \return CAN ID of the message
 */
bool Configuration::getMessageId(const std::string &key, std::uint32_t &msgId)
{
    if (isVariableSupported(key)) {
        json_t *signal = json_object_get(json_object_get(m_canMapping, "signals"), key.c_str());
        msgId = json_integer_value(json_object_get(signal, "id"));
        return true;
    } else {
        LOG(LOG_WARN, "warning=3 Message ID not found for variable '%s'\n", key.c_str());
        return false;
    }
}

/*!
 * \brief Configuration::getSendIDs
 * Get CAN IDs of the sendable CAN messages.
 * \return Reference to the set of sendable IDs
 */
const std::set<std::uint32_t> &Configuration::getSendIDs() const
{
    return m_sendIDs;
}

/*!
 * \brief Configuration::getReceiveIDs
 * Get CAN IDs of the receivable CAN messages.
 * \return Reference to the set of receivable IDs
 */
const std::set<std::uint32_t> &Configuration::getReceiveIDs() const
{
    return m_receiveIDs;
}

/*!
 * \brief Configuration::getVariables
 * Get all configured variables.
 * \return Reference to the set of configured variables
 */
const std::set<std::string> &Configuration::getVariables() const
{
    return m_variables;
}

/*!
 * \brief Configuration::getSignal
 * Get CANSignal by variable name
 * \param key: Variable name
 * \return Pointer to CANSignal object
 */
CANSignal *Configuration::getSignal(const std::string &key)
{
    std::string signalName;

    CANMessage *msg = getMessage(key);
    if (msg) {
        if (getSignalName(key, signalName)) {
            return msg->getSignalPrivate(signalName);
        }
    }
    return NULL;
}

/*!
 * \brief Configuration::getMessage
 * Get CANMessage by CAN message id
 * \param msgId: CAN message ID
 * \return Pointer to CANMessage object
 */
CANMessage *Configuration::getMessage(const std::uint32_t &msgId)
{
    std::map<std::uint32_t, CANMessage>::iterator msg = m_messages.find(msgId);
    if (msg != m_messages.end()) {
        return &msg->second;
    } else {
        LOG(LOG_WARN, "warning=3 Message %u (%#x) not found in dbc file\n", msgId, msgId);
        return NULL;
    }
}

/*!
 * \brief Configuration::getMessage
 * Get CANMessage by variable name
 * \param key: Variable name
 * \return Pointer to CANMessage object
 */
CANMessage *Configuration::getMessage(const std::string &key)
{
    std::uint32_t msgId;
    if (getMessageId(key, msgId)) {
        return getMessage(msgId);
    }
    return NULL;
}

/*!
 * \brief Configuration::getSignalName
 * Get signal name for variable
 * \param key: Variable name
 * \param signalName: Reference to name of the signal
 * \return True if successful, false otherwise.
 */
bool Configuration::getSignalName(const std::string &key, std::string &signalName)
{
    if (isVariableSupported(key)) {
        json_t *signal = json_object_get(json_object_get(m_canMapping, "signals"), key.c_str());
        signalName = json_string_value(json_object_get(signal, "signal"));
        return true;
    } else {
        LOG(LOG_WARN, "warning=3 Signal name not found for variable '%s'\n", key.c_str());
        return false;
    }
}

/*!
 * \brief Configuration::getAttribute
 * Get the attribute corresponding to a name
 * \param name: Attribute name
 * \return Pointer to Attribute corresponding to a name, NULL if not found
 */
const Attribute *Configuration::getAttribute(const std::string &name) const
{
    std::map<std::string, Attribute>::const_iterator attr = m_attributes.find(name);
    if (attr != m_attributes.end()) {
        return &attr->second;
    } else {
        return NULL;
    }
}

/*!
 * \brief Configuration::setAttribute
 * Set attribute
 * \param attr: Attribute
 */
void Configuration::setAttribute(const Attribute &attr)
{
    m_attributes.insert({attr.getName(), attr});
}

/*!
 * \brief Configuration::initCANMessageDirections
 * Initialize each CANMessage direction information, to be used with metrics
 */
void Configuration::initCANMessageDirections()
{
    for (std::set<std::uint32_t>::iterator it = m_receiveIDs.begin(); it != m_receiveIDs.end(); ++it) {
        CANMessage *msg = NULL;
        if ((msg = getMessage(*it)) != NULL) {
            msg->setDirection(MessageDirection::RECEIVE);
        }
    }
}

/*!
 * \brief Configuration::createFilterList
 * Add all messages to filter list
 * \param list: filter list to be updated
 * \return true if successfully updated filtering list, false if not
 */
bool Configuration::createFilterList(std::map<std::uint32_t, bool> &list)
{
    std::map<std::uint32_t, CANMessage>::iterator messages;
    for (messages = m_messages.begin(); messages != m_messages.end(); ++messages) {
        list.insert(std::pair<std::uint32_t, bool>(messages->first, false));
    }
    return (list.size() > 0);
}
