// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Math/Vector.h"
#include "SG_TagPawn.generated.h"

UCLASS()
class SG_VR_MULTIPLAYER_API ASG_TagPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASG_TagPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Tag")
	FVector GetRCVector() const;

	UFUNCTION(BlueprintCallable, Category = "Tag")
	FVector GetLCVector() const;

	UFUNCTION(BlueprintCallable, Category = "Tag")
	FRotator GetHMDRotator() const;

	UFUNCTION(BlueprintCallable, Category = "Tag")
	FVector GetHMDVector() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera;
	
private:
	// VR Components

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* LeftController;
	
	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* RightController;

	FVector RC_Vector;

	FVector LC_Vector;
	
	FRotator HMD_Rotation;

	FVector HMD_Vector;
};
