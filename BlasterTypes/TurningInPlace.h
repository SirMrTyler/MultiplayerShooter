#pragma once

// We're creating a turning in place enum here rather than in BlasterCharacter.h/BlasterAnimInstance.h so that we don't have to include either one of those in another header file.
// This will help with optimization, compile times, and encapsulation.
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    ETIP_Left UMETA(DisplayName = "Turning Left"),
    ETIP_Right UMETA(DisplayName = "Turning Right"),
    ETIP_NotTurning UMETA(DisplayName = "Not Turning"),

    ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};