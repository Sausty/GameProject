/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 01/02/2023 11:30
 */

#pragma once

#include <cstdint>
#include <cstdlib>

#include "gpu_command_buffer.hpp"
#include "gpu_image.hpp"
#include "math_types.hpp"

enum class gpu_backend
{
    DirectX12,
    Vulkan
};

gpu_backend GpuGetBackend();

void GpuInit();
void GpuExit();
void GpuBeginFrame();
void GpuEndFrame();
void GpuResize(uint32_t Width, uint32_t Height);
void GpuPresent();
void GpuWait();
hmm_v2 GpuGetDimensions();
gpu_command_buffer* GpuGetImageCommandBuffer();
gpu_image* GpuGetSwapChainImage();
