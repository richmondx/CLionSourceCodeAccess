// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

UCLionSettings::UCLionSettings(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{

}

bool UCLionSettings::CheckSettings()
{

#if PLATFORM_WINDOWS
	if (this->CLion.FilePath.IsEmpty())
	{
		// Damn windows specific folders
		if (FPaths::FileExists(TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.1\\bin\\clion64.exe")))
		{
			this->CLion.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion64.exe");
		}
		else if (FPaths::FileExists(TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion.exe")))
		{
			this->CLion.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion.exe");
		}
	}
#elif PLATFORM_MAC
    if ( this->CLion.FilePath.IsEmpty())
    {
        this->CLion.FilePath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
    }
    if (this->Mono.FilePath.IsEmpty() )
    {
	    if (FPaths::FileExists(TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono")))
	    {
		    this->Mono.FilePath = TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono");
	    }
    }
#else
	if (this->Mono.FilePath.IsEmpty() )
	{
		if (FPaths::FileExists(TEXT("/usr/bin/mono")))
		{
			this->Mono.FilePath = TEXT("/usr/bin/mono");
		}
		else if (FPaths::FileExists(TEXT("/opt/mono/bin/mono")))
		{
			this->Mono.FilePath = TEXT("/opt/mono/bin/mono");
		}
	}
#endif

	// Reset the setup complete before we check things
	this->bSetupComplete = true;

	if (this->CLion.FilePath.IsEmpty())
	{
		this->bSetupComplete = false;
	}

#if !PLATFORM_WINDOWS
	if ( this->Mono.FilePath.IsEmpty())
	{
		this->bSetupComplete = false;
	}
#endif

	return this->bSetupComplete;
}

bool UCLionSettings::IsSetup()
{
	return this->bSetupComplete;
}

#if WITH_EDITOR

void UCLionSettings::PreEditChange(UProperty *PropertyAboutToChange)
{
//	UE_LOG(LogTemp, Error, TEXT("PRE PROCESS"));
//	TMap<FString, FString> PropertyValues;
//	PropertyAboutToChange->GetNativePropertyValues(PropertyValues);
//
//	for ( TMap<FString,FString>::TIterator It(PropertyValues); It; ++It )
//	{
//		FString key = *It.Key();
//		FString value = *It.Value();
//		UE_LOG(LogTemp, Error, TEXT("%s %s"), *key, *value);
//	}

	this->PreviousCCompiler = this->CCompiler.FilePath;
	this->PreviousMono = this->Mono.FilePath;
	this->PreviousCLion = this->CLion.FilePath;
	this->PreviousCXXCompiler = this->CXXCompiler.FilePath;

	//this->PreviousData = *Cast<FString>(&PyropertyAboutToChange);
}

void UCLionSettings::PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent)
{
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr)
	                                 ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;


	UE_LOG(LogTemp, Error, TEXT("POST PROCESS"));

	// CLion Executable Path Check
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CLion))
	{
		this->CLion.FilePath = FPaths::ConvertRelativePathToFull(this->CLion.FilePath);
		this->CLion.FilePath = this->CLion.FilePath.Trim();
		this->CLion.FilePath = this->CLion.FilePath.TrimTrailing();

		FText FailReason;

#if PLATFORM_MAC
		if (this->CLion.FilePath.EndsWith(TEXT("clion.app")))
		{
			this->CLion.FilePath = this->CLion.FilePath.Append(TEXT("/Contents/MacOS/clion"));
		}

		if (!this->CLion.FilePath.Contains(TEXT("clion.app")))
		{
			FailReason = LOCTEXT("CLionSelectMacApp", "Please select the CLion app");
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			return;
		}
#endif

		if (this->CLion.FilePath == this->PreviousCLion)
		{
			return;
		}

		if (!FPaths::ValidatePath(this->CLion.FilePath, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			return;
		}
	}

	// Mono Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, Mono))
	{
		this->Mono.FilePath = FPaths::ConvertRelativePathToFull(this->Mono.FilePath);
		this->Mono.FilePath = this->Mono.FilePath.Trim();
		this->Mono.FilePath = this->Mono.FilePath.TrimTrailing();

		FText FailReason;

		if (this->Mono.FilePath == this->PreviousMono)
		{
			return;
		}

		if (!FPaths::ValidatePath(this->Mono.FilePath, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->Mono.FilePath = this->PreviousMono;
			return;
		}
	}


	// Check C Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CCompiler))
	{
		this->CCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CCompiler.FilePath);
		this->CCompiler.FilePath = this->CCompiler.FilePath.Trim();
		this->CCompiler.FilePath = this->CCompiler.FilePath.TrimTrailing();

		if (this->CCompiler.FilePath == this->PreviousCCompiler)
		{
			return;
		}

		if (!FPaths::FileExists(this->CCompiler.FilePath))
		{
			this->CCompiler.FilePath = this->PreviousCCompiler;
			return;
		}
		this->bRequireRefresh = true;
	}

	// Check C++ Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CXXCompiler)) {
		this->CXXCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CXXCompiler.FilePath);
		this->CXXCompiler.FilePath = this->CXXCompiler.FilePath.Trim();
		this->CXXCompiler.FilePath = this->CXXCompiler.FilePath.TrimTrailing();

		if (this->CXXCompiler.FilePath == this->PreviousCXXCompiler) return;

		if (!FPaths::FileExists(CXXCompiler.FilePath)) {
			this->CXXCompiler.FilePath = this->PreviousCXXCompiler;
			return;
		}
		this->bRequireRefresh = true;
	}

	this->CheckSettings();
}

#endif


