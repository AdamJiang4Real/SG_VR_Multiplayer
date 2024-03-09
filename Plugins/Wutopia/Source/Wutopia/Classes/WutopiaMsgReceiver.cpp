#include "WutopiaMsgReceiver.h"
#include "Misc/ScopeLock.h"

#define BUFFER_SIZE 1024 //*16

//typedef bool(*_on_send_msg)(char* data, size_t size);
//typedef bool(*_on_start_connect)(OnViuMessageCallback callback, int reconnection_interval);
//typedef bool(*_on_stop_connect)();
//
//_on_send_msg m_on_send_msgFromDll;
//_on_start_connect m_on_start_connectFromDll;
//_on_stop_connect m_on_stop_connectFromDll;
//void* v_dllHandle;
//std::queue<TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>> receivedMsgsViu;
//bool viuConnected;

AWutopiaMsgReceiver::AWutopiaMsgReceiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ListenSocket = NULL;
	is_running = false;
	RecvBuffer.SetNumUninitialized(BUFFER_SIZE);
	connected = false;

	ListenCamSocket = NULL;
	camRecvBuffer.SetNumUninitialized(BUFFER_SIZE);
	gotNewCamStream = false;

	ListenLightSocket = NULL;
	lightRecvBuffer.SetNumUninitialized(BUFFER_SIZE);
	gotNewLightStream = false;

	FString configDir = IPluginManager::Get().FindPlugin("Wutopia")->GetBaseDir() + "/Config/viu.txt";
	
	//std::string path(TCHAR_TO_UTF8(*configDir));
	//std::ifstream cfile(path);
	//std::string viu;
	//getline(cfile, viu);
	//cfile.close();
	//if (viu == "1")
	//{
	//	useViu = true;
	//}
	//viuConnected = false;
	//v_dllHandle = NULL;
}

bool AWutopiaMsgReceiver::StartWutopiaMsgReceiver(const FString& SocketName, const int32 msgPort, const int32 camStreamPort, const int32 lightStreamPort)
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, msgPort);
	int32 BufferSize = 2 * 1024; //* 1024;
	ListenSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	is_running = true;
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(5);
	msgRecvThread = std::thread(std::bind(&AWutopiaMsgReceiver::msgRecving, this));
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("useViu %d"),useViu));
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, IPluginManager::Get().FindPlugin("Wutopia")->GetBaseDir() + "/Config/viu.txt");
	if (camStreamPort != -1)
	{
		FIPv4Endpoint EndpointCam(FIPv4Address::Any, camStreamPort);
		ListenCamSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
			.AsReusable()
			.BoundToEndpoint(EndpointCam)
			.WithReceiveBufferSize(BufferSize);
		camStreamRecvThread = std::thread(std::bind(&AWutopiaMsgReceiver::camStreamRecving, this));
	}

	if (lightStreamPort != -1)
	{
		FIPv4Endpoint EndpointLight(FIPv4Address::Any, lightStreamPort);
		ListenLightSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
			.AsReusable()
			.BoundToEndpoint(EndpointLight)
			.WithReceiveBufferSize(BufferSize);
		lightStreamRecvThread = std::thread(std::bind(&AWutopiaMsgReceiver::lightStreamRecving, this));
	}
	return true;
}

bool AWutopiaMsgReceiver::IsStarted()
{
	return is_running;
}

void AWutopiaMsgReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	is_running = false;

	//Clear all sockets
	if (ListenSocket)
	{
		msgRecvThread.join();
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}

	if (ListenCamSocket)
	{
		camStreamRecvThread.join();
		ListenCamSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenCamSocket);
	}

	if (ListenLightSocket)
	{
		lightStreamRecvThread.join();
		ListenLightSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenLightSocket);
	}
}

void AWutopiaMsgReceiver::msgRecving()
{
	while (is_running)
	{
		int32 Read = 0;
		ListenSocket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender);
		if (Read > 0)
		{
			connected = true;
			TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
			ReceivedData->SetNumUninitialized(Read);
			memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
			{
				FScopeLock ScopeLock(&mutex);
				receivedMsgs.push(ReceivedData);
				// ReceivedMsgs.Enqueue(ReceivedData);
			}
			//parseMsg(ReceivedData);

		}

		// FPlatformProcess::Sleep(0.02f);
	}
}

