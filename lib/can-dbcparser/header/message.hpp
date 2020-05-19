#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include "attribute.hpp"
#include "signal.hpp"
#include <string>
#include <iosfwd>
#include <cstdint>
#include <set>
#include <map>

/**
 * Class representing a Message in the DBC-File. It allows its user to query
 * Data and to iterate over the Signals contained in the Message
 */
class Message {
protected:
	typedef std::map<std::string, Signal> signals_t;
	//Name of the Message
	std::string name;
	//The CAN-ID assigned to this specific Message
	std::uint32_t id;
	//The length of this message in Bytes. Allowed values are between 0 and 8
	std::size_t dlc;
	//String containing the name of the Sender of this Message if one exists in the DB
	std::string from;
	//List containing all Signals which are present in this Message
	signals_t signalList;
	//The description of the message
	std::string description;
	//This list contains all the attributes
	std::map<std::string, Attribute> attributeList;

public:
	typedef signals_t::const_iterator const_iterator;
	typedef signals_t::iterator iterator;
	//Overload of operator>> to enable parsing of Messages from streams of DBC-Files
	friend std::istream& operator>>(std::istream& in, Message& msg);

	//Getter functions for all the possible Data one can request from a Message
	const std::string &getName() const { return name; }
	std::uint32_t getId() const { return id; }
	std::size_t getDlc() const { return dlc; }
	const std::string &getFrom() const { return from; }
	std::set<std::string> getTo() const;
	const std::string &getDescription() const { return description; }
	void setDescription(const std::string &desc) { description = desc; }
	Attribute *getAttribute(const std::string &name);
	void setAttribute(const Attribute &attr);
	const std::map<std::string, Attribute> &getAttributes() const {
		return attributeList;
	}

	/*
	 * Functionality to access the Signals contained in this Message
	 * either via the iterators provided by begin() and end() or by
	 * random access operator[]
	 */
	const_iterator begin() const { return signalList.begin(); }
	const_iterator end() const { return signalList.end(); }
	signals_t::iterator operator[](const std::string &elem) {
		return signalList.find(elem);
	}

};

#endif /* MESSAGE_HPP_ */
