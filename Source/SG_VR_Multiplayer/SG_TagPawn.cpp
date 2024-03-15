// Fill out your copyright notice in the Description page of Project Settings.


#include "SG_TagPawn.h"

// Sets default values
ASG_TagPawn::ASG_TagPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	// Create motion controller components
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(Camera);
	
	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(Camera);

	RC_Vector = FVector::ZeroVector;
	LC_Vector= FVector::ZeroVector;
	
	HMD_Rotation = FRotator::ZeroRotator;
	HMD_Vector = FVector::ZeroVector;
	
}

// Called when the game starts or when spawned
void ASG_TagPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASG_TagPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 获取左手柄的位置和旋转信息
	FVector LeftControllerLocation = LeftController->GetComponentLocation();
	FRotator LeftControllerRotation = LeftController->GetComponentRotation();
	
	
	// 获取右手柄的位置和旋转信息
	FVector RightControllerLocation = RightController->GetComponentLocation();
	FRotator RightControllerRotation = RightController->GetComponentRotation();

	GetHMDVector();
	
	

}

// Called to bind functionality to input
void ASG_TagPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector ASG_TagPawn::GetRCVector() const
{
	return RC_Vector;
}

FVector ASG_TagPawn::GetLCVector() const
{
	return LC_Vector;
}

FRotator ASG_TagPawn::GetHMDRotator() const
{
	return HMD_Rotation;
}

FVector ASG_TagPawn::GetHMDVector() const
{

	return FVector::ZeroVector;
}



