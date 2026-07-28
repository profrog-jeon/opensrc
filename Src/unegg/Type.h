#pragma once

#ifdef UNICODE
typedef wchar_t EGG_TCHAR;
#define EGGTEXT(s) L##s
#else
typedef char EGG_TCHAR;
#define EGGTEXT(s) s
#endif

typedef EGG_TCHAR* LPEGGTSTR;
typedef const EGG_TCHAR* LPCEGGTSTR;
