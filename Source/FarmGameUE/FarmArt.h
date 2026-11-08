#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;

/** Helpers for the imported Unity art (Content/Art) with graceful fallbacks. */
namespace FarmArt
{
	UStaticMesh* LoadMesh(const TCHAR* Path);

	/**
	 * Assigns a mesh and scales it so its Z size equals TargetHeight, with its
	 * bottom resting at Z = 0 (relative to the component's parent).
	 * Returns false when the mesh cannot be loaded (caller keeps its fallback).
	 */
	bool Apply(UStaticMeshComponent* Component, const TCHAR* Path, float TargetMaxSize);

	/** Same, but scales by the mesh's Z size (upright props). */
	bool ApplyHeight(UStaticMeshComponent* Component, const TCHAR* Path, float TargetHeight);

	/** Imported mesh for a crop id at a growth stage (0..MaxStage), or nullptr. */
	const TCHAR* CropMeshPath(FName CropId, int32 Stage = 0, int32 MaxStage = 2);
}
