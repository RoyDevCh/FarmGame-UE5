#include "FarmAudio.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace FarmAudio
{
	void Play(const UObject* WorldContext, const TCHAR* Path, float Volume)
	{
		if (!WorldContext || !Path) return;
		if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, Path))
		{
			UGameplayStatics::PlaySound2D(WorldContext, Sound, Volume);
		}
	}
}
