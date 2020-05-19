#ifndef ATTRIBUTE_HPP_
#define ATTRIBUTE_HPP_

#include <iosfwd>
#include <string>
#include <vector>

/**
 * Class representing an attribute in the DBC-File.
 */
class Attribute {
protected:
	//Name of the attribute
	std::string name;
	//The type of the object
	std::string type;
	//The type of the value
	std::string valueType;
	//The default value
	std::string defaultValue;
	//The value
	std::string value;
	//The minimum value
	std::string minValue;
	//The maximum value
	std::string maxValue;
	//The enum values
	std::vector<std::string> enumValues;

public:
	Attribute() {}
	Attribute(const Attribute &attr);
	//Overload of operator>> to enable parsing of Attribute definition from streams of DBC-Files
	friend std::istream& operator>>(std::istream& in, Attribute& attr);

	//Getter functions for all the possible Data one can request from an attribute
	const std::string &getName() const { return name; }
	const std::string &getType() const { return type; }
	const std::string &getValueType() const { return valueType; }
	const std::string &getDefaultValue() const { return defaultValue; }
	const std::string &getValue() const { return value.empty() ? defaultValue : value; }
	const std::string &getMinValue() const { return minValue; }
	const std::string &getMaxValue() const { return maxValue; }
	const std::vector<std::string> &getEnumValues() const { return enumValues; }
	void setDefaultValue(const std::string &val);
	void setValue(const std::string &val);
	int toInt() const { return std::stoi(getValue()); }
	float toFloat() const { return std::stof(getValue()); }

private:
	bool checkValue(const std::string &val);
};

#endif /* ATTRIBUTE_HPP_ */