bool AWutopiaMsgReceiver::IsConnected()
{
	//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, FString::Printf(TEXT("%d,%d"), connected, viuConnected));
	return connected;

	//return (connected || viuConnected);
}

bool AWutopiaMsgReceiver::IsMsgAvailable()
{
	return !receivedMsgs.empty();

	//if (useViu)
	//{
	//	return !receivedMsgsViu.empty();
	//}
	//else
	//{
	//	return !receivedMsgs.empty();
	//}
}

bool AWutopiaMsgReceiver::IsDebugMsgAvailable()
{
	return !ReceivedMsgs.IsEmpty();
}

FWutopiaData AWutopiaMsgReceiver::GetData()
{
	FWutopiaData data;

	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
	
	if (!receivedMsgs.empty())
	{
		{
			FScopeLock ScopeLock(&mutex);

			ReceivedData = receivedMsgs.front();
			receivedMsgs.pop();

		}
		data = parseMsg(ReceivedData);

		// UE_LOG(LogTemp, Warning, TEXT("tid is %s"), *data.TagBag.tid);
		
		//派发消息
		// UWutopiaoMsgDelegate::DispatchMessage(data);
	}
	return data;
}

FString AWutopiaMsgReceiver::GetDebugData()
{
	FString Str;

	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());

	{
		FScopeLock ScopeLock(&mutex);

		if (!ReceivedMsgs.IsEmpty())
		{
			{
				ReceivedMsgs.Dequeue(ReceivedData);
			}
			Str = ParseJsonMsg(ReceivedData);
		
			return Str;
		}
		
	}
	
	return "NOT VALID";
}

FWutopiaData AWutopiaMsgReceiver::parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FWutopiaData data;


	TArray<uint8> jsonBytes;
	for (uint8& Byte : *ReceivedData.Get())
	{
		jsonBytes.Add(Byte);
	}
	FUTF8ToTCHAR Convert(reinterpret_cast<const char*>(jsonBytes.GetData()), jsonBytes.Num());
	FString JsonString(Convert);

	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, JsonString);
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (flag)  // StringתJSON
	{
		FString str;
		if (JsonObject->TryGetStringField(TEXT("mt"), str))
		{
			if (str == "rq")
			{
				data = parseRequest(JsonObject);
				
				return data;

				
			}
			else // not request add log
			{
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not a request msg");
				return data;
			}
		}
		else // no mt field  add log
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not a valid msg, no mt field");
			return data;
		}
		return  data;;
	}
	else  // not json add warning log
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not a valid msg, ignore");
		return data;
	}
}

FString AWutopiaMsgReceiver::ParseJsonMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FWutopiaData data;

	TArray<uint8> jsonBytes;
	for (uint8& Byte : *ReceivedData.Get())
	{
		jsonBytes.Add(Byte);
	}
	FUTF8ToTCHAR Convert(reinterpret_cast<const char*>(jsonBytes.GetData()), jsonBytes.Num());
	FString JsonString(Convert);

	// GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, JsonString);
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	return JsonString;
}

