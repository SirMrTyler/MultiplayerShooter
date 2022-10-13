// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class WITCHYSHOOTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	// UPROPERTY gives Character AnimBP, and AnimInstance C++ access to BlasterCharacter member functions/variables
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class ABlasterCharacter* BlasterCharacter;	

	// UPROPERTY passes current speed to the Character's AnimBP
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	// UPROPERTY lets the Character's AnimBP know when the PC is the air or not
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	// bIsAccelerating is used to update our animBP to state whether the character is moving
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	// bWeaponEquipped is used to update our AnimBP when the player equips/unequips a weapon
	bool bWeaponEquipped;

	// This is used to attach left hand to lefthand socket of weapon.
	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	// bWeaponEquipped is used to update our AnimBP when the player presses the crouch key
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	// bAiming is used to update our AnimBP when the player presses the aim key.
	bool bAiming;

	// UPROPERTY updates movement animation depending on direction of player movementa
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	// UPROPERTY updates movement animation when player moves the mouse quickly
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	// Used to store the rotation of the character in the previous frame
	FRotator CharacterRotationLastFrame;
	// Used to store character rotation as of current frame
	FRotator CharacterRotation;
	// Used to store the difference between last frame and current frame rotation
	FRotator DeltaRotation;

	// The 2 following UPROPERTIES will be used to update the look angle of the player during animations according to AO_Yaw/Pitch in our BlasterCharacter file
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	// This UPROPERTY is used to attach the left hand to the weapon's under barrel
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
};
