// Fill out your copyright notice in the Description page of Project Settings.

//#include "PlayerSetting.h"
#include "PlayerMovementStruct.h"

bool FPlayerMovementStruct::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	//unsigned Byte = 0-255
	//signed Byte = -128-127
	UE_LOG(LogTemp, Warning, TEXT("NetSerializing"));

	uint8 ByteYaw = FRotator::CompressAxisToByte(PlayerYaw);
	
	uint8 B = (ByteYaw != 0);
	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteYaw; else ByteYaw = 0;

	if (Ar.IsLoading())
	{
		PlayerYaw = FRotator::DecompressAxisFromByte(ByteYaw);
	}

	return true;
}