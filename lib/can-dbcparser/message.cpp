#include "header/message.hpp"
#include "logger.h"

#include <istream>
#include <limits>
#include <algorithm>

std::istream& operator>>(std::istream& in, Message& msg) {
	//Parse the message ID
	in >> msg.id;

	//Parse the name of the Message
	std::string name;
	in >> name;
	msg.name = name.substr(0, name.length() - 1);

	//Parse the Messages length
	in >> msg.dlc;
	if (msg.dlc > 8) {
		LOG(LOG_WARN, "warning=1 Incorrect dlc %u in CAN message %u\n", msg.dlc, msg.id);
		in.setstate(std::ios_base::failbit);
		return in;
	}

	//Parse the sender;
	in >> msg.from;

	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	//As long as there is a Signal, parse the Signal
	while(in) {
		Signal sig;
		in >> sig;
		if (in) {
			msg.signalList.insert({sig.getName(), sig});
		}
	}

	in.clear();
	return in;
}


std::set<std::string> Message::getTo() const {
	std::set<std::string> collection;
	for (auto sig : signalList) {
		auto toList = sig.second.getTo();
		collection.insert(toList.begin(), toList.end());
	}
	return collection;
}

/*!
 * \brief Message::getAttribute
 * Get the attribute corresponding to a name
 * \param name: Attribute name
 * \return Pointer to Attribute corresponding to a name, NULL if not found
 */
Attribute *Message::getAttribute(const std::string &name) {
	std::map<std::string, Attribute>::iterator attr = attributeList.find(name);
	if (attr != attributeList.end()) {
		return &attr->second;
	} else {
		return NULL;
	}
}

/*!
 * \brief Message::setAttribute
 * Set attribute
 * \param attr: Attribute
 */
void Message::setAttribute(const Attribute &attr) {
	attributeList.insert({attr.getName(), attr});
}
