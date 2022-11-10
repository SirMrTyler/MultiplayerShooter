// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class WITCHYSHOOTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float EjectionImpulseMin;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float EjectionImpulseMax;

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

	UPROPERTY(EditAnywhere)
	float DestroyDelay;
};
