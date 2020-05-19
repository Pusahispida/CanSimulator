#ifndef DBCTREE_HPP_
#define DBCTREE_HPP_

#include "message.hpp"
#include <iostream>
#include <iosfwd>
#include <string>
#include <map>

/**
 * This is the Top class of the dbclib and the interface to the user.
 * It enables its user to iterate over the Messages of a DBC-File
 */

class DBCIterator {
private:
	typedef std::map<std::uint32_t, Message> messages_t;
	//This list contains all the messages which got parsed from the DBC-File
	messages_t messageList;
	typedef std::map<std::string, Attribute> attributes_t;
	//This list contains all the attribute definitions and their default values (and global values) which got parsed from the DBC-File
	attributes_t attributeList;

public:
	typedef messages_t::const_iterator const_iterator;
	typedef messages_t::iterator iterator;
	typedef attributes_t attributesMap;

	//Constructors taking either a File or a Stream of a DBC-File
	explicit DBCIterator(const char *filePath);
	explicit DBCIterator(std::istream& stream);

	/*
	 * Functionality to access the Messages parsed from the File
	 * either via the iterators provided by begin() and end() or by
	 * random access operator[]
	 */
	const_iterator begin() const { return messageList.begin(); }
	const_iterator end() const { return messageList.end(); }

	iterator operator[](std::uint32_t id) {
		return messageList.find(id);
	}

	/*
	 * Get message by id
	 */
	messages_t::iterator getMessage(std::uint32_t id) {
		return messageList.find(id);
	}

	/*
	 * Get attributes
	 */
	const attributesMap getAttributes() const {
		return attributeList;
	}

private:
	void init(std::istream& stream);
	void parseAttributeDefinition(std::istream& in);
	void parseAttributeDefaultValue(std::istream& in);
	void parseAttributeValue(std::istream& in);
	void parseDescriptions(std::istream& in);
	void parseValueDescriptions(std::istream& in);
};

#endif /* DBCTREE_HPP_ */
