#pragma once

#include <filesystem>

#include <wx/string.h>

class ProjectPackage
{
public:
	ProjectPackage();

	bool Create(const std::filesystem::path &packageFile, const wxString &title, wxString *errorMessage);
	bool Open(const std::filesystem::path &packageFile, wxString *errorMessage);

	bool IsOpen() const;
	wxString GetTitle() const;
	wxString GetProjectKey() const;
	std::filesystem::path GetPackageFile() const;
	std::filesystem::path GetRootFolder() const;
	std::filesystem::path GetDatabaseFile() const;
	std::filesystem::path GetAssetsFolder() const;
	std::filesystem::path GetDraftsFolder() const;
	std::filesystem::path GetExportsFolder() const;
	std::filesystem::path ResolveStoredPath(const wxString &storedPath) const;
	wxString MakeInternalDraftPath(const wxString &title) const;

private:
	static wxString MakePortableName(const wxString &text);
	static wxString MakeProjectKey();

	bool WriteManifest(wxString *errorMessage) const;
	bool ReadManifest(wxString *errorMessage);

	bool m_IsOpen;
	wxString m_Title;
	wxString m_ProjectKey;
	std::filesystem::path m_PackageFile;
	std::filesystem::path m_DatabaseFile;
	std::filesystem::path m_AssetsFolder;
	std::filesystem::path m_DraftsFolder;
	std::filesystem::path m_ExportsFolder;
};
