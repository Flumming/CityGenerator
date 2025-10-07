// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "CityGenerator/Public/CityGeneratorActor.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeCityGeneratorActor() {}

// Begin Cross Module References
CITYGENERATOR_API UClass* Z_Construct_UClass_ACityGeneratorActor();
CITYGENERATOR_API UClass* Z_Construct_UClass_ACityGeneratorActor_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AActor();
UPackage* Z_Construct_UPackage__Script_CityGenerator();
// End Cross Module References

// Begin Class ACityGeneratorActor Function GenerateCity
struct Z_Construct_UFunction_ACityGeneratorActor_GenerateCity_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "City Generator" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Function callable in editor & Blueprints\n" },
#endif
		{ "ModuleRelativePath", "Public/CityGeneratorActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Function callable in editor & Blueprints" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_ACityGeneratorActor_GenerateCity_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_ACityGeneratorActor, nullptr, "GenerateCity", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_ACityGeneratorActor_GenerateCity_Statics::Function_MetaDataParams), Z_Construct_UFunction_ACityGeneratorActor_GenerateCity_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_ACityGeneratorActor_GenerateCity()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_ACityGeneratorActor_GenerateCity_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(ACityGeneratorActor::execGenerateCity)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->GenerateCity();
	P_NATIVE_END;
}
// End Class ACityGeneratorActor Function GenerateCity

// Begin Class ACityGeneratorActor
void ACityGeneratorActor::StaticRegisterNativesACityGeneratorActor()
{
	UClass* Class = ACityGeneratorActor::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GenerateCity", &ACityGeneratorActor::execGenerateCity },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(ACityGeneratorActor);
UClass* Z_Construct_UClass_ACityGeneratorActor_NoRegister()
{
	return ACityGeneratorActor::StaticClass();
}
struct Z_Construct_UClass_ACityGeneratorActor_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "CityGeneratorActor.h" },
		{ "ModuleRelativePath", "Public/CityGeneratorActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CitySize_MetaData[] = {
		{ "Category", "City Generator" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Expose city size in the editor\n" },
#endif
		{ "ModuleRelativePath", "Public/CityGeneratorActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Expose city size in the editor" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CitySize;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_ACityGeneratorActor_GenerateCity, "GenerateCity" }, // 3474775569
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ACityGeneratorActor>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_ACityGeneratorActor_Statics::NewProp_CitySize = { "CitySize", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACityGeneratorActor, CitySize), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CitySize_MetaData), NewProp_CitySize_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ACityGeneratorActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACityGeneratorActor_Statics::NewProp_CitySize,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACityGeneratorActor_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_ACityGeneratorActor_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_CityGenerator,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACityGeneratorActor_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_ACityGeneratorActor_Statics::ClassParams = {
	&ACityGeneratorActor::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_ACityGeneratorActor_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_ACityGeneratorActor_Statics::PropPointers),
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_ACityGeneratorActor_Statics::Class_MetaDataParams), Z_Construct_UClass_ACityGeneratorActor_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_ACityGeneratorActor()
{
	if (!Z_Registration_Info_UClass_ACityGeneratorActor.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_ACityGeneratorActor.OuterSingleton, Z_Construct_UClass_ACityGeneratorActor_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_ACityGeneratorActor.OuterSingleton;
}
template<> CITYGENERATOR_API UClass* StaticClass<ACityGeneratorActor>()
{
	return ACityGeneratorActor::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(ACityGeneratorActor);
ACityGeneratorActor::~ACityGeneratorActor() {}
// End Class ACityGeneratorActor

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_ACityGeneratorActor, ACityGeneratorActor::StaticClass, TEXT("ACityGeneratorActor"), &Z_Registration_Info_UClass_ACityGeneratorActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(ACityGeneratorActor), 3970967969U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_2634463548(TEXT("/Script/CityGenerator"),
	Z_CompiledInDeferFile_FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
