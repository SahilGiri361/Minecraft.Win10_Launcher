﻿#include "pch.h"
#include "combaseapi.h"
#include <thread>

#using ".\\clrlib\\Windows.Security.winmd";
#using ".\\clrlib\\Windows.winmd";

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Authentication::Web::Core;
using namespace Windows::Security::Authentication::Web;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Credentials;

#define WU_NO_ACCOUNT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200)

extern "C" __declspec(dllexport) int  __stdcall GetWUToken(wchar_t** retToken) {
	auto tokenBrokerStatics = get_activation_factory<TokenBrokerInternal, Windows::Foundation::IUnknown>();
	auto statics = tokenBrokerStatics.as<ITokenBrokerInternalStatics>();
	auto accounts = statics.FindAllAccountsAsync().get();
	wprintf(L"Account count = %i\n", accounts.Size());
	if (accounts.Size() == 0)
		return WU_NO_ACCOUNT;
	auto accountInfo = accounts.GetAt(0);
	wprintf(L"ID = %s\n", accountInfo.Id().c_str());
	wprintf(L"Name = %s\n", accountInfo.UserName().c_str());

	auto accountProvider = WebAuthenticationCoreManager::FindAccountProviderAsync(L"https://login.microsoft.com", L"consumers").get();
	WebTokenRequest request(accountProvider, L"service::dcat.update.microsoft.com::MBI_SSL", L"{28520974-CE92-4F36-A219-3F255AF7E61E}");
	auto result = WebAuthenticationCoreManager::GetTokenSilentlyAsync(request, accountInfo).get();
	auto token = result.ResponseData().GetAt(0).Token();
	wprintf(L"Token = %s\n", token.c_str());
	auto tokenBinary = CryptographicBuffer::ConvertStringToBinary(token, BinaryStringEncoding::Utf16LE);
	auto tokenBase64 = CryptographicBuffer::EncodeToBase64String(tokenBinary);
	wprintf(L"Encoded token = %s\n", tokenBase64.c_str());

	*retToken = (wchar_t*)::CoTaskMemAlloc((tokenBase64.size() + 1) * sizeof(wchar_t));
	memcpy(*retToken, tokenBase64.data(), (tokenBase64.size() + 1) * sizeof(wchar_t));
	return S_OK;
}
