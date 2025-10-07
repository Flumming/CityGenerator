// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "CityGeneratorActor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef CITYGENERATOR_CityGeneratorActor_generated_h
#error "CityGeneratorActor.generated.h already included, missing '#pragma once' in CityGeneratorActor.h"
#endif
#define CITYGENERATOR_CityGeneratorActor_generated_h

#define FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execGenerateCity);


#define FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACityGeneratorActor(); \
	friend struct Z_Construct_UClass_ACityGeneratorActor_Statics; \
public: \
	DECLARE_CLASS(ACityGeneratorActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/CityGenerator"), NO_API) \
	DECLARE_SERIALIZER(ACityGeneratorActor)


#define FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	ACityGeneratorActor(ACityGeneratorActor&&); \
	ACityGeneratorActor(const ACityGeneratorActor&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACityGeneratorActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACityGeneratorActor); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(ACityGeneratorActor) \
	NO_API virtual ~ACityGeneratorActor();


#define FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_8_PROLOG
#define FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_INCLASS_NO_PURE_DECLS \
	FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h_10_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> CITYGENERATOR_API UClass* StaticClass<class ACityGeneratorActor>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_flemm_Documents_Unreal_Projects_CityGenerator_CityGeneratorPlugin_Plugins_CityGenerator_Source_CityGenerator_Public_CityGeneratorActor_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
