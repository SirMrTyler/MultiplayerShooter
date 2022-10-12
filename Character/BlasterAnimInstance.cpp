// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

// UE native function used to update animations every frame
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (BlasterCharacter == nullptr)
    {
        // States who is the active pawn controller of the BlasterCharacter
        BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
    }
    if (BlasterCharacter == nullptr) return; // if no one controlls BlasterCharacter, return from this function early; before other logic is completed.

    // Updates speed variable to pass to AnimBP each frame
    FVector Velocity = BlasterCharacter->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    // Updates variables passed every frame to the AnimBP to update PC statuses
    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling(); // Changes anim based on whether the PC is in air or not
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; // States whether the player is moving or not
    bWeaponEquipped = BlasterCharacter->IsWeaponEquip(); // Here we set bWeaponEquipped equal to a getter function in our Char.h that pulls the equipped weapon status of the player from CombatComponent
    bIsCrouched = BlasterCharacter->bIsCrouched; // Is PC crouching or not
    bAiming = BlasterCharacter->IsAiming(); // Here we set bAiming equal to a getter function in our Char.h that pulls the status of an aiming bull declared in CombatComponent

    // Offset Yaw for Strafing
    FRotator AimRotation = BlasterCharacter->GetBaseAimRotation(); // This gives us the relative global rotation
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity()); // This gives us the relative global rotational movement direction of our character mesh
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
    YawOffset = DeltaRotation.Yaw;

    // This block of code allows the PC to lean when their mouse is moved quickly providing a smoother, more natural animation transition.
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = BlasterCharacter->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target = Delta.Yaw / DeltaTime;
    // FInterpTo returns a float every frame. It works like so FInterpTo(CurrentFrameValue, FinalValue, TickTime, SpeedToGetToFinalValue)
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
    // This causes the lean to only change slightly (between a min and max value of -90, and 90). This is so that the PC doesn't change in unrealistic amount
    Lean = FMath::Clamp(Interp, -90.f, 90.f);

    // Updates AnimationOffset data every frame according to logic performed within BlasterCharacter.cpp
    AO_Yaw = BlasterCharacter->GetAO_Yaw();
    AO_Pitch = BlasterCharacter->GetAO_Pitch();
}
