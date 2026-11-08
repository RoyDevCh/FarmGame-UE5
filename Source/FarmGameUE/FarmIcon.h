#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

/** Item icon lookup for Slate rows (textures imported from the original Unity Sprites/Icons). */
namespace FarmIcon
{
	/** Cached brush for an item id; null when no icon was imported. */
	TSharedPtr<FSlateBrush> BrushFor(FName ItemId);
}
