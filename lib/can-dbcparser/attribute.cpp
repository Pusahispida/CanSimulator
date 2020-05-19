/*!
* \file
* \brief attribute.cpp foo
*/

#include "header/attribute.hpp"
#include "header/canconstants.hpp"
#include "logger.h"

#include <algorithm>
#include <istream>
#include <limits>
#include <stdexcept>
#include "stringtools.h"

/*!
 * \brief Attribute::Attribute
 * Constructor
 */
Attribute::Attribute(const Attribute &attr) :
	name(attr.getName()),
	type(attr.getType()),
	valueType(attr.getValueType()),
	defaultValue(attr.getDefaultValue()),
	value(attr.getValue()),
	minValue(attr.getMinValue()),
	maxValue(attr.getMaxValue()),
	enumValues(attr.getEnumValues()) {
}

std::istream& operator>>(std::istream& in, Attribute& attr) {
	std::string line;
	//Object type
	in.ignore(1);
	getline(in, attr.type, ' ');
	if (!attr.type.empty()) {
		in.ignore(1);
	}

	//Parse the name of the Attribute
	getline(in, attr.name, ' ');
	if (attr.name.empty()) {
		in.setstate(std::ios_base::failbit);
		return in;
	}
	attr.name = trim(attr.name, "\"");

	//Parse the value type
	getline(in, attr.valueType, ' ');
	if (attr.valueType.empty()) {
		in.setstate(std::ios_base::failbit);
		return in;
	}
	if (attr.valueType == DBC_ATTRIBUTE_TYPE_INTEGER || attr.valueType == DBC_ATTRIBUTE_TYPE_HEX || attr.valueType == DBC_ATTRIBUTE_TYPE_FLOAT) {
		getline(in, attr.minValue, ' ');
		if (attr.minValue.empty()) {
			in.setstate(std::ios_base::failbit);
			return in;
		}
		getline(in, attr.maxValue, ';');
		if (attr.maxValue.empty()) {
			in.setstate(std::ios_base::failbit);
			return in;
		}
	} else if (attr.valueType == DBC_ATTRIBUTE_TYPE_ENUM) {
		std::getline(in, line);
		attr.enumValues = split(line, ',');
		for (auto it = attr.enumValues.begin(); it != attr.enumValues.end(); ++it) {
			*it = trim(*it, " ;\"\r");
		}
	}

	in.clear();
	return in;
}

/*!
 * \brief Attribute::setDefaultValue
 * Set default value for attribute.
 * \param val: Value as a string
 */
void Attribute::setDefaultValue(const std::string &val) {
	if (checkValue(val)) {
		defaultValue = val;
	}
}

/*!
 * \brief Attribute::setValue
 * Set value for attribute.
 * \param val: Value as a string
 */
void Attribute::setValue(const std::string &val) {
	if (checkValue(val)) {
		value = val;
	}
}

/*!
 * \brief Attribute::checkValue
 * Check if value is valid.
 * \param val: Value as a string
 * \return True if value is valid, false otherwise
 */
bool Attribute::checkValue(const std::string &val) {
	try {
		if (valueType == DBC_ATTRIBUTE_TYPE_INTEGER || valueType == DBC_ATTRIBUTE_TYPE_HEX) {
			int value = std::stoi(val);
			if (value >= std::stoi(minValue) && value <= std::stoi(maxValue)) {
				return true;
			}
		} else if (valueType == DBC_ATTRIBUTE_TYPE_FLOAT) {
			double value = std::stod(val);
			if (value >= std::stod(minValue) && value <= std::stod(maxValue)) {
				return true;
			}
		} else if (valueType == DBC_ATTRIBUTE_TYPE_ENUM) {
			if (std::find(enumValues.begin(), enumValues.end(), val) != enumValues.end()) {
				return true;
			}
		} else {
			return true;
		}
	}
	catch (const std::exception &) {
		return false;
	}
	return false;
}
