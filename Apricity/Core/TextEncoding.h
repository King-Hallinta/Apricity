#pragma once

#include <string>

#include <wx/string.h>

std::string ToUtf8(const wxString &text);
wxString FromUtf8(const std::string &text);
wxString FromUtf8(const unsigned char *text);
