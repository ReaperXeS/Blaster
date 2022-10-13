#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_Red UMETA(DisplayName = "Red"),
	ET_Blue UMETA(DisplayName = "Blue"),
	ET_NoTeam UMETA(DisplayName = "No Team"),
	ET_MAX UMETA(DisplayName = "Default Max")
};
