#include "WutopiaHumanStreamMsgReceiver.h"


AWutopiaHumanStreamMsgReceiver::AWutopiaHumanStreamMsgReceiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bodyListenSocket = NULL;
	is_running = false;
	bodyRecvBuffer.SetNumUninitialized(1024);
	got_new_body_data = false;
}

void AWutopiaHumanStreamMsgReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	is_running = false;

	bodyMsgRecvThread.join();

	//Clear all sockets
	if (bodyListenSocket)
	{
		bodyListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(bodyListenSocket);
	}
}

//body data
bool AWutopiaHumanStreamMsgReceiver::StartWutopiaBodyMsgReceiver(const FString& SocketName, const int32 Port)
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	int32 BufferSize = 2 * 1024 * 1024;
	bodyListenSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	is_running = true;
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	bodyMsgRecvThread = std::thread(std::bind(&AWutopiaHumanStreamMsgReceiver::bodyMsgRecving, this));
	return true;
}

void AWutopiaHumanStreamMsgReceiver::bodyMsgRecving()
{
	//TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	while (is_running)
	{
		if (bodyListenSocket->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
		{
			uint32 Size;

			while (bodyListenSocket->HasPendingData(Size))
			{
				int32 Read = 0;
				if (bodyListenSocket->RecvFrom(bodyRecvBuffer.GetData(), bodyRecvBuffer.Num(), Read, *Sender))
				{
					if (Read > 0)
					{
						TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
						ReceivedData->SetNumUninitialized(Read);
						memcpy(ReceivedData->GetData(), bodyRecvBuffer.GetData(), Read);
						HandleBodyStreamData(ReceivedData);  // 处理躯干数据
					}
				}
			}
		}
	}
}

void AWutopiaHumanStreamMsgReceiver::HandleBodyStreamData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (flag)  // String JSON I frame
	{
		FString strMt;
		FString strDt;
		JsonObject->TryGetStringField(TEXT("mt"), strMt);
		JsonObject->TryGetStringField(TEXT("dt"), strDt);

		if (strMt == "st" && strDt == "body")
		{
			HandleBodyData(JsonObject);
		}
		else
		{
			//error info
		}
	}
	else
	{
		//error
	}
}

void AWutopiaHumanStreamMsgReceiver::HandleBodyData(TSharedPtr<FJsonObject> JsonObject)
{
	FString str;
	const TSharedPtr<FJsonObject>* jsonData;
	int isPFrame = 1;
	
	if (JsonObject->TryGetObjectField(TEXT("da"), jsonData))
	{
		isPFrame = 0;
		if ((*jsonData)->TryGetNumberField(TEXT("numPoses"), numPosesCur)) //I frame
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("numPosesCur , %d"), numPosesCur));
			const TArray< TSharedPtr<FJsonValue> >* tmp;
			if ((*jsonData)->TryGetArrayField(TEXT("poseNames"), tmp))
			{
				TArray<TSharedPtr<FJsonValue>> poseNames = (*jsonData)->GetArrayField(TEXT("poseNames"));

				BoneNamesCur.Empty();

				for (int i = 0; i < numPosesCur; i++)
				{
					BoneNamesCur.Add(FName(poseNames[i]->AsString()));
				}
			}
		}

		FWutopiaBodyFrameData bodyFrameDataCur;
		TArray<TSharedPtr<FJsonValue>> framePose = (*jsonData)->GetArrayField(TEXT("data"));

		if ((!isPFrame && framePose.Num() == numPosesCur * 6) || (isPFrame && framePose.Num() == numPosesCur * 6+1))
		{
			if (!isPFrame)
			{
				(*jsonData)->TryGetStringField(TEXT("ID"), str);
				bodyFrameDataCur.ID = str;
			}
			else
			{
				bodyFrameDataCur.ID = framePose[0]->AsString();
			}

			for (int poseId = 0; poseId < numPosesCur; poseId++)  // 获取每一骨骼
			{
				FName boneName = BoneNamesCur[poseId];
				FVector BoneLocation; // loaction的值

				double X = 0;
				double Y = 0;
				double Z = 0;
				if (boneName == FName("Hips"))
				{
					X = framePose[poseId * 6 + isPFrame]->AsNumber();
					Y = framePose[1 + poseId * 6 + isPFrame]->AsNumber();
					Z = framePose[2 + poseId * 6 + isPFrame]->AsNumber();
				}
				BoneLocation = FVector(X, Y, Z);

				FRotator BoneQuat;
				double Roll = framePose[3 + poseId * 6 + isPFrame]->AsNumber();
				double Pitch = framePose[4 + poseId * 6 + isPFrame]->AsNumber();
				double Yaw = framePose[5 + poseId * 6 + isPFrame]->AsNumber();
				if (boneName == FName("LeftShoulder") || boneName == FName("RightArm"))
				{
					Yaw -= 10;
				}
				if (boneName == FName("RightShoulder") || boneName == FName("leftArm"))
				{
					Yaw += 10;
				}
				BoneQuat = FRotator(Pitch, Yaw, Roll);  // FRotator(Pitch,Yaw,Roll)   Pitch:Y轴   Yaw:Z轴  Roll:X轴

				FVector BoneScale; // scale的值
				double ScaleX = 1.0;// framePose[6 + poseId * 9]->AsNumber();
				double ScaleY = 1.0;// framePose[7 + poseId * 9]->AsNumber();
				double ScaleZ = 1.0;// framePose[8 + poseId * 9]->AsNumber();
				BoneScale = FVector(ScaleX, ScaleY, ScaleZ);

				FWutopiaBoneData boneData;
				boneData.boneName = boneName;
				boneData.boneTransform = FTransform(BoneQuat, BoneLocation, BoneScale);
				bodyFrameDataCur.bodyData.Add(boneData);
			}

			bodyFrameData = bodyFrameDataCur;
			got_new_body_data = true;
		}

	}
}

