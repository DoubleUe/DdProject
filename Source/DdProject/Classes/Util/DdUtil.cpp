#include "Util/DdUtil.h"

#include "Misc/PackageName.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPath.h"

UClass* FDdUtil::LoadClassFromPath(const FString& ResourcePath, const UClass* ExpectedBaseClass)
{
	const FString ClassPath = ConvertToClassPath(ResourcePath);
	if (ClassPath.IsEmpty())
	{
		return nullptr;
	}

	UClass* LoadedClass = FSoftClassPath(ClassPath).TryLoadClass<UObject>();
	if (LoadedClass == nullptr)
	{
		return nullptr;
	}

	if (ExpectedBaseClass != nullptr && !LoadedClass->IsChildOf(ExpectedBaseClass))
	{
		return nullptr;
	}

	return LoadedClass;
}

FString FDdUtil::ConvertToClassPath(const FString& ResourcePath)
{
	if (ResourcePath.IsEmpty())
	{
		return FString();
	}

	FString ObjectPath = ResourcePath;
	if (ObjectPath.Contains(TEXT("'")))
	{
		const FString ConvertedObjectPath = FPackageName::ExportTextPathToObjectPath(ResourcePath);
		if (!ConvertedObjectPath.IsEmpty())
		{
			ObjectPath = ConvertedObjectPath;
		}
	}

	if (ObjectPath.EndsWith(TEXT("_C")))
	{
		return ObjectPath;
	}

	const FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPath);
	const FString ObjectName = FPackageName::ObjectPathToObjectName(ObjectPath);
	if (PackageName.IsEmpty() || ObjectName.IsEmpty())
	{
		return FString();
	}

	return FString::Printf(TEXT("%s.%s_C"), *PackageName, *ObjectName);
}
