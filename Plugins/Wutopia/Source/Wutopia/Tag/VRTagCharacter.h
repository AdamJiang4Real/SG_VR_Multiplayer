// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "VRTagCharacter.generated.h"

UCLASS()
class WUTOPIA_API AVRTagCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRTagCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	// Sets the actors location taking into account the HMD as a pivot point, returns the location difference
	// If SetCapsuleLocation is true then it offsets the capsule to the location, otherwise it will move the Camera itself to the location
	UFUNCTION(BlueprintCallable, Category = "BaseVRCharacter|VRLocations")
	FVector SetActorLocationVR(FVector NewLoc, bool bTeleport, bool bSetCapsuleLocation = true);

	inline FVector GetVRLocation_Inline() const
	{
		return OffsetComponentToWorld.GetLocation();
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera;

	// This is the offset location of the player, use this for when checking against player transform instead of the actors transform
	UPROPERTY(BlueprintReadOnly, Transient, Category = "VRExpansionLibrary")
	FTransform OffsetComponentToWorld;

	// Returns the head location projected from the world offset (if applicable)
	virtual FVector GetProjectedVRLocation() const;
	
public:
	// VR Components

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* LeftController;
	
	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* RightController;
	
};
