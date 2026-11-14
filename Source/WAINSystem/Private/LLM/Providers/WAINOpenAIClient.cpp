/**
 * @file WAINOpenAIClient.cpp
 * @brief OpenAI Responses API 전송, HTTP 오류 분류 및 응답 Envelope 처리를 구현한다.
 */

#include "WAINOpenAIClient.h"
#include "../Prompt/WAINPromptBuilder.h"
#include "../Validation/WAINStructuredProposal.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace WAINLLMPrivate
{
    constexpr int32 MaxEnvelopeChars = 131072;

    TSharedPtr<FJsonObject> ParseObject(const FString &Text)
    {
        TSharedPtr<FJsonObject> Root;
        if (Text.IsEmpty() || Text.Len() > MaxEnvelopeChars)
            return nullptr;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        if (!FJsonSerializer::Deserialize(Reader, Root))
            return nullptr;
        return Root;
    }

    FString SerializeObject(const TSharedRef<FJsonObject> &Root)
    {
        FString                         Result;
        const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
        if (!FJsonSerializer::Serialize(Root, Writer))
            return FString();
        return Result;
    }

} // namespace WAINLLMPrivate

// ============================================================
// 00. OpenAI Response Validation
// ============================================================
#pragma region 00_OpenAI_Response_Validation

    bool FWAINOpenAIClient::ExtractProposal(const FString         &ResponseJson,
                                            FWAINResponseProposal &OutProposal,
                                            FString               &OutError)
    {
        const TSharedPtr<FJsonObject> Root = WAINLLMPrivate::ParseObject(ResponseJson);
        if (!Root.IsValid())
        {
            OutError = TEXT("LLM API 결과가 비어 있거나 너무 크거나 JSON이 아닙니다.");
            return false;
        }
        FString Status;
        if (!Root->TryGetStringField(TEXT("status"), Status) || Status != TEXT("completed"))
        {
            OutError = TEXT("LLM 응답이 완료되지 않았습니다 (incomplete/refusal/API error).");
            return false;
        }
        const TArray<TSharedPtr<FJsonValue>> *Output = nullptr;
        if (!Root->TryGetArrayField(TEXT("output"), Output) || !Output)
        {
            OutError = TEXT("LLM API 출력 배열이 없습니다.");
            return false;
        }
        FString ModelText;
        for (const TSharedPtr<FJsonValue> &Item : *Output)
        {
            if (!Item.IsValid() || Item->Type != EJson::Object)
                continue;
            const TSharedPtr<FJsonObject> Object = Item->AsObject();
            FString                       Type;
            if (!Object->TryGetStringField(TEXT("type"), Type) || Type != TEXT("message"))
                continue;
            const TArray<TSharedPtr<FJsonValue>> *Contents = nullptr;
            if (!Object->TryGetArrayField(TEXT("content"), Contents) || !Contents)
                continue;
            for (const TSharedPtr<FJsonValue> &Part : *Contents)
            {
                if (!Part.IsValid() || Part->Type != EJson::Object)
                    continue;
                const TSharedPtr<FJsonObject> Content = Part->AsObject();
                FString                       ContentType;
                if (!Content->TryGetStringField(TEXT("type"), ContentType))
                    continue;
                if (ContentType == TEXT("refusal"))
                {
                    OutError = TEXT("모델이 요청에 대한 응답을 거부했습니다.");
                    return false;
                }
                if (ContentType == TEXT("output_text"))
                {
                    FString Text;
                    if (!Content->TryGetStringField(TEXT("text"), Text) || !ModelText.IsEmpty())
                    {
                        OutError = TEXT("LLM 텍스트 블록은 정확히 하나여야 합니다.");
                        return false;
                    }
                    ModelText = MoveTemp(Text);
                }
            }
        }
        if (ModelText.IsEmpty())
        {
            OutError = TEXT("LLM 응답에 Structured Text가 없습니다.");
            return false;
        }
        return FWAINStructuredProposal::Parse(ModelText, OutProposal, OutError);
    }

#pragma endregion

