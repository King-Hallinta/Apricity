#pragma once

#include "StoryRepository.h"

#include <string>
#include <vector>

struct StoryExportDefinition
{
	StoryExport target;
	std::string fileName;
	std::string heading;
	std::string tableName;
	std::vector<std::string> columns;
};

const std::vector<StoryExportDefinition> &GetStoryExportDefinitions();
const StoryExportDefinition *FindStoryExportDefinition(StoryExport target);
std::string BuildStoryExportText(const StoryExportDefinition &exportDefinition, const std::vector<StoryRecord> &records);
