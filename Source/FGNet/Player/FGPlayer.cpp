#include "FGPlayer.h"
#include "../FGMovementStatics.h"
#include "../Components/FGMovementComponent.h"

#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "Engine/NetDriver.h"
#include "Net/UnrealNetwork.h"
#include "PlayerSetting.h"
#include "PlayerMovementStruct.h"
#include "../DebugWidget.h"
#include "../Pickup.h"
#include "../Components/FGRocketComponent.h"
#include "../Components/RocketSpawner.h"
#include <Components/ActorComponent.h>

AFGPlayer::AFGPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->SetupAttachment(CollisionComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	MovementComponent = CreateDefaultSubobject<UFGMovementComponent>(TEXT("MovementComponent"));

	RocketSpawner = CreateDefaultSubobject<URocketSpawner>(TEXT("RocketSpawner"));
	SetReplicateMovement(false);

}

void AFGPlayer::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent->SetUpdatedComponent(CollisionComponent);

	CreateDebugWidget();
	if (DebugMenuInstance != nullptr)
	{
		DebugMenuInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HasAuthority())
	{
		ServerNumRockets = AmmoAtStart;
		NumRockets = AmmoAtStart;
		BP_OnNumRocketsChanged(NumRockets);
	}

	SpawnRockets();
	BP_OnNumRocketsChanged(NumRockets);
	BP_OnHealthChanged(Health);

	OriginalMeshOffset = MeshComponent->GetRelativeLocation();
}

void AFGPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireCooldownElapsed -= DeltaTime;

	FFGFrameMovement FrameMovement = MovementComponent->CreateFrameMovement();

	if (IsLocallyControlled())
	{
		ClientTimeStamp += DeltaTime;

		if (MoveData == nullptr)
		{
			return;
		}

		const float MaxVelocity = MoveData->MaxVelocity;
		const float Friction = IsBraking() ? MoveData->BrakingFriction : MoveData->Friction;
		const float Alpha = FMath::Clamp(FMath::Abs(MovementVelocity / (MaxVelocity * 0.75F)), 0.0F, 1.0F);
		const float TurnSpeed = FMath::InterpEaseOut(0.0F, MoveData->TurnSpeedDefault, Alpha, 5.0F);
		const float TurnDirection = MovementVelocity > 0.0F ? Turn : -Turn;

		Yaw += (TurnDirection * TurnSpeed) * DeltaTime;
		FQuat WantedFacingDirection = FQuat(FVector::UpVector, FMath::DegreesToRadians(Yaw));
		MovementComponent->SetFacingRotation(WantedFacingDirection);

		AddMovementVelocity(DeltaTime);
		MovementVelocity *= FMath::Pow(Friction, DeltaTime);

		MovementComponent->ApplyGravity();
		FrameMovement.AddDelta(GetActorForwardVector() * MovementVelocity * DeltaTime);
		MovementComponent->Move(FrameMovement);
		PlayerMovement.PlayerYaw = GetActorRotation().Yaw;
		PlayerMovement.ClientForward = Forward;
		PlayerMovement.PlayerFriction = Friction;
		PlayerMovement.PlayerBrakingFriction = Friction;
		Server_SendMovement(GetActorLocation(), ClientTimeStamp, PlayerMovement);
		UE_LOG(LogTemp, Warning, TEXT("Forward: %f"), Forward);
	}
	else
	{
		const float Friction = IsBraking() ? MoveData->BrakingFriction : MoveData->Friction;
		MovementVelocity *= FMath::Pow(Friction, DeltaTime);
		FrameMovement.AddDelta(GetActorForwardVector() * MovementVelocity * DeltaTime);
		MovementComponent->Move(FrameMovement);

		if (bPerformNetworkSmoothing)
		{
			const FVector NewRelativeLocation = FMath::VInterpTo(MeshComponent->GetRelativeLocation(), OriginalMeshOffset, LastCorrectionDelta, LocationLerpSpeed);
			UE_LOG(LogTemp, Warning, TEXT("NewRelativeLocation: %s"), *NewRelativeLocation.ToString());
			MeshComponent->SetRelativeLocation(NewRelativeLocation);
		}
	}
}

void AFGPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Accelerate"), this, &AFGPlayer::Handle_Accelerate);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AFGPlayer::Handle_Turn);

	PlayerInputComponent->BindAction(TEXT("Brake"), IE_Pressed, this, &AFGPlayer::Handle_BrakePressed);
	PlayerInputComponent->BindAction(TEXT("Brake"), IE_Released, this, &AFGPlayer::Handle_BrakeReleased);

	PlayerInputComponent->BindAction(TEXT("DebugMenu"), IE_Pressed, this, &AFGPlayer::Handle_DebugMenuPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AFGPlayer::Handle_FirePressed);
}

