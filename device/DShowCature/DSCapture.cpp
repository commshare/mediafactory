#include "General.h"
#include "DSCapture.h"

DSCapture::DSCapture()
{
    // initializing a directshow filtergraph
    //
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    ds_graph_ = new DSGraph;

    //audio_encoder_thread_ = NULL;
}

DSCapture::~DSCapture()
{
    SAFE_DELETE( ds_graph_ );
    CoUninitialize();
}

std::string DSCapture::CStringToString(const CString& mfcStr)
{
    CT2CA pszConvertedAnsiString(mfcStr);
    return (pszConvertedAnsiString);
}
