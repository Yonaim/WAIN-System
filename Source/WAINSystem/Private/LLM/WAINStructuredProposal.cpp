/**
 * @file WAINStructuredProposal.cpp
 * @brief 필드 누락·알 수 없는 필드·비정상 숫자·미지원 Mutation을 거부한다.
 */

#include "WAINStructuredProposal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace WAINStructuredProposal
{
    constexpr int32 MaxResponseChars = 32768;
    constexpr int32 MaxDialogueChars = 4096;
    constexpr int32 MaxMutations = 8;

    bool RequireExactFields(const TSharedPtr<FJsonObject> &Object, const TArray<FString> &Keys, FString &OutError)
    {
        if (!Object.IsValid() || Object->Values.Num() != Keys.Num())
        {
            OutError = TEXT("Structured Proposal에 필수 필드가 없거나 알 수 없는 필드가 있습니다.");
            return false;
        }
        for (const FString &Key : Keys)
        {
            if (!Object->HasField(Key))
            {
                OutError = FString::Printf(TEXT("필수 필드가 없습니다: %s"), *Key);
                return false;
            }
        }
        return true;
    }

    bool ReadBoundedString(const TSharedPtr<FJsonObject> &Object,
                           const TCHAR                   *Key,
                           const int32                    Limit,
                           FString                       &OutValue,
                           FString                       &OutError)
    {
        if (!Object->TryGetStringField(Key, OutValue) || OutValue.Len() > Limit)
        {
            OutError = FString::Printf(TEXT("문자열 필드가 잘못되었습니다: %s"), Key);
            return false;
        }
        return true;
    }

    bool ReadInteger(const TSharedPtr<FJsonObject> &Object, const TCHAR *Key, int32 &OutValue, FString &OutError)
    {
        double Number = 0.0;
        if (!Object->TryGetNumberField(Key, Number) || !FMath::IsFinite(Number) ||
            Number < static_cast<double>(MIN_int32) || Number > static_cast<double>(MAX_int32))
        {
            OutError = FString::Printf(TEXT("정수 필드가 잘못되었습니다: %s"), Key);
            return false;
        }
        OutValue = static_cast<int32>(Number);
        if (static_cast<double>(OutValue) != Number)
        {
            OutError = FString::Printf(TEXT("소수는 허용되지 않습니다: %s"), Key);
            return false;
        }
        return true;
    }

    bool ReadFiniteFloat(const TSharedPtr<FJsonObject> &Object, const TCHAR *Key, float &OutValue, FString &OutError)
    {
        double Number = 0.0;
        if (!Object->TryGetNumberField(Key, Number) || !FMath::IsFinite(Number) || Number < -1.0 || Number > 1.0)
        {
            OutError = FString::Printf(TEXT("감정 Delta 범위를 초과했습니다: %s"), Key);
            return false;
        }
        OutValue = static_cast<float>(Number);
        return true;
    }

    bool ParseMutation(const TSharedPtr<FJsonObject> &Object, FWAINStateMutation &OutMutation, FString &OutError)
    {
        const TArray<FString> Keys = {TEXT("owner"),
                                      TEXT("targetId"),
                                      TEXT("operation"),
                                      TEXT("integerValue"),
                                      TEXT("floatValue")};
        if (!RequireExactFields(Object, Keys, OutError))
            return false;

        FString Owner, Target, Operation;
        if (!ReadBoundedString(Object, TEXT("owner"), 16, Owner, OutError) ||
            !ReadBoundedString(Object, TEXT("targetId"), 128, Target, OutError) ||
            !ReadBoundedString(Object, TEXT("operation"), 32, Operation, OutError) ||
            !ReadInteger(Object, TEXT("integerValue"), OutMutation.IntegerValue, OutError) ||
            !ReadFiniteFloat(Object, TEXT("floatValue"), OutMutation.FloatValue, OutError))
        {
            return false;
        }
        if (Target.IsEmpty() || Target.TrimStartAndEnd() != Target)
        {
            OutError = TEXT("Mutation TargetId는 비어 있거나 공백을 포함할 수 없습니다.");
            return false;
        }
        if (Owner == TEXT("character"))
            OutMutation.Owner = EWAINMutationOwner::Character;
        else if (Owner == TEXT("game"))
            OutMutation.Owner = EWAINMutationOwner::Game;
        else
        {
            OutError = TEXT("지원하지 않는 Mutation Owner입니다.");
            return false;
        }

        if (Operation == TEXT("addInteger"))
            OutMutation.Operation = EWAINMutationOperation::AddInteger;
        else if (Operation == TEXT("setInteger"))
            OutMutation.Operation = EWAINMutationOperation::SetInteger;
        else if (Operation == TEXT("addFloat"))
            OutMutation.Operation = EWAINMutationOperation::AddFloat;
        else
        {
            OutError = TEXT("지원하지 않는 Mutation Operation입니다.");
            return false;
        }

        if ((OutMutation.Operation == EWAINMutationOperation::AddFloat && OutMutation.IntegerValue != 0) ||
            (OutMutation.Operation != EWAINMutationOperation::AddFloat && OutMutation.FloatValue != 0.0F))
        {
            OutError = TEXT("Mutation에 사용되지 않는 숫자 필드는 0이어야 합니다.");
            return false;
        }
        OutMutation.TargetId = FName(*Target);
        return true;
    }
} // namespace WAINStructuredProposal

