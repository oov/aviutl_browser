#pragma once

#include <Windows.h>

bool report_(HRESULT hr, LPCWSTR message);
#define report(hr, msg) (report_(hr, msg))
