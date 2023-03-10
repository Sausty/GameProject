/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 01/02/2023 14:44
 */

#include "dx12_fence.hpp"

#include "dx12_context.hpp"
#include "systems/log_system.hpp"
#include "windows/windows_data.hpp"

#include <exception>

void Dx12FenceInit(dx12_fence *Fence)
{
    Fence->Value = 0;
    HRESULT Result = DX12.Device->CreateFence(Fence->Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence->Fence));
    if (FAILED(Result))
        LogError("D3D12: Failed to create fence!");
}

void Dx12FenceFree(dx12_fence *Fence)
{
    SafeRelease(Fence->Fence);
}

uint64_t Dx12FenceSignal(dx12_fence *Fence, ID3D12CommandQueue *Queue)
{
    Fence->Value++;
    Queue->Signal(Fence->Fence, Fence->Value);
    return Fence->Value;
}

bool Dx12FenceReached(dx12_fence *Fence, uint64_t Value)
{
    return Fence->Fence->GetCompletedValue() >= Value;
}

void Dx12FenceSync(dx12_fence *Fence, uint64_t Value)
{
    while (!Dx12FenceReached(Fence, Value)) {
        Fence->Fence->SetEventOnCompletion(Value, nullptr);
    }
}

void Dx12FenceFlush(dx12_fence *Fence, ID3D12CommandQueue *Queue)
{
    Dx12FenceSync(Fence, Dx12FenceSignal(Fence, Queue));
}

void Dx12FenceWait(dx12_fence *Fence, uint64_t TargetValue, uint32_t Timeout)
{
    if (Fence->Fence->GetCompletedValue() < TargetValue)
    {
        HANDLE Event = CreateEvent(nullptr, false, false, nullptr);
        Fence->Fence->SetEventOnCompletion(TargetValue, Event);
        if (!Event)
        {
            LogError("D3D12: fence timeout!");
            throw std::exception();
        }
        WaitForSingleObject(Event, Timeout);
    }
}
