// Fill out your copyright notice in the Description page of Project Settings.

//#include "PlayerSetting.h"
#include "PlayerMovementStruct.h"

bool FPlayerMovementStruct::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	//unsigned Byte = 0-255
	//signed Byte = -128-127
	uint8 ByteYaw = FRotator::CompressAxisToByte(PlayerYaw);
	uint8 ByteFriction = CompressUIntByte(PlayerFriction, MaxFriction);
	uint8 ByteBrakingFriction = CompressUIntByte(PlayerBrakingFriction, MaxFriction);
	int8 ByteForward = CompressIntByte(ClientForward, MaxForward);

	uint8 B = (ByteYaw != 0);

	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteYaw; else ByteYaw = 0;	
	
	B = (ByteFriction != 0);
	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteFriction; else ByteFriction = 0;
	
	B = (ByteBrakingFriction != 0);
	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteBrakingFriction; else ByteBrakingFriction = 0;
	
	B = (ByteForward != 0);
	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteForward; else ByteForward = 0;

	if (Ar.IsLoading())
	{
		PlayerYaw = FRotator::DecompressAxisFromByte(ByteYaw);
		ClientForward = DecompressByte(ByteForward, MaxForward);
		PlayerFriction = DecompressByte(ByteFriction, MaxFriction);
		PlayerBrakingFriction = DecompressByte(ByteBrakingFriction, MaxFriction);
	}

	return true;
}

uint8 FPlayerMovementStruct::CompressUIntByte(float valueToCompress, float maxValue)
{
	return valueToCompress * Int8Size / maxValue;
}

int8 FPlayerMovementStruct::CompressIntByte(float valueToCompress, float maxValue)
{
	return valueToCompress * Int8Size / maxValue;
}

float FPlayerMovementStruct::DecompressByte(float compressedValue, float maxValue)
{
	return compressedValue * maxValue / Int8Size;
}
