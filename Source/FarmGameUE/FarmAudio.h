#pragma once

#include "CoreMinimal.h"

/** Small helper for the imported Unity SFX (Content/Art/Audio). */
namespace FarmAudio
{
	/** Plays a 2D sound by asset path; silently does nothing when missing. */
	void Play(const UObject* WorldContext, const TCHAR* Path, float Volume = 1.f);
}
