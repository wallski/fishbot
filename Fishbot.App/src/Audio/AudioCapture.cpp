#include "AudioCapture.h"
#include <audioclientactivationparams.h>
#include <cmath>
#include <sstream>

std::string GetHrString(HRESULT hr) {
    std::stringstream ss;
    ss << "0x" << std::hex << hr;
    return ss.str();
}

void ShowError(const std::string& step, HRESULT hr) {
    std::string msg = "Failed at: " + step + "\nError code: " + GetHrString(hr);
    MessageBoxA(NULL, msg.c_str(), "Audio Error", MB_OK | MB_ICONERROR);
}

AudioCapture::AudioCapture() : pwfx(nullptr), initialized(false) {}

AudioCapture::~AudioCapture() {
    if (pAudioClient) pAudioClient->Stop();
    if (pwfx) CoTaskMemFree(pwfx);
}

bool AudioCapture::Initialize(const std::string& windowTitle) {

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        ShowError("CoInitializeEx", hr);
        return false;
    }

    HWND hwnd = FindWindowA(NULL, windowTitle.c_str());
    if (!hwnd) {
        MessageBoxA(NULL, "Failed to find WoW window.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    AUDIOCLIENT_ACTIVATION_PARAMS params = {};
    params.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
    params.ProcessLoopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
    params.ProcessLoopbackParams.TargetProcessId = pid;

    PROPVARIANT propVariant = {};
    propVariant.vt = VT_BLOB;
    propVariant.blob.cbSize = sizeof(params);
    propVariant.blob.pBlobData = (BYTE*)&params;

    Microsoft::WRL::ComPtr<AudioActivationHandler> handler = Microsoft::WRL::Make<AudioActivationHandler>();
    Microsoft::WRL::ComPtr<IActivateAudioInterfaceAsyncOperation> asyncOp;

    hr = ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, __uuidof(IAudioClient), &propVariant, handler.Get(), &asyncOp);
    if (FAILED(hr)) {
        ShowError("ActivateAudioInterfaceAsync", hr);
        return false;
    }

    WaitForSingleObject(handler->hEvent, 5000);

    Microsoft::WRL::ComPtr<IUnknown> audioInterface;
    HRESULT activateHr = S_OK;
    hr = asyncOp->GetActivateResult(&activateHr, &audioInterface);

    if (FAILED(hr)) {
        ShowError("GetActivateResult (Call)", hr);
        return false;
    }
    if (FAILED(activateHr)) {
        ShowError("GetActivateResult (Activation)", activateHr);
        return false;
    }

    hr = audioInterface.As(&pAudioClient);
    if (FAILED(hr)) {
        ShowError("audioInterface.As", hr);
        return false;
    }


    pwfx = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));
    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->nChannels = 2;
    pwfx->nSamplesPerSec = 48000;
    pwfx->wBitsPerSample = 16;
    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
    pwfx->cbSize = 0;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, pwfx, NULL);
    if (FAILED(hr)) {
        ShowError("pAudioClient->Initialize", hr);
        return false;
    }

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), &pCaptureClient);
    if (FAILED(hr)) {
        ShowError("pAudioClient->GetService", hr);
        return false;
    }

    pAudioClient->Start();
    initialized = true;
    return true;
}

float AudioCapture::GetCurrentRMS() {
    if (!initialized) return 0.0f;

    UINT32 packetLength = 0;
    pCaptureClient->GetNextPacketSize(&packetLength);

    float highestRMS = 0.0f;

    while (packetLength != 0) {
        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;

        pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);

        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            short* sData = (short*)pData;
            float sum = 0.0f;
            int samples = numFramesAvailable * pwfx->nChannels;

            for (int i = 0; i < samples; i++) {
                float sample = sData[i] / 32768.0f;
                sum += sample * sample;
            }

            float rms = std::sqrt(sum / samples);
            if (rms > highestRMS) highestRMS = rms;
        }

        pCaptureClient->ReleaseBuffer(numFramesAvailable);
        pCaptureClient->GetNextPacketSize(&packetLength);
    }

    return highestRMS;
}