#pragma once

#include "GameFramework/Actor.h"
#include "Engine.h"
#include "Json.h"
#include "Networking.h"
#include <thread>
#include <iostream>
#include <functional>
#include "WutopiaData.h"
#include "WutopiaHumanStreamMsgReceiver.generated.h"

#define RECV_BUFFER_SIZE 1024*12

UCLASS()
class WUTOPIA_API AWutopiaHumanStreamMsgReceiver : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Wutopia")
		void BPEvent_DataReceived();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		FWutopiaBodyFrameData GetBodyData();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool StartWutopiaBodyMsgReceiver(const FString& SocketName,
			const int32 Port);

	void bodyMsgRecving();

	void HandleBodyStreamData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	void HandleBodyData(TSharedPtr<FJsonObject> JsonObject);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		FWutopiaFaceFrameData GetFaceData();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool StartWutopiaFaceMsgReceiver(const FString& SocketName,
			const int32 Port);

	void faceMsgRecving();

	void HandleFaceStreamData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	void HandleFaceData(TSharedPtr<FJsonObject> JsonObject);

	FTimespan WaitTime;
	bool is_running;
	
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();

	//body specific
	FWutopiaBodyFrameData bodyFrameData;
	FSocket* bodyListenSocket;
	TArray<uint8> bodyRecvBuffer;
	std::thread bodyMsgRecvThread;
	bool got_new_body_data;
	TArray<FName> BoneNamesCur;
	int numPosesCur;
	
	//face specific
	FWutopiaFaceFrameData faceFrameData;
	FSocket* faceListenSocket;
	TArray<uint8> faceRecvBuffer;
	std::thread faceMsgRecvThread; 
	bool got_new_face_data;
	TArray<FName> BSNamesCur;
	int numBsCur;
};