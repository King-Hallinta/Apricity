#include "StoryRepository.h"

#include "StoryExportDefinitions.h"
#include "../Core/FileWriter.h"
#include "../Core/StringList.h"
#include "../Core/TextEncoding.h"
#include "../Project/ProjectPackage.h"

#include <cctype>
#include <set>
#include <sstream>

#include <sqlite3.h>

namespace
{
	const std::set<std::string> KnownTables =
		{
			"characters",
			"continuity_facts",
			"entities",
			"locations",
			"relationships",
			"scenes",
			"timeline_events"};
} // namespace

StoryRepository::StoryRepository()
	: m_Database(nullptr)
{
}

StoryRepository::~StoryRepository()
{
	Close();
}

bool StoryRepository::Open(const std::filesystem::path &databaseFile, wxString *errorMessage)
{
	Close();

	sqlite3 *handle = nullptr;
	const std::wstring path = databaseFile.wstring();

	if (sqlite3_open16(path.c_str(), &handle) != SQLITE_OK)
	{
		*errorMessage = "Could not open the project database.";
		m_Database = handle;
		Close();

		return false;
	}

	m_Database = handle;

	return true;
}

void StoryRepository::Close()
{
	if (m_Database != nullptr)
	{
		sqlite3_close(reinterpret_cast<sqlite3 *>(m_Database));
		m_Database = nullptr;
	}
}

bool StoryRepository::Initialize(const wxString &title, const wxString &projectKey, wxString *errorMessage)
{
	const char *schemaSql = R"sql(
		PRAGMA foreign_keys = ON;

		CREATE TABLE IF NOT EXISTS project
		(
			project_key TEXT PRIMARY KEY,
			title TEXT NOT NULL
		);

		CREATE TABLE IF NOT EXISTS entities
		(
			entity_name TEXT NOT NULL,
			entity_type TEXT,
			description TEXT,
			aliases TEXT,
			rules TEXT,
			notes TEXT
		);

		CREATE TABLE IF NOT EXISTS characters
		(
			display_name TEXT NOT NULL,
			aliases TEXT,
			traits TEXT,
			goals TEXT,
			secrets TEXT,
			notes TEXT,
			first_appearance TEXT,
			last_appearance TEXT
		);

		CREATE TABLE IF NOT EXISTS locations
		(
			location_name TEXT NOT NULL,
			description TEXT,
			linked_characters TEXT,
			important_events TEXT,
			mood_notes TEXT
		);

		CREATE TABLE IF NOT EXISTS scenes
		(
			scene_title TEXT NOT NULL,
			summary TEXT,
			pov_character TEXT,
			location_name TEXT,
			story_time TEXT,
			status TEXT,
			tags TEXT,
			draft_path TEXT
		);

		CREATE TABLE IF NOT EXISTS relationships
		(
			source_entity TEXT NOT NULL,
			target_entity TEXT NOT NULL,
			statement TEXT,
			relation_type TEXT,
			status TEXT,
			strength TEXT,
			notes TEXT
		);

		CREATE TABLE IF NOT EXISTS timeline_events
		(
			event_title TEXT NOT NULL,
			story_order TEXT,
			manuscript_order TEXT,
			story_time TEXT,
			summary TEXT,
			entities TEXT,
			location_name TEXT,
			notes TEXT
		);

		CREATE TABLE IF NOT EXISTS continuity_facts
		(
			fact_title TEXT NOT NULL,
			fact_text TEXT,
			scope_record TEXT,
			known_by TEXT,
			starts_in_scene TEXT,
			ends_in_scene TEXT,
			notes TEXT
		);

		CREATE TABLE IF NOT EXISTS tags
		(
			tag_name TEXT PRIMARY KEY
		);

		CREATE TABLE IF NOT EXISTS scene_entity_links
		(
			scene_row INTEGER NOT NULL,
			entity_row INTEGER NOT NULL
		);

		CREATE TABLE IF NOT EXISTS scene_tag_links
		(
			scene_row INTEGER NOT NULL,
			tag_name TEXT NOT NULL
		);

		CREATE TABLE IF NOT EXISTS location_entity_links
		(
			location_row INTEGER NOT NULL,
			entity_row INTEGER NOT NULL
		);
	)sql";

	if (not Execute(schemaSql, errorMessage))
	{
		return false;
	}

	sqlite3_stmt *statement = nullptr;
	const char *sql =
		"INSERT INTO project(project_key, title) VALUES(?, ?) "
		"ON CONFLICT(project_key) DO UPDATE SET title = excluded.title;";

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql, -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not prepare project metadata.";

		return false;
	}

	const std::string keyText = ToUtf8(projectKey);
	const std::string titleText = ToUtf8(title);

	sqlite3_bind_text(statement, 1, keyText.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(statement, 2, titleText.c_str(), -1, SQLITE_TRANSIENT);

	const bool succeeded = sqlite3_step(statement) == SQLITE_DONE;
	sqlite3_finalize(statement);

	if (not succeeded)
	{
		*errorMessage = "Could not save project metadata.";

		return false;
	}

	return true;
}

