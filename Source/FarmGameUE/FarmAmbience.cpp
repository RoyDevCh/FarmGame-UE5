#include "FarmAmbience.h"
#include "Sound/SoundBase.h"

namespace
{
	/** Imported Unity placeholder bed; one loop is reused for every track. */
	const TCHAR* PlaceholderSoundPath = TEXT("/Game/Art/Audio/craft.craft");
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
	if (Audio)
	{
		Audio->SetSound(PlaceholderSound);
	}
}

void AFarmAmbience::UpdateAmbience(float DayProgress, bool bRaining)
{
	const FarmAmbienceLogic::ETrack Track = FarmAmbienceLogic::TrackFor(DayProgress, bRaining);
	if (bHasTrack && Track == CurrentTrack) return;

	CurrentTrack = Track;
	bHasTrack = true;

	if (Audio)
	{
		Audio->Stop();
		Audio->SetSound(PlaceholderSound);
		Audio->SetVolumeMultiplier(FarmAmbienceLogic::VolumeFor(Track));
		if (PlaceholderSound)
		{
			Audio->Play();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FarmGame: ambience %s"), FarmAmbienceLogic::NameFor(Track));
}
