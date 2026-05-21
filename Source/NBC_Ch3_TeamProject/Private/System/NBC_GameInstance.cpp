// Fill out your copyright notice in the Description page of Project Settings.


#include "System/NBC_GameInstance.h"

void UNBC_GameInstance::ClearSavedData()
{
	SavedWeaponClasses.Empty();
	SavedCurrentWeaponIndex = -1;
}