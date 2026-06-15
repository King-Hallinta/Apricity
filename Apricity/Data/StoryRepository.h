#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <wx/string.h>

struct StoryRecord
{
	std::int32_t rowId = 0;
	std::map<std::string, std::string> fields;
};

enum class StoryExport
{
	Characters,
	Locations,
	SceneOutline,
	Timeline,
	Relationships
};

class ProjectPackage;

class StoryRepository
{
public:
	StoryRepository();
	~StoryRepository();

	bool Open(const std::filesystem::path &databaseFile, wxString *errorMessage);
	void Close();
	bool Initialize(const wxString &title, const wxString &projectKey, wxString *errorMessage);
	bool IsOpen() const;

	std::vector<StoryRecord> LoadRecords(
		const std::string &tableName,
		const std::vector<std::string> &columnNames,
		const std::vector<std::string> &searchColumns,
		const wxString &filterText,
		wxString *errorMessage);

	bool InsertRecord(
		const std::string &tableName,
		const std::map<std::string, std::string> &fields,
		wxString *errorMessage);

	bool UpdateRecord(
		const std::string &tableName,
		std::int32_t rowId,
		const std::map<std::string, std::string> &fields,
		wxString *errorMessage);

	std::vector<wxString> LoadDistinctColumnValues(
		const std::string &tableName,
		const std::string &columnName,
		wxString *errorMessage);

	bool DeleteRecord(const std::string &tableName, std::int32_t rowId, wxString *errorMessage);
	bool WriteExport(const ProjectPackage &package, StoryExport exportTarget, wxString *errorMessage);
	bool WriteExports(const ProjectPackage &package, wxString *errorMessage);

private:
	bool Execute(const char *sql, wxString *errorMessage);
	bool IsKnownTable(const std::string &tableName) const;
	bool IsSafeColumnName(const std::string &columnName) const;

	void *m_Database;
};
