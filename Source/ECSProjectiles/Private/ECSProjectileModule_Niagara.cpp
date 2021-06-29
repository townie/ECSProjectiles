// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Niagara.h"

#include "DrawDebugHelpers.h"
#include "flecs.h"
#include "ECSProjectileModule_SimpleSim.h"
#include "MegaFLECSTypes.h"
#if ECSPROJECTILES_NIAGARA
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraFunctionLibrary.h"
#include "Async/ParallelFor.h"

#endif

void UECSProjectileModule_Niagara::InitializeComponents(TSharedPtr<flecs::world> World)
{
#if ECSPROJECTILES_NIAGARA
	flecs::component<FECSNiagaraComponentHandle>(*World.Get());
	flecs::component<FECSNiagaraSystemHandle>(*World.Get());

	flecs::component<FECSRNiagaraProjectileManager>(*World.Get());
	flecs::component<FECSRNiagaraHitsManager>(*World.Get());
	flecs::component<FECSNiagaraGroupManager>(*World.Get());

#endif
}

namespace FNiagaraECSSystem
{
#if ECSPROJECTILES_NIAGARA
	//Just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector but copied here.
	//not a system? should put somewhere else I suppose
	void SetNiagaraVectorArray(UNiagaraComponent* NiagaraSystem, FName ParameterName,TArray<FVector> VectorArray)
	{
		if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
			UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(NiagaraSystem, ParameterName))
		{
			FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
			ArrayDI->FloatData = VectorArray;
			ArrayDI->MarkRenderDataDirty();
		}
	}
	
	void UpdateNiagaraPositions(flecs::entity e, const FECSNiagaraComponentHandle& Handle, const FECSBulletTransform& Transform)
	{
		if (Handle.Component.IsValid())
		{
			Handle.Component->SetVectorParameter(Handle.LocationParameterName, Transform.CurrentTransform.GetLocation());
			Handle.Component->SetVectorParameter(Handle.PreviousLocationParameterName, Transform.PreviousTransform.GetLocation());
		}
	}


	void UpdateNiagaraPositionsArray(flecs::iter& Iter, FECSBulletTransform* BulletTransform)
	{
		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandleObject = Iter.term_id(2).object();
		//
		auto ParentHandle = const_cast<FECSNiagaraGroupManager*>(ParentHandleObject.get<FECSNiagaraGroupManager>());
		ParentHandle->IteratorOffset += Iter.count();

		//TODO: shrink these eventually? could set a system to run ever x seconds etc
		 ParentHandle->FirstArray.SetNum(ParentHandle->IteratorOffset, false);
		 ParentHandle->SecondArray.SetNum(ParentHandle->IteratorOffset, false);
		
		 auto PositionArrayDirect = ParentHandle->FirstArray.GetData();
		 auto PrevPositionArrayDirect = ParentHandle->SecondArray.GetData();
		//int Offset = ParentHandle->IteratorOffset;

		 ParallelFor(Iter.count(), [&](int32 index)
		 {
		 	auto& CurrentTransform = BulletTransform[index];
		
		 	PositionArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.CurrentTransform.GetTranslation();
		 	PrevPositionArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.PreviousTransform.GetTranslation();
		 });

	}
	void UpdateNiagaraHitsArray(flecs::iter& Iter, FECSBulletHit* BulletHit)
	{
		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandleObject = Iter.term_id(2).object();
		auto ParentHandle = const_cast<FECSNiagaraGroupManager*>(ParentHandleObject.get<FECSNiagaraGroupManager>());
		ParentHandle->IteratorOffset += Iter.count();

		//TODO: shrink these eventually? could set a system to run ever x seconds etc
		ParentHandle->FirstArray.SetNum(ParentHandle->IteratorOffset, false);
		ParentHandle->SecondArray.SetNum(ParentHandle->IteratorOffset, false);
		UE_LOG(LogTemp,Warning,TEXT("Added %i hits!"),Iter.count())
		auto FirstArrayDirect = ParentHandle->FirstArray.GetData();
		auto SecondArrayDirect = ParentHandle->SecondArray.GetData();

		ParallelFor(Iter.count(), [&](int32 index)
		{
			auto CurrentHitResult = BulletHit[index].HitResult;
		
			FirstArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentHitResult.ImpactPoint;
			SecondArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentHitResult.ImpactNormal;
		});

	}
	void NiagaraHitFX(flecs::iter& Iter, FECSBulletHit* BulletHit)
	{
		UWorld* World = (UWorld*)Iter.world().get_context();
		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandleObject = Iter.term_id(2).object();
		auto ParentHandle = const_cast<FECSNiagaraSystemHandle*>(ParentHandleObject.get<FECSNiagaraSystemHandle>());
		for (auto i : Iter)
		{
			const auto CurrentHitResult = BulletHit[i].HitResult;
			if(ParentHandle->System.IsValid())
			{
				auto SpawnedSystemComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World,ParentHandle->System.Get(),
		CurrentHitResult.ImpactPoint,
		CurrentHitResult.ImpactNormal.ForwardVector.Rotation(),
			FVector(1.0f),true,true,ENCPoolMethod::None);

				SpawnedSystemComp->SetWorldRotation(CurrentHitResult.Normal.ToOrientationQuat().GetForwardVector().ToOrientationRotator());

			}
		}

	}
	void PushNiagaraPositionsArray(flecs::entity e, FECSNiagaraGroupManager& Handle)
	{
		if (Handle.Component.IsValid())
		{
			//Set current positions
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.FirstParameterName,Handle.FirstArray);
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.SecondParameterName,Handle.SecondArray);
		}
		Handle.FirstArray.Empty();
		Handle.SecondArray.Empty();

		Handle.IteratorOffset = 0;
	}


#endif

}

void UECSProjectileModule_Niagara::InitializeSystems(TSharedPtr<flecs::world> World)
{
#if ECSPROJECTILES_NIAGARA
	World->system<FECSNiagaraComponentHandle, FECSBulletTransform>("Write Individual Projectile Positions to Niagara")
		.kind(flecs::PreStore)
		.each(&FNiagaraECSSystem::UpdateNiagaraPositions);

	World->system<FECSBulletTransform>("Write Grouped Projectile Positions TArrays")
		.kind(flecs::PreStore)
		.term<FECSRNiagaraProjectileManager>(flecs::Wildcard)
		.iter(&FNiagaraECSSystem::UpdateNiagaraPositionsArray);
	
	World->system<FECSBulletHit>("Write Grouped Projectile Hits TArrays when set")
		.kind(flecs::UnSet)
		.term<FECSRNiagaraHitsManager>(flecs::Wildcard)
		.iter(&FNiagaraECSSystem::UpdateNiagaraHitsArray);
	
	World->system<FECSBulletHit>("SpawnProjectile Hits FX component")
		.kind(flecs::UnSet)
		.term<FECSRNiagaraHitFX>(flecs::Wildcard)
		.iter(&FNiagaraECSSystem::NiagaraHitFX);

	World->system<FECSNiagaraGroupManager>("Grouped Particle TArrays to Niagara")
		.kind(flecs::PreStore)
		.each(&FNiagaraECSSystem::PushNiagaraPositionsArray);
#endif
}

