#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "FarmAmbience.generated.h"

class USoundBase;

/** Which looping ambience bed is playing. Plain C++ so tests need no world. */
namespace FarmAmbienceLogic
{
	enum class ETrack : uint8
	{
		Day,
		Night,
		Rain
	};

	/** DayProgress runs 0..1 over the game day (game mode: 06:00 -> 24:00); night starts at 0.8. */
	constexpr float NightThreshold = 0.8f;

	/** Rain always wins, otherwise the clock decides. Clamped so bad input still yields a valid track. */
	inline ETrack TrackFor(float DayProgress, bool bRaining)
	{
		if (bRaining) return ETrack::Rain;
		return FMath::Clamp(DayProgress, 0.f, 1.f) >= NightThreshold ? ETrack::Night : ETrack::Day;
	}

	/** Playback volume of each bed (0..1). */
	inline float VolumeFor(ETrack Track)
	{
		switch (Track)
		{
		case ETrack::Night: return 0.2f;
		case ETrack::Rain: return 0.3f;
		default: return 0.35f;
		}
	}

	/** Name used by the once-per-change log line. */
	inline const TCHAR* NameFor(ETrack Track)
	{
		switch (Track)
		{
		case ETrack::Night: return TEXT("Night");
		case ETrack::Rain: return TEXT("Rain");
		default: return TEXT("Day");
		}
	}
}

/** Day/night/rain ambience bed. The game mode pushes the clock and weather, this actor has no tick. */
UCLASS()
class FARMGAMEUE_API AFarmAmbience : public AActor
{
	GENERATED_BODY()

public:
	AFarmAmbience();

	virtual void BeginPlay() override;

	/** Restarts the bed only when the track changed. Called by the game mode with GetDayProgress() and bRaining. */
	void UpdateAmbience(float DayProgress, bool bRaining);

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UAudioComponent* Audio;

	/** Music beds imported from the original Unity project (Content/Art/Audio/Music). */
	UPROPERTY(Transient) USoundBase* DaySound = nullptr;
	UPROPERTY(Transient) USoundBase* NightSound = nullptr;
	UPROPERTY(Transient) USoundBase* HouseSound = nullptr;
	UPROPERTY(Transient) USoundBase* PlaceholderSound = nullptr;
	bool bHouseMode = false;

private:
	/** Track currently playing; bHasTrack is false until the game mode pushes the first state. */
	FarmAmbienceLogic::ETrack CurrentTrack = FarmAmbienceLogic::ETrack::Day;
	bool bHasTrack = false;
};
