// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WitchyShooter/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class WITCHYSHOOTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	
protected:
	virtual void BeginPlay() override;
	
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	
	void JumpButtonPressed();
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	
	// called from tick function. 
	void AimOffset(float DeltaTime);
private:
	// This UPROPERTY tells the Camera how far it needs to be from the PC
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	// This UPROPERTY will be used to have the viewport attach to the PC
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// This is a widget used for testing who is a client, and who is the server. Modified through UE project gamemode bp to change if it's visible or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	
	// This UPROPERTY is tagged with an OnRep_Notify which flags it within the UE so anytime it's changed the server will be notified.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	// Creates a replicated Weapon variable that's sent to the server to update pick capabilities of weapons
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// When this UPROPERTY allows us to access functions/variables/information from the CombatComponent.h file
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	// This function tells the server what to do when the equip button is pressed.
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	// These 2 AO variables are the logic used in conjuction with BlasterAnimInstance.h/cpp's AO variables to update PC weapon angle information
	float AO_Yaw;
	float AO_Pitch;
	// Will be used to rotate root bone of PC while standing in place.
	float InterpAO_Yaw;
	// This stores our base aim rotation to update our weapons rotation within the animationbp
	FRotator StartingAimRotation;

	// The turning in place logic will be done in this class, then passed through a getter function to the animation instance
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

// This public is for getters and setters
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);

	// These 3 getters derive from CombatComponent.cpp
	bool IsWeaponEquip();
	bool IsAiming();

	// Getter that can be used on BlasterAnimInstance to find out BlasterCharacter's AimOffset information/logic (Make sure to call within a tick function)
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	
	AWeapon* GetEquippedWeapon();
	// This getter passes logic in TurningInPlace enum variable to other classes
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

};
