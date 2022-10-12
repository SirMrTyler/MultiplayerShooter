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

// This function handles linking the functions associated with keybinds between our C++ and the UE editor
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

void ABlasterCharacter::AimButtonReleased()
{
	
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

// This function is a getter that passes whether or not a player has a weapon equipped
bool ABlasterCharacter::IsWeaponEquip()
{
	return (Combat && Combat->EquippedWeapon);
}

// This will be used to update our character weapon aiming location/rotation every frame
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// if PC doesn't have a weapon, this logic will not run and we will leave the function early
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	// We need to know whether the player character is moving or not because that will change which animation changes we're going to use.
	FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    float Speed = Velocity.Size();
	// This sets bIsInAir equal to a native unreal engine function that returns true when the player character is falling.
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	// Standing still, and not jumping
	if (Speed == 0.f && !bIsInAir)
	{

	}
	// PC is moving or jumping
	if (Speed < 0.f || bIsInAir)
	{

	}
}

// This is a getter function that returns whether or not a player is aiming.
bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