// ============================================================
// 00. Local Strict Parsing
// ============================================================
#pragma region 00_Local_Strict_Parsing

    bool FWAINStructuredProposal::Parse(const FString &Json, FWAINResponseProposal &OutProposal, FString &OutError)
    {
        OutError.Empty();
        if (Json.IsEmpty() || Json.Len() > WAINStructuredProposal::MaxResponseChars)
        {
            OutError = TEXT("Structured Proposal의 크기가 허용 범위를 벗어났습니다.");
            return false;
        }
        TSharedPtr<FJsonObject>         Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            OutError = TEXT("LLM 응답이 유효한 JSON 객체가 아닙니다.");
            return false;
        }
        const TArray<FString> Keys = {TEXT("dialogue"),
                                      TEXT("actionId"),
                                      TEXT("expressionId"),
                                      TEXT("expressionVariantId"),
                                      TEXT("mutations")};
        if (!WAINStructuredProposal::RequireExactFields(Root, Keys, OutError))
            return false;

        FWAINResponseProposal Parsed;
        FString               Action, Expression, Variant;
        if (!WAINStructuredProposal::ReadBoundedString(Root,
                                                       TEXT("dialogue"),
                                                       WAINStructuredProposal::MaxDialogueChars,
                                                       Parsed.Dialogue,
                                                       OutError) ||
            Parsed.Dialogue.TrimStartAndEnd().IsEmpty() ||
            !WAINStructuredProposal::ReadBoundedString(Root, TEXT("actionId"), 128, Action, OutError) ||
            !WAINStructuredProposal::ReadBoundedString(Root, TEXT("expressionId"), 128, Expression, OutError) ||
            !WAINStructuredProposal::ReadBoundedString(Root, TEXT("expressionVariantId"), 128, Variant, OutError))
        {
            if (OutError.IsEmpty())
                OutError = TEXT("LLM 대사가 비어 있습니다.");
            return false;
        }
        if (Expression.IsEmpty() != Variant.IsEmpty())
        {
            OutError = TEXT("Expression과 Variant는 모두 비어 있거나 모두 지정해야 합니다.");
            return false;
        }
        Parsed.ActionId = FName(*Action);
        Parsed.ExpressionId = FName(*Expression);
        Parsed.ExpressionVariantId = FName(*Variant);

        const TArray<TSharedPtr<FJsonValue>> *Mutations = nullptr;
        if (!Root->TryGetArrayField(TEXT("mutations"), Mutations) || !Mutations ||
            Mutations->Num() > WAINStructuredProposal::MaxMutations)
        {
            OutError = TEXT("Mutation 배열이 잘못되었거나 너무 많습니다.");
            return false;
        }
        for (const TSharedPtr<FJsonValue> &Value : *Mutations)
        {
            if (!Value.IsValid() || Value->Type != EJson::Object)
            {
                OutError = TEXT("Mutation 항목은 객체여야 합니다.");
                return false;
            }
            FWAINStateMutation Mutation;
            if (!WAINStructuredProposal::ParseMutation(Value->AsObject(), Mutation, OutError))
                return false;
            Parsed.StateChanges.Add(MoveTemp(Mutation));
        }
        OutProposal = MoveTemp(Parsed);
        return true;
    }

#pragma endregion

// ============================================================
// 01. API Structured Output Schema
// ============================================================
#pragma region 01_API_Structured_Output_Schema

    FString FWAINStructuredProposal::MakeJsonSchema()
    {
        // OpenAI Responses API text.format.schema에 사용하는 닫힌 형식이다.
        return TEXT("{\"type\":\"object\",\"additionalProperties\":false,\"properties\":{") TEXT(
            "\"dialogue\":{\"type\":\"string\"},") TEXT("\"actionId\":{\"type\":\"string\"},")
            TEXT("\"expressionId\":{\"type\":\"string\"},") TEXT("\"expressionVariantId\":{\"type\":\"string\"},") TEXT(
                "\"mutations\":{\"type\":\"array\",\"items\":{")
                TEXT("\"type\":\"object\",\"additionalProperties\":false,\"properties\":{") TEXT(
                    "\"owner\":{\"type\":\"string\",\"enum\":[\"character\",\"game\"]},")
                    TEXT("\"targetId\":{\"type\":\"string\"},") TEXT(
                        "\"operation\":{\"type\":\"string\",\"enum\":[\"addInteger\",\"setInteger\",\"addFloat\"]},")
                        TEXT("\"integerValue\":{\"type\":\"integer\"},") TEXT("\"floatValue\":{\"type\":\"number\"}},")
                            TEXT("\"required\":[\"owner\",\"targetId\",\"operation\",\"integerValue\",\"floatValue\"]}}},")
                                TEXT("\"required\":[\"dialogue\",\"actionId\",\"expressionId\",\"expressionVariantId\","
                                     "\"mutations\"]}");
    }

#pragma endregion
