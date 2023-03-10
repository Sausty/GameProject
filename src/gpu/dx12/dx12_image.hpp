/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 06/02/2023 10:44
 */

#pragma once

#include "gpu/gpu_image.hpp"
#include "dx12_descriptor_heap.hpp"
#include <d3d12.h>
#include <D3D12MA/D3D12MemAlloc.h>

struct dx12_image
{
    ID3D12Resource *Resource;
    D3D12MA::Allocation *Allocation;
    D3D12_RESOURCE_STATES State;

    uint32_t RTV;
    uint32_t DSV;
    uint32_t SRV_UAV;
};

DXGI_FORMAT GetDXGIFormat(gpu_image_format Format);