// ============================================================
// 01. Usage And Sanitized Error Classification
// ============================================================
#pragma region 01_Usage_And_Error_Classification

    // 인증 키나 원시 오류 본문은 기록하지 않고 API의 error.code만 검사한다.

    FString FWAINOpenAIClient::ClassifyError(const int32             HttpStatus,
                                             const FString          &ResponseJson,
                                             FWAINLLMRequestMetrics &OutMetrics)
    {
        OutMetrics.HttpStatus = HttpStatus;
        const TSharedPtr<FJsonObject>  Root = WAINLLMPrivate::ParseObject(ResponseJson);
        const TSharedPtr<FJsonObject> *ErrorObject = nullptr;
        if (Root.IsValid() && Root->TryGetObjectField(TEXT("error"), ErrorObject) && ErrorObject)
        {
            FString Code;
            if ((*ErrorObject)->TryGetStringField(TEXT("code"), Code) && Code.Len() <= 80)
            {
                bool bSafe = true;
                for (const TCHAR Character : Code)
                {
                    if (!((Character >= TEXT('a') && Character <= TEXT('z')) ||
                          (Character >= TEXT('0') && Character <= TEXT('9')) || Character == TEXT('_')))
                    {
                        bSafe = false;
                        break;
                    }
                }
                if (bSafe)
                    OutMetrics.ErrorCode = Code;
            }
        }
        OutMetrics.bCreditExhausted = OutMetrics.ErrorCode == TEXT("insufficient_quota") ||
                                      OutMetrics.ErrorCode == TEXT("credit_balance_exhausted") ||
                                      OutMetrics.ErrorCode == TEXT("billing_hard_limit_reached");
        OutMetrics.bRateLimited = HttpStatus == 429 && !OutMetrics.bCreditExhausted;
        if (OutMetrics.bCreditExhausted)
        {
            return TEXT("OpenAI API 크레딧/결제 한도가 소진되었습니다. 계정의 Billing을 확인하세요.");
        }
        if (OutMetrics.bRateLimited)
        {
            return TEXT("OpenAI API의 일시적 요청 제한(429)입니다. 무료 토큰 소진을 의미하지는 않습니다.");
        }
        if (HttpStatus == 401 || HttpStatus == 403)
        {
            return TEXT("OpenAI API 인증 또는 모델 접근 권한 오류입니다.");
        }
        return FString::Printf(TEXT("OpenAI Responses API HTTP %d (비공개 오류 본문은 숨김)"), HttpStatus);
    }

    void FWAINOpenAIClient::ExtractUsage(const FString &ResponseJson, FWAINLLMRequestMetrics &OutMetrics)
    {
        const TSharedPtr<FJsonObject>  Root = WAINLLMPrivate::ParseObject(ResponseJson);
        const TSharedPtr<FJsonObject> *Usage = nullptr;
        if (!Root.IsValid() || !Root->TryGetObjectField(TEXT("usage"), Usage) || !Usage)
            return;
        double Input = 0.0, Output = 0.0;
        if ((*Usage)->TryGetNumberField(TEXT("input_tokens"), Input) &&
            (*Usage)->TryGetNumberField(TEXT("output_tokens"), Output) && FMath::IsFinite(Input) &&
            FMath::IsFinite(Output) && Input >= 0 && Output >= 0 && Input <= MAX_int32 && Output <= MAX_int32)
        {
            OutMetrics.InputTokens = static_cast<int32>(Input);
            OutMetrics.OutputTokens = static_cast<int32>(Output);
        }
    }

#pragma endregion

