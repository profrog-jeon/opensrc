#ifndef __ERROR_CODES_H__
#define __ERROR_CODES_H__

#define START_ERROR_CODE 79

enum ExtractResultCode
{
    ercSucceeded = 0,                   // S_OK

    ercBadFormat = START_ERROR_CODE,    // S_FALSE or HRESULT_CODE(hr) == ERROR_BAD_FORMAT
    ercNoInterface,                     // E_NOINTERFACE
    ercFileNotFound,                    // HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND
    ercOutOfMemory,                     // E_OUTOFMEMORY
    ercEndOfFile,                       // HRESULT_CODE(hr) == ERROR_HANDLE_EOF
    ercUnsupported,                     // HRESULT_CODE(hr) == ERROR_NOT_SUPPORTED
    ercInvalidParameters,               // HRESULT_CODE(hr) == ERROR_INVALID_PARAMETER
    ercInvalidFlags,                    // HRESULT_CODE(hr) == ERROR_INVALID_FLAGS

    ercInvalidPassword,                 // Invalid password
    ercOpenFailed,                      // Failed to file open

    ercUndefined,                       // Other

    ercLast = ercUndefined
};

#endif
