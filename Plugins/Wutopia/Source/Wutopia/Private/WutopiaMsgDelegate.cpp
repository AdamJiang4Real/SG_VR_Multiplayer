// Fill out your copyright notice in the Description page of Project Settings.


#include "WutopiaMsgDelegate.h"

TMap<FString, FOnCustomMessage> UWutopiaoMsgDelegate::customMessageHanderMap;
FOnCharacterMessage UWutopiaoMsgDelegate::characterMessage;

void UWutopiaoMsgDelegate::DispatchMessage(FWutopiaData& data)
{
	if (data.type.IsEmpty()) {
		return;
	}
	if (data.type.Equals("custom")) 
	{
		if (!data.customType.IsEmpty() && customMessageHanderMap.Contains(data.customType))
		{
			if (customMessageHanderMap[data.customType].IsBound()) {
				customMessageHanderMap[data.customType].Execute(data.customStr);
			}
		}
	}
	else if (data.type.Equals("character"))
	{
		if (characterMessage.IsBound()) {
			characterMessage.Execute(data.chrControl);
		}
	}
}



void UWutopiaoMsgDelegate::SetCustomMsgDelegate(const FString& type,  FOnCustomMessage Delegate)
{
	if (Delegate.IsBound()) {
		if (!customMessageHanderMap.Contains(type)) {
			customMessageHanderMap.Emplace(type, Delegate);
		}
#if WITH_EDITOR
		else {
			if (!customMessageHanderMap[type].IsBound()) {
				customMessageHanderMap[type] = Delegate;
			}
		}
#endif
	}
}

void UWutopiaoMsgDelegate::SetCharacterMsgDelegate(FOnCharacterMessage Delegate)
{
	if (Delegate.IsBound()) {
		characterMessage = Delegate;
	}
}