FWutopiaData AWutopiaMsgReceiver::parseRequest(TSharedPtr<FJsonObject> JsonObject)
{
	FString str;
	const TSharedPtr<FJsonObject>* jsonData;
	const TSharedPtr<FJsonObject>* appearanceData;
	FWutopiaData data;
	if (JsonObject->TryGetStringField(TEXT("dt"), str))
	{
		data.type = str;
		if (JsonObject->TryGetObjectField(TEXT("da"), jsonData))
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, str);
			if (str == "knapsack")
			{
				FString key;
				(*jsonData)->TryGetStringField(TEXT("key"), key);
				data.key = key;
			}
			else if (str == "ability")
			{
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "asdasdasd");
				FString key;
				(*jsonData)->TryGetStringField(TEXT("key"), key);
				data.key = key;
			}
			else if (str == "cam")
			{
				FWutopiaCam cam;
				double pan;
				double tile;
				double x;
				double y;
				double z;
				double zoom;
				double focus;
				(*jsonData)->TryGetStringField(TEXT("id"), cam.id);
				(*jsonData)->TryGetNumberField(TEXT("pan"), pan);
				(*jsonData)->TryGetNumberField(TEXT("tile"), tile);
				(*jsonData)->TryGetNumberField(TEXT("x"), x);
				(*jsonData)->TryGetNumberField(TEXT("y"), y);
				(*jsonData)->TryGetNumberField(TEXT("z"), z);
				(*jsonData)->TryGetNumberField(TEXT("zoom"), zoom);
				(*jsonData)->TryGetNumberField(TEXT("focus"),focus);

				cam.pan = pan;
				cam.tilt = tile;
				cam.x = x;
				cam.y = y;
				cam.z = z;
				cam.zoom = zoom;
				cam.focus = focus;

				data.cam = cam;
			}
			else if (str == "tpoint")
			{
				FString id;
				(*jsonData)->TryGetStringField(TEXT("id"), id);
				data.id = id;
			}
			else if (str == "scenes")
			{
				FWutopiaDS ds;
				(*jsonData)->TryGetStringField(TEXT("server"), ds.server);
				(*jsonData)->TryGetNumberField(TEXT("port"), ds.port);
				data.ds = ds;
			}
			else if (str == "place") // TBD  support array
			{
				FString id;
				(*jsonData)->TryGetStringField(TEXT("id"), id);
				data.id = id;

				FString value;
				(*jsonData)->TryGetStringField(TEXT("value"), value);
				data.value = value;
			}
			else if (str == "character")
			{
				FWutopiaCharacterControl chrControl;
				if (!(*jsonData)->TryGetNumberField(TEXT("speed"), chrControl.speed))
				{
					chrControl.speed = -999;
					GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("speed %d, "), chrControl.speed));
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("type"), chrControl.type))
				{
					chrControl.type = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("hair"), chrControl.hair))
				{
					chrControl.hair = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("blouse"), chrControl.blouse))
				{
					chrControl.blouse = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("pant"), chrControl.pant))
				{
					chrControl.pant = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("shoe"), chrControl.shoe))
				{
					chrControl.shoe = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("face"), chrControl.face))
				{
					chrControl.face = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("glass"), chrControl.glass))
				{
					chrControl.glass = -999;
				}
				if (!(*jsonData)->TryGetNumberField(TEXT("prop"), chrControl.prop))
				{
					chrControl.prop = -999;
				}

				chrControl.appearance.valid = 0;
				if ((*jsonData)->TryGetObjectField(TEXT("appearance"), appearanceData))
				{
					(*appearanceData)->TryGetStringField(TEXT("type"), chrControl.appearance.type);
					(*appearanceData)->TryGetStringField(TEXT("subtype"), chrControl.appearance.subtype);
					(*appearanceData)->TryGetNumberField(TEXT("ID"), chrControl.appearance.ID);
					chrControl.appearance.valid = 1;
				}


				data.chrControl = chrControl;
				//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT(" %d, %d, %d,%d,%d,%d "),blouse,shoe, face));
			}
			else if (str == "map")
			{
				int freq;
				(*jsonData)->TryGetNumberField(TEXT("freq"), freq);
				data.minimapFreq = freq;
			}
			else if (str == "navigation")
			{
				FWutopiaPoint point;
				double x;
				double y;
				(*jsonData)->TryGetNumberField(TEXT("x"), x);
				(*jsonData)->TryGetNumberField(TEXT("y"), y);
				point.x = x;
				point.y = y;
				data.point = point;
			}
			else if (str == "TagBag")
			{
				FWutopiaTagBag TagBag;
				FString tid;
				float x;
				float y;
				float z;
				float t;
				(*jsonData)->TryGetStringField(TEXT("tid"), tid);
				(*jsonData)->TryGetNumberField(TEXT("x"), x);
				(*jsonData)->TryGetNumberField(TEXT("y"), y);
				(*jsonData)->TryGetNumberField(TEXT("z"), z);
				(*jsonData)->TryGetNumberField(TEXT("z"), z);
				(*jsonData)->TryGetNumberField(TEXT("t"), t);
				
				TagBag.tid = tid;
				TagBag.x = x;
				TagBag.y = y;
				TagBag.z = z;
				TagBag.t = t;
				data.TagBag = TagBag;
			}
			else if (str == "custom")
			{
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "asdasdasd");
				FString customStr;
				FString customType;
				if (!(*jsonData)->TryGetStringField(TEXT("type"), customType))
				{
					GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, customType);
				}

				TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&customStr);
				bool flag = FJsonSerializer::Serialize((*jsonData).ToSharedRef(), JsonWriter);
				//FJsonSerializer::Serialize()

				////customStr = (*customStrData)->
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, customStr);
				data.customType = customType;
				data.customStr = customStr;
			}
			else if (str == "preLoading")
			{
				FWutopiaInitParams par;
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "12121");
				FString username;
				FString uuid;
				FString IP;
				int port;
				int showLog;
				int comp;

				(*jsonData)->TryGetStringField(TEXT("username"), username);
				(*jsonData)->TryGetStringField(TEXT("uuid"), uuid);
				(*jsonData)->TryGetStringField(TEXT("IP"), IP);
				(*jsonData)->TryGetNumberField(TEXT("port"), port);
				(*jsonData)->TryGetNumberField(TEXT("showLog"), showLog);
				(*jsonData)->TryGetNumberField(TEXT("comp"), comp);

				par.username = username;
				par.uuid = uuid;
				par.IP = IP;
				par.port = port;
				par.showLog = showLog;
				par.comp = comp;

				data.initParams = par;
			}
			else if (str == "heartbeat")
			{
				//GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, customStr);
			}
			
			else // not supported log
			{
				GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not found");
			}

			TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
			jsonObject->SetStringField(TEXT("mt"), "ack");
			jsonObject->SetStringField(TEXT("type"), str);
			
			FString jsonStr = TEXT("");
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
			FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

			const TCHAR* StrPtr = *jsonStr;
			FTCHARToUTF8 UTF8String(StrPtr);
			int32 len = UTF8String.Length();
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);



			return data;
		}
		else // no data log
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not a valid msg, no da field");
			return data;
		}
	}
	else // no dt log
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "not a valid msg, no dt field");
		return data;
	}
	return data;
}

