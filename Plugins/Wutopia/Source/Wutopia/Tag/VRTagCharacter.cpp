// Fill out your copyright notice in the Description page of Project Settings.


#include "VRTagCharacter.h"


// Sets default values
AVRTagCharacter::AVRTagCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	// Create motion controller components
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(Camera);
	
	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(Camera);
	

	OffsetComponentToWorld = OffsetComponentToWorld = FTransform(FQuat(0.0f, 0.0f, 0.0f, 1.0f), FVector::ZeroVector, FVector(1.0f));
}

// Called when the game starts or when spawned
void AVRTagCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRTagCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVRTagCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FVector AVRTagCharacter::SetActorLocationVR(FVector NewLoc, bool bTeleport, bool bSetCapsuleLocation)
{
	FVector NewLocation;
	FRotator NewRotation;
	FVector PivotOffsetVal = (bSetCapsuleLocation ? GetVRLocation_Inline() : GetProjectedVRLocation()) - GetActorLocation();
	PivotOffsetVal.Z = 0.0f;


	NewLocation = NewLoc - PivotOffsetVal;// +PivotPoint;// NewRotation.RotateVector(PivotPoint);
	//NewRotation = NewRot;


	// Also setting actor rot because the control rot transfers to it anyway eventually
	SetActorLocation(NewLocation, false, nullptr, bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::None);
	return NewLocation - NewLoc;
}

FVector AVRTagCharacter::GetProjectedVRLocation() const
{
	return GetVRLocation_Inline();
}

