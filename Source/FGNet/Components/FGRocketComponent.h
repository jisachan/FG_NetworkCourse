// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FGRocketComponent.generated.h"

class UStaticMesh;

UCLASS( /*ClassGroup=(Custom),*/ meta=(BlueprintSpawnableComponent) )
class FGNET_API UFGRocketComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFGRocketComponent();

protected:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:
	UPROPERTY(EditAnywhere)
		float MovementVelocity = 1300.0f;

	UPROPERTY(EditAnywhere)
		float LifeTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = VFX)
		UParticleSystem* Explosion = nullptr;

	UPROPERTY(EditAnywhere, Category = Debug)
		bool bDebugDrawCorrection = true;

	FVector OriginalFacingDirection = FVector::ZeroVector;
	FVector FacingRotationStart = FVector::ZeroVector;
	FQuat FacingRotationCorrection = FQuat::Identity;
	FVector RocketStartLocation = FVector::ZeroVector;

	FCollisionQueryParams CachedCollisionQueryParams;

	bool bIsFree = true;
	float LifeTimeElapsed = 0.0f;
	float DistanceMoved = 0.0f;


public:	
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartMoving(const FVector& Forward, const FVector& InStartLocation);
	void ApplyCorrection(const FVector& Forward);

	bool IsFree() const { return bIsFree; }

	UFUNCTION(BlueprintCallable)
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Explode();
	void MakeFree();

	void SetRocketVisibility(bool bVisible);

private:
};
