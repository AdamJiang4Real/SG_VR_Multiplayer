// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WutopiaData.h"
#include "WutopiaMsgDelegate.generated.h"

//DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCustomMessage, FString, data);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCharacterMessage, FWutopiaCharacterControl, data);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WUTOPIA_API UWutopiaoMsgDelegate : public UObject
{
	GENERATED_BODY()

private:

	static TMap<FString, FOnCustomMessage> customMessageHanderMap;

	static FOnCharacterMessage characterMessage;
public:	

	/**
	 * 派发监听的消息协议
	 */
	static void DispatchMessage(FWutopiaData& data);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetCustomMessageDelegate"))
	static void SetCustomMsgDelegate(const FString& type, FOnCustomMessage Delegate);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetCharacterMessageDelegate"))
	static void SetCharacterMsgDelegate(FOnCharacterMessage Delegate);
};