void AWutopiaMsgReceiver::BroadcastLoading(int status)
{
	if (connected)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "loading");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("status"), status);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

//			OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();
	
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastCollide(int id, FString type, TMap<FString, FString> custom)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "collide");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("id"), id);
		
		TSharedPtr<FJsonObject> jsonDataValue = MakeShareable(new FJsonObject);
		jsonDataValue->SetStringField(TEXT("type"), type);
		for (auto Iter = custom.CreateIterator(); Iter; ++Iter)
		{
			FString key = Iter->Key;
			FString value = Iter->Value;
			jsonDataValue->SetStringField(key, value);
		}

		jsonData->SetObjectField(TEXT("value"), jsonDataValue);


		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

			//OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastClick(int id, FString type, TMap<FString, FString> custom, bool addMD5, FString signKey)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "click");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("id"), id);

		TSharedPtr<FJsonObject> jsonDataValue = MakeShareable(new FJsonObject);
		jsonDataValue->SetStringField(TEXT("type"), type);
		for (auto Iter = custom.CreateIterator(); Iter; ++Iter)
		{
			FString key = Iter->Key;
			FString value = Iter->Value;
			jsonDataValue->SetStringField(key, value);
		}

		if (addMD5)
		{
			int64 timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000 + FDateTime::Now().GetMillisecond();
			jsonDataValue->SetNumberField(TEXT("timestamp"), timestamp);

			//FString signKey = "7bcuIsr5tfRiwm114i0OVyzZnUzBZZqx";
			FString sign = signKey + "+" + FString::FromInt(id) + "+" + FString::Printf(TEXT("%lld"), timestamp);
			FString md5Sign = FMD5::HashAnsiString(*sign);
			jsonDataValue->SetStringField(TEXT("md5"), md5Sign);
		}

		jsonData->SetObjectField(TEXT("value"), jsonDataValue);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();

		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastScenes(TArray<FWutopiaIdNamePair> scenes)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "scenes");


		TArray<TSharedPtr<FJsonValue>> jsonData;
		for (int i = 0; i < scenes.Num(); i++)
		{
			TSharedPtr<FJsonObject> scene = MakeShareable(new FJsonObject);
			scene->SetNumberField("id", scenes[i].id);
			scene->SetStringField("name", scenes[i].name);
			jsonData.Add(MakeShareable(new FJsonValueObject(scene)));
		}
		jsonObject->SetArrayField("da", jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastCurrentScene(FString scene)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "currentScene");

		jsonObject->SetStringField("da", scene);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastTips(int id, FString msg)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "tips");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("id"), id);
		jsonData->SetStringField(TEXT("message"), msg);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastModals(int id, FString title, FString msg)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "modals");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("id"), id);
		jsonData->SetStringField(TEXT("title"), title);
		jsonData->SetStringField(TEXT("message"), msg);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastProgress(int status)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "progress");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		jsonData->SetNumberField(TEXT("status"), status);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastMinimap(FWutopiaMap loc)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "map");

		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);
		double x = loc.x;
		double y = loc.y;
		double z = loc.z;
		double yaw = loc.yaw;
		double fov = loc.fov;
		int valid = loc.valid;
		jsonData->SetNumberField(TEXT("x"), x);
		jsonData->SetNumberField(TEXT("y"), y);
		jsonData->SetNumberField(TEXT("z"), z);  
		jsonData->SetNumberField(TEXT("yaw"), yaw);
		jsonData->SetNumberField(TEXT("fov"), fov);
		jsonData->SetNumberField(TEXT("valid"), valid);

		jsonObject->SetObjectField(TEXT("da"), jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastTansferPoints(TArray<FWutopiaIdNamePair> tpoints)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "tpoint");


		TArray<TSharedPtr<FJsonValue>> jsonData;
		for (int i = 0; i < tpoints.Num(); i++)
		{
			TSharedPtr<FJsonObject> tpoint = MakeShareable(new FJsonObject);
			tpoint->SetNumberField("id", tpoints[i].id);
			tpoint->SetStringField("name", tpoints[i].name);
			jsonData.Add(MakeShareable(new FJsonValueObject(tpoint)));
		}
		jsonObject->SetArrayField("da", jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		//int len = jsonStr.Len();
		int32 sent = 0;
		TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
		
		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		

		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastPlaceholders(TArray<FWutopiaIdNamePair> placeholders)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "place");


		TArray<TSharedPtr<FJsonValue>> jsonData;
		for (int i = 0; i < placeholders.Num(); i++)
		{
			TSharedPtr<FJsonObject> placeholder = MakeShareable(new FJsonObject);
			placeholder->SetNumberField("id", placeholders[i].id);
			placeholder->SetStringField("name", placeholders[i].name);
			jsonData.Add(MakeShareable(new FJsonValueObject(placeholder)));
		}
		jsonObject->SetArrayField("da", jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastCams(TArray<FWutopiaIdNamePair> cams)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "cams");


		TArray<TSharedPtr<FJsonValue>> jsonData;
		for (int i = 0; i < cams.Num(); i++)
		{
			TSharedPtr<FJsonObject> cam = MakeShareable(new FJsonObject);
			cam->SetNumberField("id", cams[i].id);
			cam->SetStringField("name", cams[i].name);
			jsonData.Add(MakeShareable(new FJsonValueObject(cam)));
		}
		jsonObject->SetArrayField("da", jsonData);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastCustom(FString customMsg)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "custom");

		TSharedPtr<FJsonObject> JsonObjectCustom;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(customMsg);
		bool flag = FJsonSerializer::Deserialize(Reader, JsonObjectCustom);

		jsonObject->SetObjectField(TEXT("da"), JsonObjectCustom);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

