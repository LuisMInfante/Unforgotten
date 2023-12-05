#pragma once

#include "UObject/ObjectMacros.h"

/** Custom movement modes for Characters. */
UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	MOVE_WallRun   UMETA(DisplayName = "WallRunning"),
	MOVE_Crouch   UMETA(DisplayName = "Crouch"),
	MOVE_Slide   UMETA(DisplayName = "Slide"),
	CMOVE_MAX			UMETA(Hidden),
};