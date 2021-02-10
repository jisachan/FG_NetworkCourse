// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerSetting.h"
#include "PlayerMovementStruct.generated.h"


USTRUCT()
struct FPlayerMovementStruct
{
	GENERATED_BODY()

public:

	//UPROPERTY(EditAnywhere, Category = Settings)
	UPlayerSetting* MovementData;

	void SetMovementData(UPlayerSetting* InMovementData)
	{
		MovementData = InMovementData;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

private:

	float Compress8UnsignedByte(float valueToCompress, float maxValue);
	float Decompress8UnsignedByte(float compressedValue, float maxValue);
	float MaxAcceleration = 800.f;
	float MaxBrakingFriction = 1.f;
	float MaxTurnSpeed = 300.f;
	float MaxFriction = 1.f;
	float MaxMaxVelocity = 3000.f;
	float Int8Size = 255.f;
};

template<>
struct TStructOpsTypeTraits<FPlayerMovementStruct> : public TStructOpsTypeTraitsBase2<FPlayerMovementStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};