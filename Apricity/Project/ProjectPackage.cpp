#include "ProjectPackage.h"

#include <array>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <map>
#include <system_error>

#include <wx/datetime.h>

namespace
{
	constexpr std::array<char, 8> PackageMagic = {'A', 'P', 'R', 'I', 'C', 'I', 'T', 'Y'};
	constexpr std::uint32_t PackageVersion = 1;

	void WriteText(std::ofstream &stream, const wxString &value)
	{
		const std::string text = value.ToUTF8().data();
		const std::uint32_t size = static_cast<std::uint32_t>(text.size());

		stream.write(reinterpret_cast<const char *>(&size), sizeof(size));

		if (size > 0)
		{
			stream.write(text.data(), size);
		}
	}

	bool ReadText(std::ifstream &stream, wxString *value)
	{
		std::uint32_t size = 0;
		stream.read(reinterpret_cast<char *>(&size), sizeof(size));

		if (not stream)
		{
			return false;
		}

		std::string text(size, '\0');

		if (size > 0)
		{
			stream.read(text.data(), size);
		}

		if (not stream)
		{
			return false;
		}

		*value = wxString::FromUTF8(text.c_str());

		return true;
	}

	wxString PathToPortableText(const std::filesystem::path &path)
	{
		const auto text = path.generic_u8string();

		return wxString::FromUTF8(reinterpret_cast<const char *>(text.c_str()));
	}

	std::filesystem::path PortableTextToPath(const wxString &text)
	{
		return std::filesystem::path(std::string(text.ToUTF8().data()));
	}
} // namespace

ProjectPackage::ProjectPackage()
	: m_IsOpen(false)
{
}

bool ProjectPackage::Create(const std::filesystem::path &requestedPackageFile, const wxString &requestedTitle, wxString *errorMessage)
{
	const std::filesystem::path rootFolder = requestedPackageFile.parent_path();
	std::error_code errorCode;

	std::filesystem::create_directories(rootFolder, errorCode);

	if (errorCode)
	{
		*errorMessage = "Could not create project folder: " + wxString::FromUTF8(errorCode.message().c_str());

		return false;
	}

	m_PackageFile = requestedPackageFile;
	m_Title = requestedTitle;
	m_ProjectKey = MakeProjectKey();
	m_DatabaseFile = rootFolder / (requestedPackageFile.stem().wstring() + L".sqlite");
	m_AssetsFolder = rootFolder / "Assets";
	m_DraftsFolder = rootFolder / "Drafts";
	m_ExportsFolder = rootFolder / "Exports";

	std::filesystem::create_directories(m_AssetsFolder, errorCode);

	if (not errorCode)
	{
		std::filesystem::create_directories(m_DraftsFolder, errorCode);
	}

	if (not errorCode)
	{
		std::filesystem::create_directories(m_ExportsFolder, errorCode);
	}

	if (errorCode)
	{
		*errorMessage = "Could not create project folders: " + wxString::FromUTF8(errorCode.message().c_str());

		return false;
	}

	if (not WriteManifest(errorMessage))
	{
		return false;
	}

	m_IsOpen = true;

	return true;
}

bool ProjectPackage::Open(const std::filesystem::path &requestedPackageFile, wxString *errorMessage)
{
	m_PackageFile = requestedPackageFile;

	if (not ReadManifest(errorMessage))
	{
		m_IsOpen = false;

		return false;
	}

	m_IsOpen = true;

	return true;
}

bool ProjectPackage::IsOpen() const
{
	return m_IsOpen;
}

wxString ProjectPackage::GetTitle() const
{
	return m_Title;
}

wxString ProjectPackage::GetProjectKey() const
{
	return m_ProjectKey;
}

std::filesystem::path ProjectPackage::GetPackageFile() const
{
	return m_PackageFile;
}

std::filesystem::path ProjectPackage::GetRootFolder() const
{
	return m_PackageFile.parent_path();
}

std::filesystem::path ProjectPackage::GetDatabaseFile() const
{
	return m_DatabaseFile;
}

std::filesystem::path ProjectPackage::GetAssetsFolder() const
{
	return m_AssetsFolder;
}

std::filesystem::path ProjectPackage::GetDraftsFolder() const
{
	return m_DraftsFolder;
}

std::filesystem::path ProjectPackage::GetExportsFolder() const
{
	return m_ExportsFolder;
}

