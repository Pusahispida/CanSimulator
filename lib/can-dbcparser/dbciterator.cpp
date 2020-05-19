#include "header/canconstants.hpp"
#include "header/dbciterator.hpp"
#include "header/signal.hpp"
#include "logger.h"
#include "stringtools.h"

#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>

DBCIterator::DBCIterator(const char *filePath) {
	std::ifstream file(filePath);
	if (file) {
		init(file);
	} else {
		throw std::invalid_argument("The File could not be opened");
	}
	file.close();
}

DBCIterator::DBCIterator(std::istream& stream) {
	init(stream);
}

void DBCIterator::parseAttributeDefaultValue(std::istream& in) {
	std::string name;
	std::string value;

	// Get attribute name
	in >> name;
	// Get attribute default value
	in >> value;

	// Check that the attribute exists
	attributes_t::iterator attr = attributeList.find(trim(name, "\""));
	if (attr == attributeList.end()) {
		in.setstate(std::ios_base::failbit);
		return;
	}
	Attribute &attribute = attr->second;
	// Set default value for the attribute
	attribute.setDefaultValue(trim(value, " ;\""));

	if (attribute.getType() == DBC_MESSAGE || attribute.getType() == DBC_SIGNAL) {
		for (auto msg = begin(); msg != end(); ++msg) {
			if (!messageList.count(msg->second.getId())) {
				in.setstate(std::ios_base::failbit);
				return;
			}
			Message *message = &messageList[msg->second.getId()];
			// Set description to correct place based on type
			if (attribute.getType() == DBC_MESSAGE) {
				// Handle message attribute
				message->setAttribute(attribute);
			} else if (attribute.getType() == DBC_SIGNAL) {
				// Handle signal attribute
				for (auto it = message->begin(); it != message->end(); ++it) {
					Message::iterator sig = (*message)[it->second.getName()];
					if (sig != message->end()) {
						// Set signal attribute
						sig->second.setAttribute(attribute);
					}
				}
			}
		}
	}

	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void DBCIterator::parseAttributeValue(std::istream& in) {
	std::uint32_t id;
	std::string name;
	std::string type;
	std::string signal;
	std::string value;

	// Get attribute name
	in >> name;
	name = trim(name, "\"");
	// Check that the attribute exists
	attributes_t::iterator attr = attributeList.find(name);
	if (attr == attributeList.end()) {
		in.setstate(std::ios_base::failbit);
		return;
	}
	// Get object type or value
	in >> type;
	// For signal description also get the signal ID
	if (type == DBC_SIGNAL) {
		in >> id;
		in >> signal;
		in >> value;
	} else if (type == DBC_MESSAGE) {
		in >> id;
		in >> value;
	} else if (type == DBC_NODE || type == DBC_ENVIRONMENT_VARIABLE) {
		// Not implemented
		// Fail unsupported attribute
		in.setstate(std::ios_base::failbit);
		return;
	} else {
		// Global attribute
		attr->second.setValue(trim(type, " ;\""));
		return;
	}
	value = trim(value, " ;\"");
	// Handle enum attributes
	if (attr->second.getValueType() == DBC_ATTRIBUTE_TYPE_ENUM) {
		try {
			if (attr->second.getEnumValues().size() <= (unsigned int)std::stoi(value)) {
				in.setstate(std::ios_base::failbit);
				return;
			}
			value = attr->second.getEnumValues()[std::stoi(value)];
		}
		catch (const std::exception &) {
			in.setstate(std::ios_base::failbit);
			return;
		}
	}

	Message *message;
	if (type == DBC_MESSAGE || type == DBC_SIGNAL) {
		// Check that the message exists
		messages_t::iterator msg = getMessage(id);
		if (msg == end()) {
			in.setstate(std::ios_base::failbit);
			return;
		}
		message = &msg->second;
	}

	// Set description to correct place based on type
	if (type == DBC_MESSAGE) {
		// Handle message attribute
		if (message->getAttribute(name)) {
			message->getAttribute(name)->setValue(value);
		}
	} else if (type == DBC_SIGNAL) {
		// Handle signal attribute
		Message::iterator sig = (*message)[signal];
		if (sig != message->end()) {
			// Set signal attribute
			if (sig->second.getAttribute(name)) {
				sig->second.getAttribute(name)->setValue(value);
			}
		} else {
			in.setstate(std::ios_base::failbit);
		}
	} else {
		// Fail unsupported attribute
		in.setstate(std::ios_base::failbit);
	}
}

void DBCIterator::parseDescriptions(std::istream& in) {
	std::uint32_t id;
	std::string type;
	std::string signal;

	// Get description type (signal or message) and message ID
	in >> type;
	in >> id;
	// For signal description also get the signal ID
	if (type == DBC_SIGNAL) {
		in >> signal;
	}

	// Get whole description ending with ';'
	std::string line;
	std::getline(in, line, ';');
	// Remove leading and trailing spaces and '"'
	trim(line, " \"");

	// Check that the message exists
	messages_t::iterator msg = getMessage(id);
	if (msg == end()) {
		in.setstate(std::ios_base::failbit);
		return;
	}
	Message &message = msg->second;

	// Set description to correct place based on type
	if (type == DBC_MESSAGE) {
		// Handle message description
		message.setDescription(line);
	} else if (type == DBC_SIGNAL) {
		// Handle signal description
		// Check that signal exists
		Message::iterator sig = message[signal];
		if (sig != message.end()) {
			// Set signal description
			sig->second.setDescription(line);
		} else {
			in.setstate(std::ios_base::failbit);
		}
	} else {
		// Fail unsupported descriptions
		in.setstate(std::ios_base::failbit);
	}
}

void DBCIterator::parseValueDescriptions(std::istream& in) {
	std::uint32_t id;
	std::string signal;
	std::uint32_t value;
	std::string desc;

	// Get message and signal IDs
	in >> id;
	in >> signal;

	// Get whole description ending with ';'
	std::string line;
	std::getline(in, line, ';');

	// Check that the message exists
	messages_t::iterator msg = getMessage(id);
	if (msg == end()) {
		in.setstate(std::ios_base::failbit);
		return;
	}

	// Get signal and check that it exists
	Message &message = msg->second;
	Message::iterator sig = message[signal];
	if (sig == message.end()) {
		in.setstate(std::ios_base::failbit);
		return;
	}

	std::istringstream sstream(line);

	// Add value descriptions to signal
	while (sstream >> value) {
		// Ignore spaces and '"'
		std::string discard;
		std::getline(sstream, discard, '"');
		// Get description until next '"'
		std::getline(sstream, desc, '"');
		// Set value description to signal
		sig->second.setValueDescription(value, desc);
	}
}

void DBCIterator::init(std::istream& stream) {
	messageList.clear();
	std::string preamble;
	// Skip everything before first message.
	do {
		stream >> preamble;
		// Look for the first message
		if (preamble == DBC_MESSAGE) {
			// End skipping when first signal is found
			break;
		} else {
			stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	} while (!stream.eof());

	// Parse messages and signals
	while (!stream.eof() && (preamble == DBC_MESSAGE || preamble == "")) {
		// Get message
		Message msg;
		stream >> msg;
		// Check that message parsing was successful
		if (!stream.fail()) {
			// Add message to list
			messageList.insert({msg.getId(), msg});
		} else {
			// On failure clear the current line from stream
			stream.clear();
			stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		// Get preamble of next line
		stream >> preamble;
	}

	// Parse descriptions
	while (!stream.eof()) {
		// Handle supported features
		if (preamble == DBC_DESCRIPTION) {
			// Message and signal descriptions
			parseDescriptions(stream);
		} else if (preamble == DBC_VALUE_DESCRIPTION) {
			// Signal value descriptions
			parseValueDescriptions(stream);
		} else if (preamble == DBC_ATTRIBUTE_DEFINITION) {
			// Get attribute
			Attribute attr;
			stream >> attr;
			// Check that attribute parsing was successful
			if (!stream.fail()) {
				// Add attribute to list
				attributeList.insert({attr.getName(), attr});
			}
		} else if (preamble == DBC_ATTRIBUTE_VALUE_DEFAULT) {
			// Attribute default value
			parseAttributeDefaultValue(stream);
		} else if (preamble == DBC_ATTRIBUTE_VALUE) {
			// Attribute value
			parseAttributeValue(stream);
		} else {
			// If not implemented fail the stream
			stream.setstate(std::ios_base::failbit);
		}
		// On failure clear the current line from stream
		if (stream.fail()) {
			stream.clear();
			stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		// Get preamble of next line
		stream >> preamble;
	}
}