int32 AFGPlayer::GetPing() const
{
	if (GetPlayerState())
	{
		return static_cast<int32>(GetPlayerState()->GetPing());
	}

	return 0;
}

const static float MaxMoveDeltaTime = 0.125f;

void AFGPlayer::Server_SendMovement_Implementation(FVector_NetQuantize100 ActorLocation, float TimeStamp, const FPlayerMovementStruct& SerializedMovement)
{
	Multicast_SendMovement(ActorLocation, TimeStamp, SerializedMovement);
}

void AFGPlayer::Multicast_SendMovement_Implementation(FVector_NetQuantize100 ActorLocation, float TimeStamp,const FPlayerMovementStruct& SerializedMovement)
{
	if (!IsLocallyControlled())
	{
		Forward = SerializedMovement.ClientForward;
		const float DeltaTime = FMath::Min(TimeStamp - ClientTimeStamp, MaxMoveDeltaTime);
		ClientTimeStamp = TimeStamp;
		const float Friction = IsBraking() ? SerializedMovement.PlayerBrakingFriction : SerializedMovement.PlayerFriction;
		UE_LOG(LogTemp, Warning, TEXT("%i Friction: %f"), GetLocalRole(), Friction);
		MovementVelocity *= FMath::Pow(Friction, DeltaTime);
		AddMovementVelocity(DeltaTime);
		MovementComponent->SetFacingRotation(FRotator(0.0f, SerializedMovement.PlayerYaw, 0.0f));

		const FVector DeltaDiff = ActorLocation - GetActorLocation();

		if (DeltaDiff.SizeSquared() > FMath::Square(40.0f))
		{
			if (bPerformNetworkSmoothing)
			{
				const FScopedPreventAttachedComponentMove PreventMeshMove(MeshComponent);
				MovementComponent->UpdatedComponent->SetWorldLocation(ActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
				LastCorrectionDelta = DeltaTime;
			}
			else
			{
				SetActorLocation(ActorLocation);
			}
		}
	}
}

void AFGPlayer::AddMovementVelocity(float DeltaTime)
{
	if (MoveData == nullptr)
	{
		return;
	}

	const float MaxVelocity = MoveData->MaxVelocity;
	const float Acceleration = MoveData->Acceleration;
	MovementVelocity += Forward * Acceleration * DeltaTime;
	MovementVelocity = FMath::Clamp(MovementVelocity, -MaxVelocity, MaxVelocity);
}

void AFGPlayer::ShowDebugMenu()
{
	CreateDebugWidget();

	if (DebugMenuInstance == nullptr)
	{
		return;
	}

	DebugMenuInstance->SetVisibility(ESlateVisibility::Visible);
	DebugMenuInstance->BP_OnShowWidget();
}

void AFGPlayer::HideDebugMenu()
{
	if (DebugMenuInstance == nullptr)
	{
		return;
	}

	DebugMenuInstance->SetVisibility(ESlateVisibility::Collapsed);
	DebugMenuInstance->BP_OnHideWidget();
}

void AFGPlayer::FireRocket()
{
	if (FireCooldownElapsed > 0.0f)
	{
		return;
	}

	if (NumRockets <= 0 && !bUnlimitedRockets)
	{
		return;
	}

	if (GetNumActiveRockets() >= MaxActiveRockets)
	{
		return;
	}

	UFGRocketComponent* NewRocket = GetFreeRocket();

	if (!ensure(NewRocket != nullptr))
	{
		return;
	}

	FireCooldownElapsed = MoveData->FireCooldown;

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		if (HasAuthority())
		{
			Server_FireRocket(NewRocket, GetRocketStartLocation(), GetActorRotation());
		}
		else
		{
			NumRockets--;
			BP_OnNumRocketsChanged(NumRockets);
			NewRocket->StartMoving(GetActorForwardVector(), GetRocketStartLocation());
			Server_FireRocket(NewRocket, GetRocketStartLocation(), GetActorRotation());
		}
	}
}

void AFGPlayer::TakeRocketDamage(float Damage)
{
	if (HasAuthority())
	{
		Health -= Damage;
		BP_OnHealthChanged(Health);
		UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
		if (Health <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dead"));
			this->Destroy();
		}
	}
}

