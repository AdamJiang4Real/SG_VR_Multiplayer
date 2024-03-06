#pragma once

#include "WutopiaData.generated.h"

USTRUCT(BlueprintType)
struct FWutopiaCam {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString id;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float pan = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float tilt = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float x = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float y = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float z = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float zoom = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float focus= 0;

	FWutopiaCam() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaDS {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString server;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int port = 0;

	FWutopiaDS() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaAppearance {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString type;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString subtype;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int ID = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int valid = 0;

	FWutopiaAppearance() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaCharacterControl {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int speed;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int type;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int hair;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int blouse;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int pant;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int shoe;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int face;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int glass;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int prop; //TBD support array
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaAppearance appearance; //TBD support array
	FWutopiaCharacterControl():
	speed(0),
	type(0),
	hair(0),
	blouse(0),
	pant(0),
	shoe(0),
	face(0),
	glass(0),
	prop(0)
	{}
};

USTRUCT(BlueprintType)
struct FWutopiaIdNamePair {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString name;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int id = 0;

	FWutopiaIdNamePair() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaPoint {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float x = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float y = 0;

	FWutopiaPoint() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaMap {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float x;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float y;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float z;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float yaw;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float fov;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int valid;

	FWutopiaMap() {
	}
};


USTRUCT(BlueprintType)
struct FWutopiaBoneData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FName boneName = FName("none");
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FTransform boneTransform;// = FTransform(FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1));
	FWutopiaBoneData() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaInitParams {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString username;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString uuid;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString IP;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int port;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int showLog;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int comp;
	FWutopiaInitParams() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaBodyFrameData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString ID;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		TArray< FWutopiaBoneData> bodyData;
	FWutopiaBodyFrameData() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaFaceData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FName Name = FName("none");
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float Value;
	FWutopiaFaceData() {
	}
};


USTRUCT(BlueprintType)
struct FWutopiaFaceFrameData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString ID;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		TArray< FWutopiaFaceData> faceData;
	FWutopiaFaceFrameData() {
	}
};

USTRUCT(BlueprintType)
struct FWutopiaTagBag {
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString tid;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float x;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float y;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		float z;
	
	FWutopiaTagBag():
	tid("NONE"),
	x(0),
	y(0),
	z(0)
	{}
};

USTRUCT(BlueprintType)

struct FWutopiaData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString type;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString key;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString id;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaCam cam;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaDS ds;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaCharacterControl chrControl;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString value;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		int minimapFreq;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaPoint point;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FString customStr;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Wutopia")
		FWutopiaInitParams initParams;
	FString customType;//自定义custom消息类型的type

	FWutopiaData() {
	}
};

