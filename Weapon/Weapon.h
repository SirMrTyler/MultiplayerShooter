// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")

};

UCLASS()
class WITCHYSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	
protected:
	virtual void BeginPlay() override;

	// This UFUNCTION provides the logic for when something overlaps with the weapons hitbox
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// This function provides the logic for when something stops overlapping with the weapons hitbox
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	// UPROPERTY allows manipulations of a weapons asset, features, and various parameters.
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	// UPROPERTY allows weapon to have a sphere for collision
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	// UPROPERTY gives PC access to various weapon states while a weapon is equipped. Such as ammo count, full auto vs semi auto, etc...
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	// Allows variable replication to server anytime the WeaponState variable is changed.
	UFUNCTION()
	void OnRep_WeaponState();

	// UPROPERTY allows a widget to appear above the weapon telling the PC what button to press to pick it up.
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;
public:	
	void SetWeaponState(EWeaponState State);
	// Used to pass collision sphere information to other classes
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere;}
	// Used to pass weaponmesh information to other classes
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh;} 
};
