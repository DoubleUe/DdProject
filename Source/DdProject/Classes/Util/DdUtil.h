#pragma once

#include "CoreMinimal.h"

class UClass;

class DDPROJECT_API FDdUtil
{
public:
	static UClass* LoadClassFromPath(const FString& ResourcePath, const UClass* ExpectedBaseClass = nullptr);

private:
	static FString ConvertToClassPath(const FString& ResourcePath);
};