bool StoryRepository::IsOpen() const
{
	return m_Database != nullptr;
}

std::vector<StoryRecord> StoryRepository::LoadRecords(
	const std::string &tableName,
	const std::vector<std::string> &columnNames,
	const std::vector<std::string> &searchColumns,
	const wxString &filterText,
	wxString *errorMessage)
{
	std::vector<StoryRecord> records;

	if (not IsKnownTable(tableName))
	{
		*errorMessage = "Unknown project table.";

		return records;
	}

	std::string sql = "SELECT rowid, " + JoinText(columnNames, ", ") + " FROM " + tableName;
	const std::string filter = ToUtf8(filterText);

	if (not filter.empty() and not searchColumns.empty())
	{
		sql += " WHERE ";

		for (std::size_t index = 0; index < searchColumns.size(); ++index)
		{
			if (index > 0)
			{
				sql += " OR ";
			}

			sql += searchColumns[index] + " LIKE ?";
		}
	}

	sql += " ORDER BY rowid DESC;";

	sqlite3_stmt *statement = nullptr;

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not load project records.";

		return records;
	}

	if (not filter.empty() and not searchColumns.empty())
	{
		const std::string pattern = "%" + filter + "%";

		for (std::int32_t index = 1; index <= static_cast<std::int32_t>(searchColumns.size()); ++index)
		{
			sqlite3_bind_text(statement, index, pattern.c_str(), -1, SQLITE_TRANSIENT);
		}
	}

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		StoryRecord record;
		record.rowId = sqlite3_column_int(statement, 0);

		for (std::int32_t index = 0; index < static_cast<std::int32_t>(columnNames.size()); ++index)
		{
			const wxString value = FromUtf8(sqlite3_column_text(statement, index + 1));
			record.fields[columnNames[index]] = ToUtf8(value);
		}

		records.push_back(record);
	}

	sqlite3_finalize(statement);

	return records;
}

bool StoryRepository::InsertRecord(
	const std::string &tableName,
	const std::map<std::string, std::string> &fields,
	wxString *errorMessage)
{
	if (not IsKnownTable(tableName))
	{
		*errorMessage = "Unknown project table.";

		return false;
	}

	std::vector<std::string> columnNames;
	std::ostringstream placeholders;

	for (const auto &field : fields)
	{
		if (not columnNames.empty())
		{
			placeholders << ", ";
		}

		columnNames.push_back(field.first);
		placeholders << "?";
	}

	const std::string sql = "INSERT INTO " + tableName + "(" + JoinText(columnNames, ", ") + ") VALUES(" + placeholders.str() + ");";

	sqlite3_stmt *statement = nullptr;

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not prepare the new record.";

		return false;
	}

	std::int32_t bindIndex = 1;

	for (const auto &field : fields)
	{
		sqlite3_bind_text(statement, bindIndex, field.second.c_str(), -1, SQLITE_TRANSIENT);
		++bindIndex;
	}

	const bool succeeded = sqlite3_step(statement) == SQLITE_DONE;
	sqlite3_finalize(statement);

	if (not succeeded)
	{
		*errorMessage = "Could not save the new record.";

		return false;
	}

	return true;
}

bool StoryRepository::UpdateRecord(
	const std::string &tableName,
	std::int32_t rowId,
	const std::map<std::string, std::string> &fields,
	wxString *errorMessage)
{
	if (not IsKnownTable(tableName))
	{
		*errorMessage = "Unknown project table.";

		return false;
	}

	std::ostringstream assignments;
	std::int32_t assignmentIndex = 0;

	for (const auto &field : fields)
	{
		if (assignmentIndex > 0)
		{
			assignments << ", ";
		}

		assignments << field.first << " = ?";
		++assignmentIndex;
	}

	const std::string sql = "UPDATE " + tableName + " SET " + assignments.str() + " WHERE rowid = ?;";

	sqlite3_stmt *statement = nullptr;

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not prepare the record update.";

		return false;
	}

	std::int32_t bindIndex = 1;

	for (const auto &field : fields)
	{
		sqlite3_bind_text(statement, bindIndex, field.second.c_str(), -1, SQLITE_TRANSIENT);
		++bindIndex;
	}

	sqlite3_bind_int(statement, bindIndex, rowId);

	const bool succeeded = sqlite3_step(statement) == SQLITE_DONE;
	sqlite3_finalize(statement);

	if (not succeeded)
	{
		*errorMessage = "Could not update the record.";

		return false;
	}

	return true;
}