void AWutopiaMsgReceiver::BroadcastCharacter(FWutopiaCharacterControl chrControl)
{
	if (connected || useViu)
	{
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetStringField(TEXT("mt"), "bd");
		jsonObject->SetStringField(TEXT("dt"), "character");


		TSharedPtr<FJsonObject> jsonData = MakeShareable(new FJsonObject);

		jsonData->SetNumberField(TEXT("speed"), chrControl.speed);
		jsonData->SetNumberField(TEXT("type"), chrControl.type);
		jsonData->SetNumberField(TEXT("hair"), chrControl.hair);
		jsonData->SetNumberField(TEXT("blouse"), chrControl.blouse);
		jsonData->SetNumberField(TEXT("pant"), chrControl.pant);
		jsonData->SetNumberField(TEXT("shoe"), chrControl.shoe);
		jsonData->SetNumberField(TEXT("prop"), chrControl.prop);

		jsonObject->SetObjectField(TEXT("da"), jsonData);


		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		const TCHAR* StrPtr = *jsonStr;
		FTCHARToUTF8 UTF8String(StrPtr);
		int32 len = UTF8String.Length();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);

		if (useViu)
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();

		//	OnSendMsgFromDll(TCHAR_TO_UTF8(SendMessage), len);
		}
		else
		{
			int32 sent = 0;
			TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
			int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
			uint32 out;
			Sender->GetIp(out);
			int port = Sender->GetPort();

			// GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
		}
	}
}

