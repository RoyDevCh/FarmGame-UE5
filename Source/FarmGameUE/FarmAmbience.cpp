#include "FarmAmbience.h"
#include "Sound/SoundBase.h"

namespace
{
	/** Imported Unity placeholder bed; one loop is reused for every track. */
	const TCHAR* PlaceholderSoundPath = TEXT("/Game/Art/Audio/craft.craft");
	const TCHAR* DaySoundPath = TEXT("/Game/Art/Audio/Music/Farm_Day.Farm_Day");
	const TCHAR* NightSoundPath = TEXT("/Game/Art/Audio/Music/Farm_Night.Farm_Night");
	const TCHAR* HouseSoundPath = TEXT("/Game/Art/Audio/Music/House_Warm.House_Warm");
}

AFarmAmbience::AFarmAmbience()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(SceneRoot);
	Audio->bAutoActivate = false;
}

void AFarmAmbience::BeginPlay()
{
	Super::BeginPlay();
	PlaceholderSound = LoadObject<USoundBase>(nullptr, PlaceholderSoundPath);
	DaySound = LoadObject<USoundBase>(nullptr, DaySoundPath);
	NightSound = LoadObject<USoundBase>(nullptr, NightSoundPath);
	HouseSound = LoadObject<USoundBase>(nullptr, HouseSoundPath);

	const bool bHouseMap = GetWorld() && GetWorld()->GetMapName().Contains(TEXT("House"));
	bHouseMode = bHouseMap && HouseSound != nullptr;

	if (Audio)
	{
		if (bHouseMode)
		{
			Audio->SetSound(HouseSound);
			Audio->SetVolumeMultiplier(0.3f);
			Audio->Play();
		}
		else
		{
			Audio->SetSound(DaySound ? DaySound : PlaceholderSound);
		}
	}
}

void AFarmAmbience::UpdateAmbience(float DayProgress, bool bRaining)
{
	if (bHouseMode) return;
	const FarmAmbienceLogic::ETrack Track = FarmAmbienceLogic::TrackFor(DayProgress, bRaining);
	if (bHasTrack && Track == CurrentTrack) return;

	CurrentTrack = Track;
	bHasTrack = true;

	if (Audio)
	{
		USoundBase* NextSound = Track == FarmAmbienceLogic::ETrack::Night && NightSound ? NightSound : (DaySound ? DaySound : PlaceholderSound);
		Audio->Stop();
		Audio->SetSound(NextSound);
		Audio->SetVolumeMultiplier(FarmAmbienceLogic::VolumeFor(Track));
		if (NextSound)
		{
			Audio->Play();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FarmGame: ambience %s"), FarmAmbienceLogic::NameFor(Track));
}
