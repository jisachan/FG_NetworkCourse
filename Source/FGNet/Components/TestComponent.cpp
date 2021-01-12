// Fill out your copyright notice in the Description page of Project Settings.


#include "TestComponent.h"
#include "FGRocketComponent.h"

// Sets default values for this component's properties
UTestComponent::UTestComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTestComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTestComponent::SpawnRocket()
{
	if (Meshie)
	{
		UFGRocketComponent* MeshieComponent = NewObject<UFGRocketComponent>(this, UFGRocketComponent::StaticClass());
		MeshieComponent->RegisterComponentWithWorld(GetWorld());
		//MeshieComponent->SetStaticMesh(Meshie);
		//MeshieComponent->SetWorldLocation(GetOwner()->GetActorLocation());
		MeshieComponent->StartMoving(GetOwner()->GetActorForwardVector(), GetOwner()->GetActorLocation());	
	}
}