//for cam stream
void AWutopiaMsgReceiver::camStreamRecving()
{
	while (is_running)
	{
		int32 Read = 0;
		ListenCamSocket->RecvFrom(camRecvBuffer.GetData(), camRecvBuffer.Num(), Read, *Sender);
		if (Read > 0)
		{
			connected = true;
			TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
			ReceivedData->SetNumUninitialized(Read);
			memcpy(ReceivedData->GetData(), camRecvBuffer.GetData(), Read);
			parseCamMsg(ReceivedData);
		}
	}
}

void AWutopiaMsgReceiver::parseCamMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FWutopiaData data;

	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (flag)  // StringתJSON
	{
		FString strMt;
		FString strDt;
		const TArray<TSharedPtr<FJsonValue>>* tmpArray;
		if (JsonObject->TryGetStringField(TEXT("mt"), strMt) && JsonObject->TryGetStringField(TEXT("mt"), strDt))
		{
			if (strMt == "st" && strDt == "cam")
			{
				
				if (JsonObject->TryGetArrayField(TEXT("da"), tmpArray))
				{
					TArray<TSharedPtr<FJsonValue>> arrayVal = JsonObject->GetArrayField(TEXT("da"));
					
					FWutopiaCam cam;
					cam.id = arrayVal[0]->AsString();
					cam.pan = arrayVal[1]->AsNumber();
					cam.tilt = arrayVal[1]->AsNumber();
					cam.x = arrayVal[1]->AsNumber();
					cam.y = arrayVal[1]->AsNumber();
					cam.z = arrayVal[1]->AsNumber();
					cam.zoom = arrayVal[1]->AsNumber();
					cam.focus = arrayVal[1]->AsNumber();

					gotNewCamStream = true;
					camStream = cam;
				}
			}
			else // not stream add log
			{
				
			}
		}
		else // no mt field  add log
		{
			
		}
	}
	else  // not json add warning log
	{
	}
}

bool AWutopiaMsgReceiver::gotNewCamStreamData()
{
	return gotNewCamStream;
}

FWutopiaCam AWutopiaMsgReceiver::getCamStreamData()
{
	gotNewCamStream = false;
	return camStream;
}

//for light stream
void AWutopiaMsgReceiver::lightStreamRecving()
{
	while (is_running)
	{
		int32 Read = 0;
		ListenLightSocket->RecvFrom(lightRecvBuffer.GetData(), lightRecvBuffer.Num(), Read, *Sender);
		if (Read > 0)
		{
			connected = true;
			TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
			ReceivedData->SetNumUninitialized(Read);
			memcpy(ReceivedData->GetData(), lightRecvBuffer.GetData(), Read);
			parseLightMsg(ReceivedData);
		}
	}
}

void AWutopiaMsgReceiver::parseLightMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FWutopiaData data;

	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (flag)  // StringתJSON
	{
		FString strMt;
		FString strDt;
		//const TArray<TSharedPtr<FJsonValue>>* tmpArray;
		if (JsonObject->TryGetStringField(TEXT("mt"), strMt) && JsonObject->TryGetStringField(TEXT("mt"), strDt))
		{
			if (strMt == "st" && strDt == "light")
			{
				//TBD
			}
			else // not stream add log
			{

			}
		}
		else // no mt field  add log
		{

		}
	}
	else  // not json add warning log
	{
	}
}


