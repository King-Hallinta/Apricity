#include "StringList.h"

#include <cstddef>
#include <sstream>

std::string JoinText(const std::vector<std::string> &values, const std::string &delimiter)
{
	std::ostringstream stream;

	for (std::size_t index = 0; index < values.size(); ++index)
	{
		if (index > 0)
		{
			stream << delimiter;
		}

		stream << values[index];
	}

	return stream.str();
}