void AFGPlayer::Server_FireRocket_Implementation(UFGRocketComponent* NewRocket, const FVector_NetQuantize100& RocketStartLocation, const FRotator& RocketFacingRotation)
{
	if ((ServerNumRockets - 1) < 0 && !bUnlimitedRockets)
	{
		Client_RemoveRocket(NewRocket);
	}
	else
	{
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(RocketFacingRotation.Yaw, GetActorForwardVector().Rotation().Yaw) * 0.5f;
		const FRotator NewFacingRotation = RocketFacingRotation + FRotator(0.0f, DeltaYaw, 0.0f);
		ServerNumRockets--;
		Multicast_FireRocket(NewRocket, RocketStartLocation, NewFacingRotation);
	}
}

void AFGPlayer::Multicast_FireRocket_Implementation(UFGRocketComponent* NewRocket, const FVector_NetQuantize100& RocketStartLocation, const FRotator& RocketFacingRotation)
{
	if (!ensure(NewRocket != nullptr))
	{
		return;
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		NewRocket->ApplyCorrection(RocketFacingRotation.Vector());
	}
	else
	{
		NumRockets--;
		BP_OnNumRocketsChanged(NumRockets);
		NewRocket->StartMoving(RocketFacingRotation.Vector(), RocketStartLocation);
	}
}

void AFGPlayer::Client_RemoveRocket_Implementation(UFGRocketComponent* RocketToRemove)
{
	RocketToRemove->MakeFree();
}

void AFGPlayer::Cheat_IncreaseRockets(int32 InNumRockets)
{
	if (IsLocallyControlled())
	{
		NumRockets += InNumRockets;
		BP_OnNumRocketsChanged(NumRockets);
	}
}

void AFGPlayer::SpawnRockets()
{
	if (HasAuthority())
	{
		const int32 RocketCache = AmmoAtStart;

		for (int32 Index = 0; Index < RocketCache; ++Index)
		{
			RocketSpawner->SpawnRocket();
			UE_LOG(LogTemp, Warning, TEXT("NumOfRockets: %i"), RocketCompInstances.Num());
		}
	}
}

FVector AFGPlayer::GetRocketStartLocation() const
{
	const FVector StartLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
	return StartLocation;
}

UFGRocketComponent* AFGPlayer::GetFreeRocket() const
{
	for (UFGRocketComponent* Rocket : RocketCompInstances)
	{
		if (Rocket == nullptr)
		{
			continue;
		}

		if (Rocket->IsFree())
		{
			return Rocket;
		}
	}

	return nullptr;
}

int32 AFGPlayer::GetNumActiveRockets() const
{
	int32 NumActive = 0;
	for (UFGRocketComponent* Rocket : RocketCompInstances)
	{
		if (!Rocket->IsFree())
		{
			NumActive++;
		}
	}
	return NumActive;
}

void AFGPlayer::Handle_Accelerate(float Value)
{
	Forward = Value;

	//To test decimals:
	//Forward = Value/4.2;
}

void AFGPlayer::Handle_Turn(float Value)
{
	Turn = Value;
}

void AFGPlayer::Handle_BrakePressed()
{
	bIsBraking = true;
}

void AFGPlayer::Handle_BrakeReleased()
{
	bIsBraking = false;
}

void AFGPlayer::Handle_DebugMenuPressed()
{
	bShowDebugMenu = !bShowDebugMenu;

	if (bShowDebugMenu)
	{
		ShowDebugMenu();
	}
	else
	{
		HideDebugMenu();
	}
}

void AFGPlayer::Handle_FirePressed()
{
	FireRocket();
}

void AFGPlayer::CreateDebugWidget()
{
	if (DebugMenuClass == nullptr)
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	if (DebugMenuInstance == nullptr)
	{
		DebugMenuInstance = CreateWidget<UDebugWidget>(GetWorld(), DebugMenuClass);
		DebugMenuInstance->AddToViewport();
	}
}

void AFGPlayer::OnPickup(APickup* Pickup)
{
	if (IsLocallyControlled())
	{
		Server_OnPickup(Pickup);
	}
}

void AFGPlayer::Server_OnPickup_Implementation(APickup* Pickup)
{
	ServerNumRockets += Pickup->NumRockets;
	NumRockets += Pickup->NumRockets;
	if (NumRockets > AmmoCapacity)
	{
		NumRockets = AmmoCapacity;
		ServerNumRockets = AmmoCapacity;
	}
	BP_OnNumRocketsChanged(NumRockets);
}

void AFGPlayer::OnRepNumRocketsChanged()
{
	BP_OnNumRocketsChanged(NumRockets);
}

void AFGPlayer::OnRepHealthChanged()
{
	BP_OnHealthChanged(Health);
}

void AFGPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFGPlayer, RocketCompInstances);
	DOREPLIFETIME(AFGPlayer, NumRockets);
	DOREPLIFETIME(AFGPlayer, Health);
}