std::filesystem::path ProjectPackage::ResolveStoredPath(const wxString &storedPath) const
{
	const std::filesystem::path path = PortableTextToPath(storedPath);

	if (path.is_absolute())
	{
		return path;
	}

	return GetRootFolder() / path;
}

wxString ProjectPackage::MakeInternalDraftPath(const wxString &draftTitle) const
{
	wxString portableName = MakePortableName(draftTitle);

	if (portableName.empty())
	{
		portableName = "draft";
	}

	return "Drafts/" + portableName + ".md";
}

wxString ProjectPackage::MakePortableName(const wxString &text)
{
	wxString result;

	for (const wxUniChar character : text)
	{
		const wchar_t value = static_cast<wchar_t>(character.GetValue());

		if (std::iswalnum(value) != 0)
		{
			result += static_cast<wchar_t>(std::towlower(value));
		}
		else if (character == ' ' or character == '-' or character == '_')
		{
			if (result.empty() or result.Last() != '_')
			{
				result += '_';
			}
		}
	}

	if (result.EndsWith("_"))
	{
		result.RemoveLast();
	}

	return result;
}

wxString ProjectPackage::MakeProjectKey()
{
	return wxDateTime::UNow().Format("%Y%m%d%H%M%S%l");
}

bool ProjectPackage::WriteManifest(wxString *errorMessage) const
{
	std::ofstream stream(m_PackageFile, std::ios::binary);

	if (not stream)
	{
		*errorMessage = "Could not write the .apy manifest.";

		return false;
	}

	stream.write(PackageMagic.data(), PackageMagic.size());
	stream.write(reinterpret_cast<const char *>(&PackageVersion), sizeof(PackageVersion));

	const std::uint32_t fieldCount = 6;
	stream.write(reinterpret_cast<const char *>(&fieldCount), sizeof(fieldCount));

	const std::filesystem::path rootFolder = GetRootFolder();

	WriteText(stream, "project_key");
	WriteText(stream, m_ProjectKey);
	WriteText(stream, "title");
	WriteText(stream, m_Title);
	WriteText(stream, "database");
	WriteText(stream, PathToPortableText(std::filesystem::relative(m_DatabaseFile, rootFolder)));
	WriteText(stream, "assets");
	WriteText(stream, PathToPortableText(std::filesystem::relative(m_AssetsFolder, rootFolder)));
	WriteText(stream, "drafts");
	WriteText(stream, PathToPortableText(std::filesystem::relative(m_DraftsFolder, rootFolder)));
	WriteText(stream, "exports");
	WriteText(stream, PathToPortableText(std::filesystem::relative(m_ExportsFolder, rootFolder)));

	if (not stream)
	{
		*errorMessage = "Could not finish writing the .apy manifest.";

		return false;
	}

	return true;
}

bool ProjectPackage::ReadManifest(wxString *errorMessage)
{
	std::ifstream stream(m_PackageFile, std::ios::binary);

	if (not stream)
	{
		*errorMessage = "Could not open the .apy manifest.";

		return false;
	}

	std::array<char, 8> magic = {};
	std::uint32_t version = 0;
	std::uint32_t fieldCount = 0;

	stream.read(magic.data(), magic.size());
	stream.read(reinterpret_cast<char *>(&version), sizeof(version));
	stream.read(reinterpret_cast<char *>(&fieldCount), sizeof(fieldCount));

	if (magic != PackageMagic or version != PackageVersion)
	{
		*errorMessage = "This file is not a supported Apricity project.";

		return false;
	}

	std::map<wxString, wxString> fields;

	for (std::uint32_t index = 0; index < fieldCount; ++index)
	{
		wxString name;
		wxString value;

		if (not ReadText(stream, &name) or not ReadText(stream, &value))
		{
			*errorMessage = "The .apy manifest is incomplete or damaged.";

			return false;
		}

		fields[name] = value;
	}

	const std::filesystem::path rootFolder = GetRootFolder();

	m_ProjectKey = fields["project_key"];
	m_Title = fields["title"];
	m_DatabaseFile = rootFolder / PortableTextToPath(fields["database"]);
	m_AssetsFolder = rootFolder / PortableTextToPath(fields["assets"]);
	m_DraftsFolder = rootFolder / PortableTextToPath(fields["drafts"]);
	m_ExportsFolder = rootFolder / PortableTextToPath(fields["exports"]);

	return true;
}
