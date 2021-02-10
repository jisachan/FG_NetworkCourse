#pragma once

#include "GameFramework/Pawn.h"
#include "PlayerMovementStruct.h"

#include "FGPlayer.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UFGMovementComponent;
class UStaticMeshComponent;
class USphereComponent;
class UPlayerSetting;
class UDebugWidget;
class APickup;
class UFGRocketComponent;
class URocketSpawner;

UCLASS()
class FGNET_API AFGPlayer : public APawn
{
	GENERATED_BODY()

public:
	AFGPlayer();

	FPlayerMovementStruct PlayerMovement;

	UPROPERTY(EditAnywhere, Category = Settings)
		UPlayerSetting* MoveData = nullptr;

protected:

	UPROPERTY(Replicated, ReplicatedUsing = OnRepHealthChanged, EditAnywhere, Category = PlayerStats);
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = PlayerStats, meta = (Tooltip = "How much ammo the player is able to carry."))
		int AmmoCapacity = 20;

	UPROPERTY(EditAnywhere, Category = PlayerStats, meta = (Tooltip = "How much ammo the player has at the start of game."))
		int AmmoAtStart = 8;

private:

#pragma region Replicated properties

	UPROPERTY(Replicated, Transient)
		TArray<UFGRocketComponent*> RocketCompInstances;

	UPROPERTY(Replicated)
		FVector ReplicatedLocation;

	UPROPERTY(Replicated, ReplicatedUsing = OnRepNumRocketsChanged)
		int32 NumRockets = 0;

	UPROPERTY(Replicated)
		float ReplicatedYaw = 0.0f;

#pragma endregion

#pragma region Components

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
		USphereComponent* CollisionComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
		USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
		UCameraComponent* CameraComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		UFGMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = RocketSpawner)
		URocketSpawner* RocketSpawner;

#pragma endregion

	UPROPERTY(Transient)
		UDebugWidget* DebugMenuInstance = nullptr;

	UPROPERTY()
		FVector TargetLocation;

	UPROPERTY()
		FQuat TargetRotation;

	UPROPERTY(EditAnywhere)
		float LocationLerpSpeed = 5.f;

	UPROPERTY(EditAnywhere)
		float RotationLerpSpeed = 5.f;

	UPROPERTY(EditAnywhere, Category = Network)
		bool bPerformNetworkSmoothing = false;

	UPROPERTY(EditAnywhere, Category = Weapon)
		bool bUnlimitedRockets = false;

	UPROPERTY(EditAnywhere)
		bool bShowDebugMenu = false;

	FVector OriginalMeshOffset;

	int32 MaxActiveRockets = 3;
	int32 ServerNumRockets = 0;

	float FireCooldownElapsed = 0.0f;
	float MovementVelocity = 0.0F;
	float Forward = 0.0F;
	float Turn = 0.0F;
	float Yaw = 0.0F;
	float ClientTimeStamp = 0.0f;
	float ServerTimeStamp = 0.0f;
	float LastCorrectionDelta = 0.0f;

	bool bIsBraking = false;

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Blueprint functions

	UFUNCTION(BlueprintPure)
		bool IsBraking() const { return bIsBraking; }

	UFUNCTION(BlueprintPure)
		int32 GetPing() const;

	UFUNCTION(BlueprintPure)
		int32 GetNumRockets() const { return NumRockets; }

	UFUNCTION(BlueprintImplementableEvent, Category = Player, meta = (DisplayName = "On Num Rockets Changed"))
		void BP_OnNumRocketsChanged(int32 NewNumRockets);

	UFUNCTION(BlueprintImplementableEvent, Category = Player, meta = (DisplayName = "On Health Changed"))
		void BP_OnHealthChanged(float InHealth);

#pragma endregion

	UPROPERTY(EditAnywhere, Category = Debug)
		TSubclassOf<UDebugWidget> DebugMenuClass;

	UFUNCTION(Server, Reliable)
		void Server_OnPickup(APickup* Pickup);

	UFUNCTION(BlueprintCallable)
		void TakeRocketDamage(float Damage);

	void AddRocketInstances(UFGRocketComponent* RocketComp) { RocketCompInstances.Add(RocketComp); }

	void OnPickup(APickup* Pickup);

	void ShowDebugMenu();
	void HideDebugMenu();

	void SpawnRockets();
	void FireRocket();

private:

	UFGRocketComponent* GetFreeRocket() const;
	FVector GetRocketStartLocation() const;
	int32 GetNumActiveRockets() const;

	UFUNCTION()
		void OnRepNumRocketsChanged();

	UFUNCTION()
		void OnRepHealthChanged();

	UFUNCTION(Server, Unreliable)
		void Server_SendMovement(const FVector& ClientLocation, float TimeStamp, float ClientForward, float ClientYaw);

	UFUNCTION(NetMulticast, Unreliable)
		void Multicast_SendMovement(const FVector& InClientLocation, float TimeStamp, float ClientForward, float ClientYaw);

	UFUNCTION(Server, Reliable)
		void Server_FireRocket(UFGRocketComponent* NewRocket, const FVector& RocketStartLocation, const FRotator& RocketFacingRotation);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_FireRocket(UFGRocketComponent* NewRocket, const FVector& RocketStartLocation, const FRotator& RocketFacingRotation);

	UFUNCTION(Client, Reliable)
		void Client_RemoveRocket(UFGRocketComponent* RocketToRemove);

	UFUNCTION(BlueprintCallable)
		void Cheat_IncreaseRockets(int32 InNumRockets);

	void AddMovementVelocity(float DeltaTime);

	void Handle_Accelerate(float Value);
	void Handle_Turn(float Value);
	void Handle_BrakePressed();
	void Handle_BrakeReleased();
	void Handle_DebugMenuPressed();
	void Handle_FirePressed();

	void CreateDebugWidget();
};
