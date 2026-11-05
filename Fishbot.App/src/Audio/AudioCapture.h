#pragma once
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <string>
#include <wrl/client.h>
#include <wrl/implements.h>


class AudioActivationHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IActivateAudioInterfaceCompletionHandler,
    Microsoft::WRL::FtmBase>
{
public:
    HANDLE hEvent;
    AudioActivationHandler() { hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); }
    ~AudioActivationHandler() { CloseHandle(hEvent); }
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation) {
        SetEvent(hEvent);
        return S_OK;
    }
};

class AudioCapture {
public:
    AudioCapture();
    ~AudioCapture();

    bool Initialize(const std::string& windowTitle);
    float GetCurrentRMS();

private:
    Microsoft::WRL::ComPtr<IAudioClient> pAudioClient;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> pCaptureClient;
    WAVEFORMATEX* pwfx;
    bool initialized;
};