// ============================================================
// 02. Asynchronous HTTP Request
// ============================================================
#pragma region 02_Asynchronous_HTTP_Request

    bool FWAINOpenAIClient::Submit(const FWAINInteractionRequest &Request,
                                   const FWAINCharacterContext   &Context,
                                   const FWAINLLMConfig          &Config,
                                   FCompletion                    Callback,
                                   FString                       &OutError)
    {
        if (!IsInGameThread() || Config.ApiKey.IsEmpty() || Config.Model.IsEmpty())
        {
            OutError = TEXT("LLM 호출은 Game Thread, API Key, Model이 필요합니다.");
            return false;
        }
        const TSharedPtr<FJsonObject> Schema = WAINLLMPrivate::ParseObject(FWAINStructuredProposal::MakeJsonSchema());
        if (!Schema.IsValid())
        {
            OutError = TEXT("내장 JSON Schema가 잘못되었습니다.");
            return false;
        }
        const TSharedRef<FJsonObject> Format = MakeShared<FJsonObject>();
        Format->SetStringField(TEXT("type"), TEXT("json_schema"));
        Format->SetStringField(TEXT("name"), TEXT("wain_response_proposal"));
        Format->SetBoolField(TEXT("strict"), true);
        Format->SetObjectField(TEXT("schema"), Schema);

        const TSharedRef<FJsonObject> Text = MakeShared<FJsonObject>();
        Text->SetObjectField(TEXT("format"), Format);
        const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
        Body->SetStringField(TEXT("model"), Config.Model);
        const TSharedRef<FJsonObject> Reasoning = MakeShared<FJsonObject>();
        Reasoning->SetStringField(TEXT("effort"), Config.ReasoningEffort);
        Body->SetObjectField(TEXT("reasoning"), Reasoning);
        Body->SetObjectField(TEXT("text"), Text);
        Body->SetBoolField(TEXT("store"), false);
        Body->SetNumberField(TEXT("max_output_tokens"), 4096);

        const TSharedRef<FJsonObject> System = MakeShared<FJsonObject>();
        System->SetStringField(TEXT("role"), TEXT("system"));
        System->SetStringField(TEXT("content"), FWAINPromptBuilder::BuildSystemInstruction());
        const TSharedRef<FJsonObject> User = MakeShared<FJsonObject>();
        User->SetStringField(TEXT("role"), TEXT("user"));
        User->SetStringField(TEXT("content"), FWAINPromptBuilder::BuildUserPrompt(Request, Context));
        TArray<TSharedPtr<FJsonValue>> Inputs;
        Inputs.Add(MakeShared<FJsonValueObject>(System));
        Inputs.Add(MakeShared<FJsonValueObject>(User));
        Body->SetArrayField(TEXT("input"), MoveTemp(Inputs));

        const FString Payload = WAINLLMPrivate::SerializeObject(Body);
        if (Payload.IsEmpty())
        {
            OutError = TEXT("LLM 요청 JSON을 생성하지 못했습니다.");
            return false;
        }
        const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Http = FHttpModule::Get().CreateRequest();
        Http->SetURL(TEXT("https://api.openai.com/v1/responses"));
        Http->SetVerb(TEXT("POST"));
        Http->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Config.ApiKey);
        Http->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        Http->SetTimeout(Config.TimeoutSeconds);
        Http->SetDelegateThreadPolicy(EHttpRequestDelegateThreadPolicy::CompleteOnGameThread);
        Http->SetContentAsString(Payload);

        const TSharedRef<FCompletion> OnComplete = MakeShared<FCompletion>(MoveTemp(Callback));
        Http->OnProcessRequestComplete().BindLambda(
            [OnComplete](FHttpRequestPtr CompletedRequest, FHttpResponsePtr Response, const bool bTransportOK)
            {
                FWAINResponseProposal  Proposal;
                FString                Error;
                bool                   bValid = false;
                FWAINLLMRequestMetrics Metrics;
                if (!bTransportOK || !Response.IsValid())
                {
                    Error = TEXT("LLM HTTP 요청이 실패하거나 시간이 초과되었습니다.");
                }
                else if (Response->GetResponseCode() != 200)
                {
                    Error = FWAINOpenAIClient::ClassifyError(Response->GetResponseCode(),
                                                             Response->GetContentAsString(),
                                                             Metrics);
                }
                else
                {
                    Metrics.HttpStatus = 200;
                    FWAINOpenAIClient::ExtractUsage(Response->GetContentAsString(), Metrics);
                    bValid = FWAINOpenAIClient::ExtractProposal(Response->GetContentAsString(), Proposal, Error);
                }
                (*OnComplete)(bValid, Proposal, Error, Metrics);
            });
        if (!Http->ProcessRequest())
        {
            Http->OnProcessRequestComplete().Unbind();
            OutError = TEXT("LLM HTTP 요청을 시작하지 못했습니다.");
            return false;
        }
        return true;
    }

#pragma endregion
