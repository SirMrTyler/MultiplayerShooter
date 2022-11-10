// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	
	EjectionImpulseMin = 5.5f;
	EjectionImpulseMax = 6.5f;

	DestroyDelay = 3.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);

	ShellEjectionImpulse = FMath::RandRange(EjectionImpulseMin, EjectionImpulseMax);
	FVector CasingEjectionDirection = GetActorForwardVector();
	CasingEjectionDirection.X += FMath::RandRange(-1.f, 1.f);
	CasingEjectionDirection.Z += FMath::RandRange(-0.5f, 0.5f);
	
	CasingMesh->AddTorqueInRadians(GetActorRightVector() * 100000.f);
	CasingMesh->AddImpulse(CasingEjectionDirection * ShellEjectionImpulse);	

	SetLifeSpan(DestroyDelay);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	CasingMesh->SetNotifyRigidBodyCollision(false);
}



