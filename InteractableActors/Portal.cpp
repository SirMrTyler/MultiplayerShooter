
#include "Portal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WitchyShooter/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
    SetRootComponent(PortalMesh);
    PortalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComp->SetupAttachment(RootComponent);
	BoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComp->SetupAttachment(BoxComp);
	
    if (PortalParticles)
    {
        PortalParticlesComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(PortalParticles, PortalMesh, FName(TEXT("InnerParticles")), FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true, true);
    }

    bTeleporting = false;
}

void APortal::BeginPlay()
{
    Super::BeginPlay();
	
	

    if (HasAuthority())
    {
		BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        BoxComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBoxBeginOverlap);
        BoxComp->OnComponentEndOverlap.AddDynamic(this, &APortal::OnBoxEndOverlap);
    }
}

void APortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortal, bTeleporting);
}

void APortal::OnBoxBeginOverlap(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bTeleporting)
    {
        if (PortalEntranceSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, PortalEntranceSound, BoxComp->GetComponentLocation());
        }
    }
    else
    {
        if (PortalExitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, PortalExitSound, OtherPortal->GetActorLocation());
        }
    }

	if (OtherActor && OtherActor != this)
    {
        if (OtherPortal)
        {
            ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
            if (Character && !OtherPortal->bTeleporting)
            {
                bTeleporting = true;
                Character->SetActorRotation(OtherPortal->BoxComp->GetComponentRotation());
                Character->GetController()->SetControlRotation(Character->GetActorRotation());
                Character->SetActorLocation(OtherPortal->BoxComp->GetComponentLocation());
            }
        }
    }
}

void APortal::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
    {
    	if (OtherPortal && !bTeleporting)
        {
            OtherPortal->bTeleporting = false;
        }
    }
}

void APortal::EnterPortal(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor && OtherActor != this)
    {
        if (OtherPortal)
        {
            ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
            if (Character && !OtherPortal->bTeleporting)
            {
                bTeleporting = true;
                Character->SetActorRotation(OtherPortal->GetActorRotation());
                Character->GetController()->SetControlRotation(Character->GetActorRotation());
                Character->SetActorLocation(OtherPortal->GetActorLocation());
            }
        }
    }
}

void APortal::ExitPortal(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor && OtherActor != this)
    {
        if (OtherPortal && !bTeleporting)
        {
            OtherPortal->bTeleporting = false;
        }
    }
}