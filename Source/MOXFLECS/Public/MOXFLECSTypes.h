// Copyright 2021 EmpiresTeam.  Licensed under MIT.  See LICENSE.

#pragma once

#include "flecs.h"
#include "MOXFLECSTypes.generated.h"


USTRUCT(BlueprintType)
struct MOXFLECS_API FECSEntityHandle
{
	GENERATED_BODY()
public:
	FECSEntityHandle()
		: Entity()
	{

	}

	FECSEntityHandle(flecs::entity entity);

	flecs::entity Entity;

	int32 EntityIDInt() const {return Entity.id();}
};

USTRUCT()
struct MOXFLECS_API FECSScriptStructComponent
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UScriptStruct* ScriptStruct;
};