bool StoryRepository::DeleteRecord(const std::string &tableName, std::int32_t rowId, wxString *errorMessage)
{
	if (not IsKnownTable(tableName))
	{
		*errorMessage = "Unknown project table.";

		return false;
	}

	const std::string sql = "DELETE FROM " + tableName + " WHERE rowid = ?;";
	sqlite3_stmt *statement = nullptr;

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not prepare the record deletion.";

		return false;
	}

	sqlite3_bind_int(statement, 1, rowId);

	const bool succeeded = sqlite3_step(statement) == SQLITE_DONE;
	sqlite3_finalize(statement);

	if (not succeeded)
	{
		*errorMessage = "Could not delete the record.";

		return false;
	}

	return true;
}

std::vector<wxString> StoryRepository::LoadDistinctColumnValues(
	const std::string &tableName,
	const std::string &columnName,
	wxString *errorMessage)
{
	std::vector<wxString> values;

	if (not IsKnownTable(tableName) or not IsSafeColumnName(columnName))
	{
		*errorMessage = "Unknown choice source.";

		return values;
	}

	const std::string sql = "SELECT DISTINCT " + columnName + " FROM " + tableName + " WHERE " + columnName + " IS NOT NULL AND " + columnName + " <> '' ORDER BY " + columnName + ";";

	sqlite3_stmt *statement = nullptr;

	if (sqlite3_prepare_v2(reinterpret_cast<sqlite3 *>(m_Database), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
	{
		*errorMessage = "Could not load choices.";

		return values;
	}

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		values.push_back(FromUtf8(sqlite3_column_text(statement, 0)));
	}

	sqlite3_finalize(statement);

	return values;
}

bool StoryRepository::WriteExports(const ProjectPackage &package, wxString *errorMessage)
{
	errorMessage->Clear();

	bool wroteAnyFile = false;

	for (const StoryExportDefinition &exportDefinition : GetStoryExportDefinitions())
	{
		const std::vector<StoryRecord> records = LoadRecords(
			exportDefinition.tableName,
			exportDefinition.columns,
			exportDefinition.columns,
			"",
			errorMessage);

		if (not errorMessage->empty())
		{
			return false;
		}

		if (records.empty())
		{
			continue;
		}

		const std::filesystem::path path = package.GetExportsFolder() / exportDefinition.fileName;

		if (not WriteTextFile(path, BuildStoryExportText(exportDefinition, records), errorMessage))
		{
			return false;
		}

		wroteAnyFile = true;
	}

	if (not wroteAnyFile)
	{
		*errorMessage = "There is no project data to export.";

		return false;
	}

	return true;
}

bool StoryRepository::WriteExport(const ProjectPackage &package, StoryExport exportTarget, wxString *errorMessage)
{
	errorMessage->Clear();

	const StoryExportDefinition *exportDefinition = FindStoryExportDefinition(exportTarget);

	if (exportDefinition == nullptr)
	{
		*errorMessage = "Unknown export target.";

		return false;
	}

	const std::vector<StoryRecord> records = LoadRecords(
		exportDefinition->tableName,
		exportDefinition->columns,
		exportDefinition->columns,
		"",
		errorMessage);

	if (not errorMessage->empty())
	{
		return false;
	}

	if (records.empty())
	{
		*errorMessage = "There are no " + wxString::FromUTF8(exportDefinition->heading.c_str()) + " records to export.";

		return false;
	}

	const std::filesystem::path path = package.GetExportsFolder() / exportDefinition->fileName;

	return WriteTextFile(path, BuildStoryExportText(*exportDefinition, records), errorMessage);
}

bool StoryRepository::Execute(const char *sql, wxString *errorMessage)
{
	char *sqliteError = nullptr;

	if (sqlite3_exec(reinterpret_cast<sqlite3 *>(m_Database), sql, nullptr, nullptr, &sqliteError) != SQLITE_OK)
	{
		if (sqliteError == nullptr)
		{
			*errorMessage = "Database command failed.";
		}
		else
		{
			*errorMessage = wxString::FromUTF8(sqliteError);
		}

		sqlite3_free(sqliteError);

		return false;
	}

	return true;
}

bool StoryRepository::IsKnownTable(const std::string &tableName) const
{
	return KnownTables.contains(tableName);
}

bool StoryRepository::IsSafeColumnName(const std::string &columnName) const
{
	if (columnName.empty())
	{
		return false;
	}

	for (const char character : columnName)
	{
		const unsigned char value = static_cast<unsigned char>(character);

		if (std::isalnum(value) == 0 and character != '_')
		{
			return false;
		}
	}

	return true;
}
