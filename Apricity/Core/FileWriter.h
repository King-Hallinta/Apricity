#pragma once

#include <filesystem>
#include <string>

#include <wx/string.h>

bool WriteTextFile(const std::filesystem::path &path, const std::string &text, wxString *errorMessage);
