// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/NBC_TestGameMode.h"
#include "UObject/ConstructorHelpers.h"

ANBC_TestGameMode::ANBC_TestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
