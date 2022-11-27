// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WitchyShooter/Weapon/Weapon.h"

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
    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
    bIsWeaponEquipped = BlasterCharacter->IsWeaponEquip(); 
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->IsAiming();
    
    EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
    TurningInPlace = BlasterCharacter->GetTurningInPlace();
    
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

    // Verifies weapon is equipped, what the weapon is, access to the weapons mesh, and access to the character's mesh
    if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
    {
        // We're beginning to attach the left hand to the weapons socket within the world space as named within the weapons SM (LeftHandSocket).
        LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        FVector OutPosition;
        FRotator OutRotation;
        // We're accessing the PC's right/left hands location/rotation relative to the right hands world space and storing this data in OutPosition and OutRotation
        BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
        // Here we take the data we stored in OutPosition and use it to set the location of PC's left hand
        LeftHandTransform.SetLocation(OutPosition);
        // Here we take the data we stored in OutRotation and use it to set the rotation of PC's left hand
        LeftHandTransform.SetRotation(FQuat(OutRotation)); // FQuat is a quaternion. A quaternion takes the quotient of two vectors within a 3d space (the 3d space being the PC's right hand).

        if (BlasterCharacter->IsLocallyControlled())
        {
            bLocallyControlled = true;
            FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
            RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
        }

    }
}

