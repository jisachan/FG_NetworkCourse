// Fill out your copyright notice in the Description page of Project Settings.


#include "FGRocketComponent.h"
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include "../Player/FGPlayer.h"

UFGRocketComponent::UFGRocketComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

void UFGRocketComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedCollisionQueryParams.AddIgnoredActor(GetOwner());
}

void UFGRocketComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LifeTimeElapsed -= DeltaTime;
	DistanceMoved += MovementVelocity * DeltaTime;

	FacingRotationStart = FQuat::Slerp(FacingRotationStart.ToOrientationQuat(), FacingRotationCorrection, 0.9f * DeltaTime).Vector();

#if !UE_BUILD_SHIPPING
	if (bDebugDrawCorrection)
	{
		const float ArrowLength = 3000.0f;
		const float ArrowSize = 50.0f;
		DrawDebugDirectionalArrow(GetWorld(), RocketStartLocation, RocketStartLocation + OriginalFacingDirection * ArrowLength, ArrowSize, FColor::Red);
		DrawDebugDirectionalArrow(GetWorld(), RocketStartLocation, RocketStartLocation + FacingRotationStart * ArrowLength, ArrowSize, FColor::Green);
	}
#endif

	const FVector NewLocation = RocketStartLocation + FacingRotationStart * DistanceMoved;
	SetWorldLocation(NewLocation);

	UE_LOG(LogTemp, Warning, TEXT("NewLocation: %s"), *NewLocation.ToString());

	FHitResult Hit;
	const FVector StartLoc = NewLocation;
	const FVector EndLoc = StartLoc + FacingRotationStart * 100.0f;
	GetWorld()->LineTraceSingleByChannel(Hit, StartLoc, EndLoc, ECC_Visibility, CachedCollisionQueryParams);

	if (Hit.bBlockingHit)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (AFGPlayer* Player = Cast<AFGPlayer>(HitActor))
			{
				Player->TakeRocketDamage(Damage);
			}
		}
		Explode();
	}

	if (LifeTimeElapsed < 0.0f)
	{
		Explode();
	}
}

void UFGRocketComponent::StartMoving(const FVector& Forward, const FVector& InStartLocation)
{
	FacingRotationStart = Forward;
	FacingRotationCorrection = FacingRotationStart.ToOrientationQuat();
	RocketStartLocation = InStartLocation;
	SetWorldLocationAndRotation(InStartLocation, Forward.Rotation());
	bIsFree = false;
	SetComponentTickEnabled(true);
	SetRocketVisibility(true);
	LifeTimeElapsed = LifeTime;
	DistanceMoved = 0.0f;
	OriginalFacingDirection = FacingRotationStart;
}

void UFGRocketComponent::ApplyCorrection(const FVector& Forward)
{
	FacingRotationCorrection = Forward.ToOrientationQuat();
}

void UFGRocketComponent::Explode()
{
	if (Explosion != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, this->GetComponentLocation(), this->GetComponentRotation(), true);
	}

	MakeFree();
}

void UFGRocketComponent::MakeFree()
{
	bIsFree = true;
	SetComponentTickEnabled(false);
	SetRocketVisibility(false);
}

void UFGRocketComponent::SetRocketVisibility(bool bIsVisible)
{
	SetVisibility(bIsVisible, true);
}

