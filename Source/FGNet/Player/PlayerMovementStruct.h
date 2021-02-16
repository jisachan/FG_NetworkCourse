// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerMovementStruct.generated.h"


USTRUCT()
struct FPlayerMovementStruct
{
	GENERATED_BODY()

public:

	float PlayerYaw;
	float ClientForward;
	float PlayerFriction;
	float PlayerBrakingFriction;
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

private:

	uint8 CompressUIntByte(float valueToCompress, float maxValue);
	int8 CompressIntByte(float valueToCompress, float maxValue);
	float DecompressByte(float compressedValue, float maxValue);
	float MaxForward = 3.f;
	float MaxFriction = 3.f;
	int UInt8Size = 255;
	int Int8Size = 127;
};

template<>
struct TStructOpsTypeTraits<FPlayerMovementStruct> : public TStructOpsTypeTraitsBase2<FPlayerMovementStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};