FWutopiaBodyFrameData AWutopiaHumanStreamMsgReceiver::GetBodyData()
{
	got_new_body_data = false;
	return bodyFrameData;
}

//Face data
bool AWutopiaHumanStreamMsgReceiver::StartWutopiaFaceMsgReceiver(const FString& SocketName, const int32 Port)
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	int32 BufferSize = 2 * 1024 * 1024;
	faceListenSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	is_running = true;
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	faceMsgRecvThread = std::thread(std::bind(&AWutopiaHumanStreamMsgReceiver::faceMsgRecving, this));
	return true;
}

void AWutopiaHumanStreamMsgReceiver::faceMsgRecving()
{
	while (is_running)
	{
		if (faceListenSocket->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
		{
			uint32 Size;

			while (faceListenSocket->HasPendingData(Size))
			{
				int32 Read = 0;
				if (faceListenSocket->RecvFrom(faceRecvBuffer.GetData(), faceRecvBuffer.Num(), Read, *Sender))
				{
					if (Read > 0)
					{
						TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
						ReceivedData->SetNumUninitialized(Read);
						memcpy(ReceivedData->GetData(), faceRecvBuffer.GetData(), Read);
						HandleFaceStreamData(ReceivedData);  // 处理躯干数据
					}
				}
			}
		}
	}
}

void AWutopiaHumanStreamMsgReceiver::HandleFaceStreamData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (flag)  // String JSON I frame
	{
		FString strMt;
		FString strDt;
		JsonObject->TryGetStringField(TEXT("mt"), strMt);
		JsonObject->TryGetStringField(TEXT("dt"), strDt);

		if (strMt == "st" && strDt == "face")
		{
			HandleFaceData(JsonObject);
		}
		else
		{
			//error info
		}
	}
	else
	{
		//error
	}
}

void AWutopiaHumanStreamMsgReceiver::HandleFaceData(TSharedPtr<FJsonObject> JsonObject)
{
	FString str;
	const TSharedPtr<FJsonObject>* jsonData;
	int isPFrame = 1;

	if (JsonObject->TryGetObjectField(TEXT("da"), jsonData))
	{
		isPFrame = 0;
		if ((*jsonData)->TryGetNumberField(TEXT("numBs"), numBsCur)) //I frame
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("numPosesCur , %d"), numPosesCur));
			const TArray< TSharedPtr<FJsonValue> >* tmp;
			if ((*jsonData)->TryGetArrayField(TEXT("bs"), tmp))
			{
				TArray<TSharedPtr<FJsonValue>> bs = (*jsonData)->GetArrayField(TEXT("bs"));

				BSNamesCur.Empty();
				// TBD handle error when numBs and bs not consistent
				for (int i = 0; i < numPosesCur; i++)
				{
					BSNamesCur.Add(FName(bs[i]->AsString()));
				}
			}
		}

		FWutopiaFaceFrameData faceFrameDataCur;
		TArray<TSharedPtr<FJsonValue>> frameBs = (*jsonData)->GetArrayField(TEXT("data"));

		if ((!isPFrame && frameBs.Num() == numBsCur) || (isPFrame && frameBs.Num() == numPosesCur + 1))
		{
			if (!isPFrame)
			{
				(*jsonData)->TryGetStringField(TEXT("ID"), str);
				faceFrameDataCur.ID = str;
			}
			else
			{
				faceFrameDataCur.ID = frameBs[0]->AsString();
			}

			for (int poseId = 0; poseId < numPosesCur; poseId++)  // 获取每一骨骼
			{
				FName bsName = BSNamesCur[poseId];
				float value = frameBs[poseId + isPFrame]->AsNumber();
				FWutopiaFaceData faceData;
				faceData.Name = bsName;
				faceData.Value = value;
				faceFrameDataCur.faceData.Add(faceData);
			}

			faceFrameData = faceFrameDataCur;
			got_new_face_data = true;
		}

	}
}

FWutopiaFaceFrameData AWutopiaHumanStreamMsgReceiver::GetFaceData()
{
	got_new_face_data = false;
	return faceFrameData;
}