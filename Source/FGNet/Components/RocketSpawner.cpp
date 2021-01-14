// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketSpawner.h"
#include "FGRocketComponent.h"
#include "../Player/FGPlayer.h"

URocketSpawner::URocketSpawner()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void URocketSpawner::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void URocketSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URocketSpawner::SpawnRocket()
{
	UWorld* World = GetWorld();

	UFGRocketComponent* NewComp = NewObject<UFGRocketComponent>(this, UFGRocketComponent::StaticClass());
	if (NewComp && World)
	{
		//NewComp->RegisterComponent();
		NewComp->RegisterComponentWithWorld(World);
		NewComp->SetStaticMesh(Mesh);
		AFGPlayer* Player = Cast<AFGPlayer>(GetOwner());
		Player->AddRocketInstances(NewComp);		
	}
}