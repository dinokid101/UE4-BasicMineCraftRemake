// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TefMineCraft.h"
#include "TefMineCraftGameMode.h"
#include "TefMineCraftHUD.h"
#include "TefMineCraftCharacter.h"

ATefMineCraftGameMode::ATefMineCraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ATefMineCraftHUD::StaticClass();
}
