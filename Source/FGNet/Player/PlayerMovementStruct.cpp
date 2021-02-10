// Fill out your copyright notice in the Description page of Project Settings.

//#include "PlayerSetting.h"
#include "PlayerMovementStruct.h"

bool FPlayerMovementStruct::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	//unsigned Byte = 0-255
	//signed Byte = -128-127
	uint8 ByteAcceleration = Compress8UnsignedByte(MovementData->Acceleration, MaxAcceleration);
	uint8 ByteBrakeFriction = Compress8UnsignedByte(MovementData->BrakingFriction, MaxBrakingFriction);
	uint8 ByteTurnSpeed = Compress8UnsignedByte(MovementData->TurnSpeedDefault, MaxTurnSpeed);
	uint8 ByteFriction = Compress8UnsignedByte(MovementData->Friction, MaxFriction);
	uint8 ByteMaxVelocity = Compress8UnsignedByte(MovementData->MaxVelocity, MaxMaxVelocity);

	uint8 B = (ByteAcceleration != 0);
	Ar.SerializeBits(&B, 1);
	if (B)  Ar << ByteAcceleration; else ByteAcceleration = 0;

	B = (ByteBrakeFriction != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << ByteBrakeFriction; else ByteBrakeFriction = 0;

	B = (ByteTurnSpeed != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << ByteTurnSpeed; else ByteTurnSpeed = 0;

	B = (ByteFriction != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << ByteFriction; else ByteFriction = 0;

	B = (ByteMaxVelocity != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << ByteMaxVelocity; else ByteMaxVelocity = 0;

	if (Ar.IsLoading())
	{
		MovementData->Acceleration = Decompress8UnsignedByte(ByteAcceleration, MaxAcceleration);
		MovementData->MaxVelocity = Decompress8UnsignedByte(ByteMaxVelocity, MaxMaxVelocity);
		MovementData->BrakingFriction = Decompress8UnsignedByte(ByteBrakeFriction, MaxBrakingFriction);
		MovementData->TurnSpeedDefault = Decompress8UnsignedByte(ByteTurnSpeed, MaxTurnSpeed);
		MovementData->Friction = Decompress8UnsignedByte(ByteFriction, MaxFriction);
	}

	return true;
}


float FPlayerMovementStruct::Compress8UnsignedByte(float valueToCompress, float maxValue)
{
	return valueToCompress * Int8Size / maxValue;
}

float FPlayerMovementStruct::Decompress8UnsignedByte(float compressedValue, float maxValue)
{
	return compressedValue * maxValue / Int8Size;
}