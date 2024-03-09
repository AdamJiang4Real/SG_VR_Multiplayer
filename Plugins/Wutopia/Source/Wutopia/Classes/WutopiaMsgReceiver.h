#pragma once
#include <string.h>
#include<sstream>
#include <fstream> 
#include <vector>
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "Networking.h"
#include "Serialization/Archive.h"
#include "Json.h"
#include <thread>
#include <functional>
#include <iostream>
#include <queue>
#include "Interfaces/IPluginManager.h"
#include "WutopiaMsgDelegate.h"
//#include "viu_msg_sdk.h"
#include "WutopiaData.h"
#include "WutopiaMsgReceiver.generated.h"


UCLASS()
class WUTOPIA_API AWutopiaMsgReceiver : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Wutopia")
		void BPEvent_DataReceived();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		FWutopiaData GetData();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
	FString GetDebugData();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool StartWutopiaMsgReceiver(const FString& SocketName, const int32 msgPort, const int32 camStreamPort, const int32 lightStreamPort);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool IsStarted();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool IsConnected();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool IsMsgAvailable();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool IsDebugMsgAvailable();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastLoading(int status);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastCollide(int id, FString type, TMap<FString, FString> custom);


	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastClick(int id, FString type, TMap<FString, FString> custom, bool addMD5, FString signKey);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastScenes(TArray<FWutopiaIdNamePair> scenes);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastCurrentScene(FString scene);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastTansferPoints(TArray<FWutopiaIdNamePair> tpoints);
	
	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastPlaceholders(TArray<FWutopiaIdNamePair> placeholders);
	
	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastCams(TArray<FWutopiaIdNamePair> cams);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastTips(int id, FString msg);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastModals(int id, FString title, FString msg);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastProgress(int status);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastMinimap(FWutopiaMap loc);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastCustom(FString customMsg);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		void BroadcastCharacter(FWutopiaCharacterControl chrControl);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void msgRecving();

	
	FWutopiaData parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		FWutopiaData GetTagBagData();

	// 解析Debug Json消息 -Tag
	FString ParseJsonMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);
	
	FWutopiaData parseRequest(TSharedPtr<FJsonObject>);

	//cam stream
	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		bool gotNewCamStreamData();

	UFUNCTION(BlueprintCallable, Category = "Wutopia")
		FWutopiaCam getCamStreamData();

	void camStreamRecving();
	void parseCamMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	//light stream
	void lightStreamRecving();
	void parseLightMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	FSocket* ListenSocket;
	std::thread msgRecvThread;
	bool is_running;
	TArray<uint8> RecvBuffer;
	bool connected;
	std::queue<TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>> receivedMsgs;
	TQueue<TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>> ReceivedMsgs;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();

	FSocket* ListenCamSocket;
	std::thread camStreamRecvThread;
	TArray<uint8> camRecvBuffer;
	FWutopiaCam camStream;
	bool gotNewCamStream;

	FSocket* ListenLightSocket;
	std::thread lightStreamRecvThread;
	TArray<uint8> lightRecvBuffer;
	//FWutopiaLight lightStream;
	bool gotNewLightStream;

	FWutopiaData CurrentData;


	//ScopeLock
	FCriticalSection mutex;

	//whether use viumsgsdk
	bool useViu = false;

	//message SDK


	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	bool getUseViu();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static bool importDLL();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static bool importOnSendMsg();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static bool importOnStartConnect();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static bool importOnStopConnect();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static bool OnStartConnect();
	//UFUNCTION(BlueprintCallable, Category = "Wutopia")
	//	static void freeDLL();


	//int OnSendMsgFromDll(char* data, size_t size);
	//int OnStopConnect();
	//static void ViuMsgCallback(VIUMessageType, char*, size_t);
	//static TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> tmpData;

};