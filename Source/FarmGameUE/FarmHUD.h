#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FarmHUD.generated.h"

/** Immediate-mode HUD: day, gold, inventory, selected seed, interaction prompt. */
UCLASS()
class FARMGAMEUE_API AFarmHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
