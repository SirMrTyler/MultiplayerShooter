
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class WITCHYSHOOTER_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnBoxBeginOverlap(
		class UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnBoxEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:	
	UPROPERTY(EditAnywhere, Category = "Portal Effects")
	class UStaticMeshComponent* PortalMesh;
	UPROPERTY(EditAnywhere, Category = "Portal Properties")
	class UBoxComponent* BoxComp;
	UPROPERTY(EditAnywhere, Category = "Portal Properties")
	class UArrowComponent* ArrowComp;
	
	UPROPERTY(EditAnywhere, Category = "Portal Effects")
	class UNiagaraSystem* PortalParticles;
	class UNiagaraComponent* PortalParticlesComponent;

	UFUNCTION()
	void EnterPortal(class AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void ExitPortal(AActor* OverlappedActor, AActor* OtherActor);

	UPROPERTY(EditAnywhere, Category = "Portal Properties")
	APortal* OtherPortal;

	UPROPERTY(EditAnywhere, Category = "Portal Effects")
	class USoundCue* PortalEntranceSound;
	UPROPERTY(EditAnywhere, Category = "Portal Effects")
	class USoundCue* PortalExitSound;

	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY(Replicated)
	bool bTeleporting;

};