//
//bool AWutopiaMsgReceiver::getUseViu()
//{
//	return useViu;
//}
//
//void AWutopiaMsgReceiver::ViuMsgCallback(VIUMessageType type, char* data, size_t size)
//{
//	if (type == CONNECTED)
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, "connect msg");
//	}
//	else if (type == ACK_MSGDATA)
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, "ack msg");
//	}
//	else if (type == USR_MSGDATA)
//	{
//		TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
//		ReceivedData->SetNumUninitialized(size);
//		memcpy(ReceivedData->GetData(), data, size);
//		receivedMsgsViu.push(ReceivedData);
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, "user msg");
//	}
//	viuConnected = true;
//	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, FString(UTF8_TO_TCHAR(data)));
//}
//
//#pragma region Load DLL
//
//// Method to import a DLL.
//bool AWutopiaMsgReceiver::importDLL()
//{
//	FString filePath = IPluginManager::Get().FindPlugin("Wutopia")->GetBaseDir() + "/Source/Wutopia/ThirdParty/bin/viumsgsdk.dll";
//	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, filePath);
//	if (FPaths::FileExists(filePath))
//	{
//		v_dllHandle = FPlatformProcess::GetDllHandle(*filePath); // Retrieve the DLL.
//		if (v_dllHandle != NULL)
//		{
//			return true;
//		}
//	}
//
//	return false;	// Return an error.
//}
//#pragma endregion Load DLL
//
//
//#pragma region Import Methods
//// Imports the method getInvertedBool from the DLL.
//bool AWutopiaMsgReceiver::importOnSendMsg()
//{
//	if (v_dllHandle != NULL)
//	{
//		m_on_send_msgFromDll = NULL;
//		FString procName = "on_send_msg";	// Needs to be the exact name of the DLL method.
//		m_on_send_msgFromDll = (_on_send_msg)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
//
//		if (m_on_send_msgFromDll != NULL)
//		{
//			return true;
//		}
//	}
//	return false;	// Return an error.
//}
//
//bool AWutopiaMsgReceiver::importOnStartConnect()
//{
//	if (v_dllHandle != NULL)
//	{
//		m_on_start_connectFromDll = NULL;
//		FString procName = "on_start_connect";	// Needs to be the exact name of the DLL method.
//		m_on_start_connectFromDll = (_on_start_connect)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
//
//		if (m_on_start_connectFromDll != NULL)
//		{
//			return true;
//		}
//	}
//	return false;	// Return an error.
//}
//
//bool AWutopiaMsgReceiver::importOnStopConnect()
//{
//	if (v_dllHandle != NULL)
//	{
//		m_on_stop_connectFromDll = NULL;
//		FString procName = "on_stop_connect";	// Needs to be the exact name of the DLL method.
//		m_on_stop_connectFromDll = (_on_stop_connect)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
//
//		if (m_on_stop_connectFromDll != NULL)
//		{
//			return true;
//		}
//	}
//	return false;	// Return an error.
//}
//#pragma endregion Import Methods
//
//#pragma region Method Calls
//
//
//int AWutopiaMsgReceiver::OnSendMsgFromDll(char* data, size_t size)
//{
//	if (m_on_send_msgFromDll != NULL)
//	{
//		int out = int(m_on_send_msgFromDll(data, size)); // Call the DLL method with arguments corresponding to the exact signature and return type of the method.
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString(UTF8_TO_TCHAR(data)));
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("%d"),out));
//		return out;
//	}
//	return -1;	// Return an error.
//}
//
//bool AWutopiaMsgReceiver::OnStartConnect()
//{
//	if (m_on_start_connectFromDll != NULL)
//	{
//		int out = int(m_on_start_connectFromDll(ViuMsgCallback, 1)); // Call the DLL method with arguments corresponding to the exact signature and return type of the method.
//		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, FString::Printf(TEXT("%d"), out));
//		return out==1;
//	}
//	return false;	// Return an error.
//}
//
//int AWutopiaMsgReceiver::OnStopConnect()
//{
//	if (m_on_stop_connectFromDll != NULL)
//	{
//		int out = int(m_on_stop_connectFromDll()); // Call the DLL method with arguments corresponding to the exact signature and return type of the method.
//		return out;
//	}
//	return -1;	// Return an error.
//}
//
//#pragma endregion Method Calls
//
//
//#pragma region Unload DLL
//
//void AWutopiaMsgReceiver::freeDLL()
//{
//	if (v_dllHandle != NULL)
//	{
//		m_on_send_msgFromDll = NULL;
//		m_on_start_connectFromDll = NULL;
//		m_on_stop_connectFromDll = NULL;
//		FPlatformProcess::FreeDllHandle(v_dllHandle);
//		v_dllHandle = NULL;
//	}
//}
//#pragma endregion Unload DLL
