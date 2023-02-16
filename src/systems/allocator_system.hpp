/**
 *  Author: Amélie Heinrich
 *  Company: Amélie Games
 *  License: MIT
 *  Create Time: 16/02/2023 12:48
 */

#pragma once

#include <cstdint>

struct linear_allocator
{
    uint64_t Start;
    uint64_t End;
    uint64_t Current;
    uint64_t Size;
};

void LinearAllocatorInit(linear_allocator *Allocator, uint64_t Size);
void LinearAllocatorFree(linear_allocator *Allocator);
uint64_t LinearAllocatorAlloc(linear_allocator *Allocator, uint64_t Size);
void LinearAllocatorFree(linear_allocator *Allocator, uint64_t Offset);
void LinearAllocatorReset(linear_allocator *Allocator);
