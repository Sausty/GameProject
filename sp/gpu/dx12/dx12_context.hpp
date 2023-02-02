/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 01/02/2023 11:33
 */

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <cstdint>

#include "dx12_descriptor_heap.hpp"
#include "dx12_swapchain.hpp"
#include "dx12_fence.hpp"

struct dx12_context
{
    uint32_t Width;
    uint32_t Height;

    ID3D12Device* Device;
    ID3D12Debug1* Debug;
    ID3D12DebugDevice* DebugDevice;
    IDXGIDevice* DXGI;
    IDXGIFactory3* Factory;
    IDXGIAdapter1* Adapter;

    ID3D12CommandQueue* CommandQueue;
    std::vector<ID3D12CommandAllocator*> Allocators;
    std::vector<ID3D12GraphicsCommandList*> Lists;

    dx12_fence DeviceFence;

    dx12_descriptor_heap RTVHeap;
    dx12_descriptor_heap CBVSRVUAVHeap;

    dx12_swapchain SwapChain;
    std::vector<dx12_fence> FrameFences;
    uint32_t FrameIndex;
};

extern dx12_context DX12;
