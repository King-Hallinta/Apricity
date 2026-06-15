#include "FileWriter.h"

#include <fstream>

bool WriteTextFile(const std::filesystem::path &path, const std::string &text, wxString *errorMessage)
{
	std::ofstream stream(path, std::ios::binary);

	if (not stream)
	{
		*errorMessage = "Could not write export file: " + wxString(path.wstring());

		return false;
	}

	stream << text;

	return true;
}
