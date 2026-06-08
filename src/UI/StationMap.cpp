#include "detail/StationMap.h"

#include "Core/GameIds.h"
#include "Core/TextUtils.h"
#include "detail/ConsoleDecor.h"

namespace
{
std::vector<std::string> buildLocalizedMapLines()
{
    return {"                            [" + TextUtils::localizeLocationId(GameIds::kDockLocation) + "]",
            "                                     |",
            "                                     |",
            "[" + TextUtils::localizeLocationId(GameIds::kMedbayLocation) + "] ---- [" +
                TextUtils::localizeLocationId(GameIds::kIntakeLocation) + "]",
            "                                     |",
            "                                     |",
            "                              [" + TextUtils::localizeLocationId(GameIds::kArchiveLocation) + "] ---- [" +
                TextUtils::localizeLocationId(GameIds::kMachineShopLocation) + "] ---- [" +
                TextUtils::localizeLocationId(GameIds::kReactorSpineLocation) + "]",
            "                                     |",
            "                                     |",
            "                               [" + TextUtils::localizeLocationId(GameIds::kSecurityLocation) + "] ---- [" +
                TextUtils::localizeLocationId(GameIds::kCommandBridgeLocation) + "] ---- [" +
                TextUtils::localizeLocationId(GameIds::kShuttleBayLocation) + "]"};
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
