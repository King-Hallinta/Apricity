#include "TextEncoding.h"

std::string ToUtf8(const wxString &text)
{
	return std::string(text.ToUTF8().data());
}

wxString FromUtf8(const std::string &text)
{
	return wxString::FromUTF8(text.c_str());
}

wxString FromUtf8(const unsigned char *text)
{
	if (text == nullptr)
	{
		return "";
	}

	return wxString::FromUTF8(reinterpret_cast<const char *>(text));
}
