#include "header/signal.hpp"
#include "header/canconstants.hpp"
#include "logger.h"

#include <istream>
#include <sstream>
#include <limits>
#include <iterator>
#include <algorithm>
#include "stringtools.h"

std::istream& operator>>(std::istream& in, Signal& sig) {
	std::string line;
	std::getline(in, line);
	if (!line.empty() && *line.rbegin() == '\r') line.erase(line.length() - 1, 1);
	if (line.empty()) {
		in.setstate(std::ios_base::failbit);
		return in;
	}

	std::istringstream sstream(line);
	std::string preamble;
	sstream >> preamble;
	//Check if we are actually reading a Signal otherwise fail the stream
	if (preamble != DBC_SIGNAL) {
		sstream.setstate(std::ios_base::failbit);
		return in;
	}

	//Parse the Signal Name
	sstream >> sig.name;

	std::string multi;
	sstream >> multi;

	//This case happens if there is no Multiplexor present
	if (multi == ":") {
		sig.multiplexor = Multiplexor::NONE;
	//Case with multiplexor
	} else {
		if (multi == DBC_MULTIPLEXOR) {
			sig.multiplexor = Multiplexor::MULTIPLEXOR;
		} else {
			//The multiplexor looks like that 'm12' so we ignore the m and parse it as integer
			std::istringstream multstream(multi);
			multstream.ignore(1);
			unsigned short multiNum;
			multstream >> multiNum;
			sig.multiplexor = Multiplexor::MULTIPLEXED;
			sig.multiplexNum = multiNum;
		}
		//ignore the next thing which is a ':'
		sstream >> multi;
	}

	sstream >> sig.startBit;
	sstream.ignore(1);
	sstream >> sig.length;
	if (sig.length > 64) {
		LOG(LOG_WARN, "warning=1 Incorrect length %u in CAN signal %s\n", sig.length, sig.name.c_str());
		sstream.setstate(std::ios_base::failbit);
		return in;
	}
	sstream.ignore(1);

	int order;
	sstream >> order;
	if (order == 0) {
		sig.order = ByteOrder::MOTOROLA;
	} else {
		sig.order = ByteOrder::INTEL;
	}

	char sign;
	sstream >> sign;
	if (sign == '+') {
		sig.sign = Sign::UNSIGNED;
	} else {
		sig.sign = Sign::SIGNED;
	}

	sstream.ignore(std::numeric_limits<std::streamsize>::max(), '(');
	sstream >> sig.factor;
	sstream.ignore(1);
	sstream >> sig.offset;
	sstream.ignore(1);

	sstream.ignore(std::numeric_limits<std::streamsize>::max(), '[');
	sstream >> sig.minimum;
	sstream.ignore(1);
	sstream >> sig.maximum;
	sstream.ignore(1);
	// Check if minimum, maximum, length and factor are valid
	// - maximum is bigger than minimum
	// - if signal value range is fits to the maximum value calculated
	//   from number of bits and scaling factor of the signal
	if ((sig.maximum <= sig.minimum) ||
			((sig.maximum - sig.minimum) > ((0xFFFFFFFFFFFFFFFF >> (64 - sig.length)) * sig.factor))) {
		LOG(LOG_WARN, "warning=1 Incorrect minimum %f or maximum %f or length %u or factor %f value in CAN signal %s\n",
			sig.minimum, sig.maximum, sig.length, sig.factor, sig.name.c_str());
	}

	std::string unit;
	sstream >> unit;
	sig.unit = trim(unit, "\"");

	std::string to;
	sstream >> to;
	std::vector<std::string> toStrings = split(to, ',');
	std::move(toStrings.begin(), toStrings.end(), std::inserter(sig.to, sig.to.begin()));

	return in;
}

/*!
 * \brief Signal::getAttribute
 * Get the attribute corresponding to a name
 * \param name: Attribute name
 * \return Pointer to Attribute corresponding to a name, NULL if not found
 */
Attribute *Signal::getAttribute(const std::string &name) {
	std::map<std::string, Attribute>::iterator attr = attributeList.find(name);
	if (attr != attributeList.end()) {
		return &attr->second;
	} else {
		return NULL;
	}
}

/*!
 * \brief Signal::setAttribute
 * Set attribute
 * \param attr: Attribute
 */
void Signal::setAttribute(const Attribute &attr) {
	attributeList.insert({attr.getName(), attr});
}
