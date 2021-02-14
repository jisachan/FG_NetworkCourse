// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerMovementStruct.generated.h"


USTRUCT()
struct FPlayerMovementStruct
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float PlayerYaw;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FPlayerMovementStruct> : public TStructOpsTypeTraitsBase2<FPlayerMovementStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};