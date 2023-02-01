/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 01/02/2023 11:45
 */

#include "dx12_context.hpp"

#include "game_data.hpp"
#include "systems/log_system.hpp"
#include "windows/windows_data.hpp"

#include <string>

extern "C" 
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

dx12_context DX12;

void GetHardwareAdapter(IDXGIFactory3 *Factory, IDXGIAdapter1 **RetAdapter, bool HighPerf)
{
    *RetAdapter = nullptr;
    IDXGIAdapter1* Adapter = 0;
    IDXGIFactory6* Factory6;
    
    if (SUCCEEDED(Factory->QueryInterface(IID_PPV_ARGS(&Factory6)))) 
    {
        for (uint32_t AdapterIndex = 0; SUCCEEDED(Factory6->EnumAdapterByGpuPreference(AdapterIndex, HighPerf == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&Adapter))); ++AdapterIndex) 
        {
            DXGI_ADAPTER_DESC1 Desc;
            Adapter->GetDesc1(&Desc);

            if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            if (SUCCEEDED(D3D12CreateDevice((IUnknown*)Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device*), nullptr)))
                break;
        }
    }
    
    if (Adapter == nullptr) {
        for (uint32_t AdapterIndex = 0; SUCCEEDED(Factory->EnumAdapters1(AdapterIndex, &Adapter)); ++AdapterIndex) 
        {
            DXGI_ADAPTER_DESC1 Desc;
            Adapter->GetDesc1(&Desc);

            if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            if (SUCCEEDED(D3D12CreateDevice((IUnknown*)Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device*), nullptr)))
                break;
        }
    }
    
    *RetAdapter = Adapter;
}

void GpuInit()
{
    bool Debug = EgcB32(EgcFile, "debug_enabled");
    
    if (Debug)
    {
        HRESULT Result = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12.Debug));
        if (FAILED(Result))
            LogError("D3D12: Failed to get debug interface!");
        DX12.Debug->EnableDebugLayer();
    }

    HRESULT Result = CreateDXGIFactory(IID_PPV_ARGS(&DX12.Factory));
    if (FAILED(Result))
        LogError("D3D12: Failed to create DXGI factory!");
    GetHardwareAdapter(DX12.Factory, &DX12.Adapter, true);

    Result = D3D12CreateDevice(DX12.Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&DX12.Device));
    if (FAILED(Result))
        LogError("D3D12: Failed to create device!");
    
    if (Debug)
    {
        Result = DX12.Device->QueryInterface(IID_PPV_ARGS(&DX12.DebugDevice));
        if (FAILED(Result))
            LogError("D3D12: Failed to query debug device!");

        ID3D12InfoQueue* InfoQueue = 0;
        DX12.Device->QueryInterface(IID_PPV_ARGS(&InfoQueue));

        InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

        D3D12_MESSAGE_SEVERITY SupressSeverities[] = {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        D3D12_MESSAGE_ID SupressIDs[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        D3D12_INFO_QUEUE_FILTER filter = {0};
        filter.DenyList.NumSeverities = ARRAYSIZE(SupressSeverities);
        filter.DenyList.pSeverityList = SupressSeverities;
        filter.DenyList.NumIDs = ARRAYSIZE(SupressIDs);
        filter.DenyList.pIDList = SupressIDs;

        InfoQueue->PushStorageFilter(&filter);
        InfoQueue->Release();
    }

    DXGI_ADAPTER_DESC Desc;
    DX12.Adapter->GetDesc(&Desc);
    std::wstring WideName = Desc.Description;
    std::string DeviceName = std::string(WideName.begin(), WideName.end());
    LogInfo("D3D12: Using GPU %s", DeviceName.c_str());

    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    Result = DX12.Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&DX12.CommandQueue));
    if (FAILED(Result))
        LogError("D3D12: Failed to create command queue!");

    int BufferCount = EgcI32(EgcFile, "buffer_count");

    DX12.Allocators.resize(BufferCount);
    DX12.Lists.resize(BufferCount);

    for (int FrameIndex = 0; FrameIndex < BufferCount; FrameIndex++)
    {
        Result = DX12.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&DX12.Allocators[FrameIndex]));
        if (FAILED(Result))
            LogError("D3D12: Failed to create command allocator (frame %d)", FrameIndex);

        Result = DX12.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, DX12.Allocators[FrameIndex], nullptr, IID_PPV_ARGS(&DX12.Lists[FrameIndex]));
        if (FAILED(Result))
            LogError("D3D12: Failed to create graphics command list (frame %d)", FrameIndex);

        Result = DX12.Lists[FrameIndex]->Close();
        if (FAILED(Result))
            LogError("D3D12: Faile to close graphics command list (frame %d)", FrameIndex);
    }

    Dx12DescriptorHeapInit(&DX12.RTVHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024);
    Dx12DescriptorHeapInit(&DX12.CBVSRVUAVHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1'000'000);

    Dx12SwapchainInit(&DX12.SwapChain);
}

void GpuExit()
{
    bool Debug = EgcB32(EgcFile, "debug_enabled");
    int BufferCount = EgcI32(EgcFile, "buffer_count");

    Dx12SwapchainFree(&DX12.SwapChain);
    Dx12DescriptorHeapFree(&DX12.CBVSRVUAVHeap);
    Dx12DescriptorHeapFree(&DX12.RTVHeap);
    for (int FrameIndex = 0; FrameIndex < BufferCount; FrameIndex++)
    {
        SafeRelease(DX12.Lists[FrameIndex]);
        SafeRelease(DX12.Allocators[FrameIndex]);
    }
    SafeRelease(DX12.CommandQueue);
    SafeRelease(DX12.Device);
    SafeRelease(DX12.Factory);
    SafeRelease(DX12.Adapter);
    if (Debug)
    {
        DX12.DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL | D3D12_RLDO_DETAIL);
        SafeRelease(DX12.DebugDevice);
        SafeRelease(DX12.Debug);
    }
}