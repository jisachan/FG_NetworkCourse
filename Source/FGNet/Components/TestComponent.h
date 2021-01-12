// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"


#include "TestComponent.generated.h"



class UStaticMesh;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FGNET_API UTestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTestComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		UStaticMesh* Meshie;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	
	UFUNCTION(BlueprintCallable)
	void SpawnRocket();

};
