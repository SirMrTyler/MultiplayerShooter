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
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);

protected:
	virtual void BeginPlay() override;
	
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	void AimOffset(float DeltaTime);
	void JumpButtonPressed();
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	

	
private:
	// This UPROPERTY tells the Camera how far it needs to be from the PC
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// This is a widget used for testing who is a client, and who is the server. Modified through UE project gamemode bp to change if it's visible or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	// This UPROPERTY is tagged with an OnRep_Notify which flags it within the UE so anytime it's changed the server will be notified.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

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
	FRotator StartingAimRotation;
	// The turning in place logic will be done in this class, then passed through a getter function to the animation instance
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
// This public is for getters and setters
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquip();
	bool IsAiming();

	AWeapon* GetEquippedWeapon();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	

	

};
