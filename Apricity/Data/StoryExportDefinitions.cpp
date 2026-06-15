#include "StoryExportDefinitions.h"

#include <sstream>

namespace
{
	const std::vector<StoryExportDefinition> StoryExportDefinitions =
		{
			{StoryExport::Characters, "characters.md", "Character Bible", "characters", {"display_name", "aliases", "traits", "goals", "secrets", "notes", "first_appearance", "last_appearance"}},
			{StoryExport::Locations, "locations.md", "Location List", "locations", {"location_name", "description", "linked_characters", "important_events", "mood_notes"}},
			{StoryExport::SceneOutline, "scene_outline.md", "Scene Outline", "scenes", {"scene_title", "summary", "pov_character", "location_name", "story_time", "status", "tags", "draft_path"}},
			{StoryExport::Timeline, "timeline.md", "Timeline", "timeline_events", {"event_title", "story_order", "manuscript_order", "story_time", "summary", "entities", "location_name", "notes"}},
			{StoryExport::Relationships, "relationships.md", "Relationship Summary", "relationships", {"source_entity", "target_entity", "statement", "relation_type", "status", "strength", "notes"}}};
}

const std::vector<StoryExportDefinition> &GetStoryExportDefinitions()
{
	return StoryExportDefinitions;
}

const StoryExportDefinition *FindStoryExportDefinition(StoryExport target)
{
	for (const StoryExportDefinition &exportDefinition : StoryExportDefinitions)
	{
		if (exportDefinition.target == target)
		{
			return &exportDefinition;
		}
	}

	return nullptr;
}

std::string BuildStoryExportText(const StoryExportDefinition &exportDefinition, const std::vector<StoryRecord> &records)
{
	std::ostringstream text;
	text << "# " << exportDefinition.heading << "\n\n";

	for (const StoryRecord &record : records)
	{
		text << "## Record " << record.rowId << "\n\n";

		for (const std::string &column : exportDefinition.columns)
		{
			const auto iterator = record.fields.find(column);
			const std::string value = iterator == record.fields.end() ? "" : iterator->second;

			text << "- " << column << ": " << value << "\n";
		}

		text << "\n";
	}

	return text.str();
}
