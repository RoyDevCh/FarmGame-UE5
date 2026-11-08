#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmWeather.generated.h"

class USceneComponent;

/**
 * Seasons that drive crop growth and the weather cycle.
 *
 * UHT only permits reflected types inside a namespace when the module opts into
 * bAllowUETypesInNamespaces (FarmGameUE does not), so the enum is declared at
 * global scope. FarmWeather re-exports it, so plain C++ call sites still write
 * FarmWeather::ESeason exactly as documented. Reflected declarations
 * (UPROPERTY/UFUNCTION) must use the unqualified ESeason name so UHT resolves it.
 */
UENUM(BlueprintType)
enum class ESeason : uint8
{
	Spring	UMETA(DisplayName = "Spring"),
	Summer	UMETA(DisplayName = "Summer"),
	Autumn	UMETA(DisplayName = "Autumn"),
	Winter	UMETA(DisplayName = "Winter")
};

/** Pure season and weather rules: deterministic and free of UObject/world state. */
namespace FarmWeather
{
	using ::ESeason;

	/** Length of one season and of the full four-season cycle, in game days. */
	inline constexpr int32 DaysPerSeason = 28;
	inline constexpr int32 DaysPerYear = DaysPerSeason * 4;

	/** Days 1..28 are Spring, 29..56 Summer, 57..84 Autumn, 85..112 Winter; repeats every 112 days. */
	inline ESeason SeasonForDay(int32 Day)
	{
		int64 CycleDay = (static_cast<int64>(Day) - 1) % DaysPerYear;
		if (CycleDay < 0)
		{
			CycleDay += DaysPerYear;
		}

		if (CycleDay < DaysPerSeason) return ESeason::Spring;
		if (CycleDay < DaysPerSeason * 2) return ESeason::Summer;
		if (CycleDay < DaysPerSeason * 3) return ESeason::Autumn;
		return ESeason::Winter;
	}

	/** Deterministic 30% rain roll: ((Day * 37 + RollPercent) % 100) < 30. */
	inline bool RollRain(int32 Day, int32 RollPercent)
	{
		const int64 Value = static_cast<int64>(Day) * 37 + RollPercent;
		return (Value % 100) < 30;
	}

	/** Crop growth multiplier for the season. */
	inline float GrowthSpeed(ESeason Season)
	{
		switch (Season)
		{
		case ESeason::Spring: return 1.0f;
		case ESeason::Summer: return 1.2f;
		case ESeason::Autumn: return 0.9f;
		case ESeason::Winter: return 0.5f;
		default: return 1.0f;
		}
	}

	/** Human-readable season name for logs and the HUD. */
	inline const TCHAR* SeasonName(ESeason Season)
	{
		switch (Season)
		{
		case ESeason::Spring: return TEXT("Spring");
		case ESeason::Summer: return TEXT("Summer");
		case ESeason::Autumn: return TEXT("Autumn");
		case ESeason::Winter: return TEXT("Winter");
		default: return TEXT("Unknown");
		}
	}
}

/** World-side weather state owned by the GameMode: tracks the day and its season. */
UCLASS()
class FARMGAMEUE_API AFarmWeatherSystem : public AActor
{
	GENERATED_BODY()

public:
	AFarmWeatherSystem();

	/** Stores the new day, derives its season, and logs the result. */
	void UpdateForDay(int32 Day);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Weather")
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farm|Weather")
	int32 CurrentDay = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm|Weather")
	ESeason CurrentSeason = ESeason::Spring;
};
