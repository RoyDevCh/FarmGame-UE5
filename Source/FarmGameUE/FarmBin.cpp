#include "FarmBin.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmBin::AFarmBin()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bin"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetRelativeScale3D(FVector(1.6f, 1.1f, 0.9f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

FString AFarmBin::GetPrompt(const AFarmPlayerController* PC) const
{
	const UFarmInventoryComponent* Inv = PC && PC->GetPawn() ? Cast<AFarmCharacter>(PC->GetPawn())->GetInventory() : nullptr;
	const int32 Count = Inv ? Inv->GetTotalProduce() : 0;
	if (Count <= 0)
	{
		return TEXT("Shipping bin: nothing to sell yet");
	}
	return FString::Printf(TEXT("E: Sell all produce (%d items)    B: Buy 1 seed"), Count);
}

void AFarmBin::Interact(AFarmCharacter* Character)
{
	AFarmPlayerController* PC = Character ? Character->GetController<AFarmPlayerController>() : nullptr;
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	UFarmInventoryComponent* Inv = Character ? Character->GetInventory() : nullptr;
	if (!PC || !GM || !Inv) return;

	const int32 Earned = Inv->SellAllProduce(GM->Crops);
	PC->ShowToast(Earned > 0 ? FString::Printf(TEXT("Sold produce for +%d gold"), Earned) : TEXT("Nothing to sell"));
}
