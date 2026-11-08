#include "FarmWeather.h"
#include "Components/SceneComponent.h"

AFarmWeatherSystem::AFarmWeatherSystem()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AFarmWeatherSystem::UpdateForDay(int32 Day)
{
	CurrentDay = Day;
	CurrentSeason = FarmWeather::SeasonForDay(Day);

	UE_LOG(LogTemp, Log, TEXT("FarmGame: season %s"), FarmWeather::SeasonName(CurrentSeason));
}
