#include "FarmArt.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

namespace FarmArt
{
	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		if (!Path || !*Path) return nullptr;
		return LoadObject<UStaticMesh>(nullptr, Path);
	}

	bool Apply(UStaticMeshComponent* Component, const TCHAR* Path, float TargetHeight)
	{
		UStaticMesh* Mesh = LoadMesh(Path);
		if (!Component || !Mesh) return false;

		Component->SetStaticMesh(Mesh);
		const FBox Box = Mesh->GetBoundingBox();
		const FVector Size = Box.GetSize();
		// Fit the largest dimension: flat meshes (soil) stay square, upright meshes
		// (crops/animals) are driven by their height.
		const float MaxDim = FMath::Max3(Size.X, Size.Y, Size.Z);
		const float Scale = (MaxDim > KINDA_SMALL_NUMBER && TargetHeight > 0.f) ? TargetHeight / MaxDim : 1.f;
		Component->SetRelativeScale3D(FVector(Scale));
		Component->SetRelativeLocation(FVector(0.f, 0.f, -Box.Min.Z * Scale));
		return true;
	}

	bool ApplyHeight(UStaticMeshComponent* Component, const TCHAR* Path, float TargetHeight)
	{
		UStaticMesh* Mesh = LoadMesh(Path);
		if (!Component || !Mesh) return false;

		Component->SetStaticMesh(Mesh);
		const FBox Box = Mesh->GetBoundingBox();
		const FVector Size = Box.GetSize();
		const float Scale = (Size.Z > KINDA_SMALL_NUMBER && TargetHeight > 0.f) ? TargetHeight / Size.Z : 1.f;
		Component->SetRelativeScale3D(FVector(Scale));
		Component->SetRelativeLocation(FVector(0.f, 0.f, -Box.Min.Z * Scale));
		return true;
	}

	const TCHAR* CropMeshPath(FName CropId)
	{
		if (CropId == TEXT("carrot")) return TEXT("/Game/Art/Farming/CarrotPlant.CarrotPlant");
		if (CropId == TEXT("potato")) return TEXT("/Game/Art/Farming/Short_Plant.Short_Plant");
		if (CropId == TEXT("corn")) return TEXT("/Game/Art/Farming/BeanPlant.BeanPlant");
		if (CropId == TEXT("tomato")) return TEXT("/Game/Art/Farming/strawberry_plant.strawberry_plant");
		if (CropId == TEXT("pumpkin")) return TEXT("/Game/Art/Farming/Pepper_plant.Pepper_plant");
		return nullptr;
	}
}
