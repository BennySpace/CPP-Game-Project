#include "detail/StationMap.h"

#include "Core/GameIds.h"
#include "Data/DataLoader.h"
#include "detail/ConsoleDecor.h"

namespace
{
std::string localizeLocationId(const std::string &locationId)
{
    const auto &locations = DataLoader::getLocations();
    const auto it = locations.find(locationId);
    if (it != locations.end())
    {
        return it->second.name;
    }
    return locationId;
}

std::vector<std::string> buildLocalizedMapLines()
{
    return {"                            [" + localizeLocationId(GameIds::kDockLocation) + "]",
            "                                     |",
            "                                     |",
            "[" + localizeLocationId(GameIds::kMedbayLocation) + "] ---- [" +
                localizeLocationId(GameIds::kIntakeLocation) + "]",
            "                                     |",
            "                                     |",
            "                              [" + localizeLocationId(GameIds::kArchiveLocation) + "] ---- [" +
                localizeLocationId(GameIds::kMachineShopLocation) + "] ---- [" +
                localizeLocationId(GameIds::kReactorSpineLocation) + "]",
            "                                     |",
            "                                     |",
            "                               [" + localizeLocationId(GameIds::kSecurityLocation) + "] ---- [" +
                localizeLocationId(GameIds::kCommandBridgeLocation) + "] ---- [" +
                localizeLocationId(GameIds::kShuttleBayLocation) + "]"};
}
} // namespace

std::vector<std::string> buildHighlightedStationMapLines(const std::string &currentLocationName)
{
    // Rebuild from live location names each time so the map stays aligned with
    // the currently loaded data set instead of caching stale labels.
    std::vector<std::string> highlightedLines = buildLocalizedMapLines();
    if (currentLocationName.empty())
    {
        return highlightedLines;
    }

    for (std::string &line : highlightedLines)
    {
        const std::string bracketedName = "[" + currentLocationName + "]";
        const std::size_t matchPosition = line.find(bracketedName);
        if (matchPosition == std::string::npos)
        {
            continue;
        }

        line.replace(matchPosition, bracketedName.size(), color("30;103", bracketedName));
    }

    return highlightedLines;
}
