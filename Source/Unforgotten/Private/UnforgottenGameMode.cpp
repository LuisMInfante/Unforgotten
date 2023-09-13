// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnforgottenGameMode.h"
#include "UnforgottenCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUnforgottenGameMode::AUnforgottenGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
