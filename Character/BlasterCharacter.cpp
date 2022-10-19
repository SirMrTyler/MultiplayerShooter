// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h" // Used to replicate variables
#include "WitchyShooter/Weapon/Weapon.h"
#include "WitchyShooter/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

ABlasterCharacter::ABlasterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// We 
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

// Binds input functions to key presses
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABlasterCharacter::JumpButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &ABlasterCharacter::AimButtonReleased);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABlasterCharacter::LookUp);
}

// This function propells the player character forward in the x axis direction
void ABlasterCharacter::MoveForward(float AxisValue)
{
	if (Controller != nullptr && AxisValue != 0.f)
	{
		// We need an FRotator declared/defined so we know which way the PC is facing.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		// The local FVector Direction is used in conjuction with YawRotation in the x axis direction (or forward direction)
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		// This UE native function combines a rotator with a vector to move an actor (in this case it's our player character).
		AddMovementInput(Direction, AxisValue);
	}		
}

// This function does the same thing as the MoveForward() function but instead of in the x direction it moves the PC in the Y direction.
void ABlasterCharacter::MoveRight(float AxisValue)
{
	if (Controller != nullptr && AxisValue != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, AxisValue);
	}
}

// This function uses the axis value read by the mouse/right stick of the player to update the horizontal direction the PC is looking
void ABlasterCharacter::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

// This function does the same things as Turn() except in the vertical direction
void ABlasterCharacter::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

// This function uses UE's native jump function to make the PC jump.
void ABlasterCharacter::JumpButtonPressed()
{
	// This if statement allows uncrouching when pressing the jump button.
	if (bIsCrouched)
	{
		UnCrouch();
	}
	this->Jump();
}

// This function handles what happens when the keybind for crouch is pressed.
void ABlasterCharacter::CrouchButtonPressed()
{
	// If Statement Logic: If the PC is crouched they will stand up, if they are not crouched they will crouch.
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	// Add a main menu with the option of toggling/releasing aim
}

// This function handles what happens when the keybind for Aiming is pressed.
void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		// If the character is aiming; stop aiming. If the player isn't aiming; then the PC will aim.
		if (Combat->bAiming == false)
		{
			Combat->SetAiming(true);
			return;
		} 
		else
		{
			Combat->SetAiming(false);
			return;
		}
	}
}

// This function will be linked to a keybind within SetupPlayerInputComponent. When keybind is pressed weapon is attached to the player characters skeletal mesh.
void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

// This function replicates our EquipButtonPressed() feature so that the server knows what it needs to do and what to pass to each client
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

// This function will be called whenever a player is overlapping a weapon
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// If the PC is no longer overlapping a weapons Capsule Component stop displaying the widget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	// The logic statement says IsLocallyControlled() because we want to update this quickly on the client without having to wait for the server in case of lag/latency.
	if (IsLocallyControlled())
	{
		// When the PC's capsule component is colliding with a weapons capsule component we will display a widget on the screen stating which button to press to pick the weapon up.
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

// This function sends all of the data from our SetOverlappingWeapon() function to the server to be replicated.
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquip()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

// This will be used to update our character weapon aiming location/rotation every frame
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// if PC doesn't have a weapon, this logic will not run and we will leave the function early
	if (Combat && Combat->EquippedWeapon == nullptr) 
	{
		// We initialize StartingAimRotation when a weapon is unequipped so that when equipped the aim isn't set to the world zero (it's aiming in front of the PC like it's supposed to).
		return;
	}

	// We need to know whether the player character is moving or not because that will change which animation changes we're going to use.
	FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    float Speed = Velocity.Size();
	// This sets bIsInAir equal to a native unreal engine function that returns true when the player character is falling.
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// Standing still, and not jumping
	if (Speed == 0.f && !bIsInAir)
	{
		// We declare this FRotator so we can know where the PC is currently aiming
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// DeltaAimRotation is Declared so we know how far the weapon has to move to get back to the base aim position
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		// AO_Yaw is set every frame so the weapon moves as the PC moves their mouse
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		// bUseControllerRotationYaw is a native UE APawn class member that allows us to use the PC's mouse rotation adjustments when true or another source when false. When not moving use AO_Yaw not PC rotation.
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
		

	}
	// PC is moving or jumping
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// AO_Yaw stays zero while moving so that when we stop moving the weapon resets to it's base position 
		AO_Yaw = 0.f;
		// When moving we don't want the gun to sway so we're going to set controlrotation to the mouse input of PC
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	// We need to workaround UE's default Pitch/yaw compression for multiplayer (it turns pitch float into an unsigned int which cannot be negative)
	// This problem only affects other clients view of the locally controlled PC. So we need to say !LocallyControlled()
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360] to [-90, 0]
		FVector2D InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		// GetMappedRangeValueClamped() takes the InRange and converts it to the OutRange for the AO_Pitch variable.
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// This if statement passes turning right logic to the animation. The else statement passes turn left logic.
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	// This if statement rotates the character when turning in place.
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		// Here, we're checking if the character has turned enough. If so, the PC stops turning.
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			// Resets our aim after turning enough.
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	// Verify that combat isn't null. If it isn't, return the currently equipped weapon.
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

