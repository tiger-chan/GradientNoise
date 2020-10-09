#ifndef GRADIENT_NOISE_SRC_JSON_LOADER_HPP
#define GRADIENT_NOISE_SRC_JSON_LOADER_HPP

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/EngineTypes.h"
#include "Json.h"

static TSharedPtr<FJsonObject> load_json_object(const FFilePath &path)
{
	FString json_file_path = path.FilePath;
	if (FPaths::IsRelative(json_file_path)) {
		json_file_path = FPaths::ConvertRelativePathToFull(json_file_path);
	}

	FString json_string;
	FFileHelper::LoadFileToString(json_string, *json_file_path);

	TSharedPtr<FJsonObject> json_object = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> json_reader = TJsonReaderFactory<>::Create(json_string);

	if (FJsonSerializer::Deserialize(json_reader, json_object) && json_object.IsValid()) {
		return json_object;
	}

	return { nullptr };
}

static TArray<TSharedPtr<FJsonValue>> load_json_array(const FFilePath &path)
{
	FString json_file_path = path.FilePath;
	if (FPaths::IsRelative(json_file_path)) {
		json_file_path = FPaths::ConvertRelativePathToFull(json_file_path);
	}

	FString json_string;
	FFileHelper::LoadFileToString(json_string, *json_file_path);

	TArray<TSharedPtr<FJsonValue>> json_array;
	TSharedRef<TJsonReader<>> json_reader = TJsonReaderFactory<>::Create(json_string);

	if (FJsonSerializer::Deserialize(json_reader, json_array) && json_array.Num() > 0) {
		return json_array;
	}

	return {};
}

#endif // GRADIENT_NOISE_SRC_JSON_LOADER_HPP