// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MOXFLECSDeveloperSettings.generated.h"

class UECSModule;

/**
 *
 */
UCLASS(config = Game, defaultconfig)
class MOXFLECS_API UMO–XFLECSDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UMOXFLECSDeveloperSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, config)
	TArray<TSubclassOf<UECSModule>> ECSModules;

};
