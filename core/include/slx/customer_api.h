// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Project customer api implementation.

// C ABI v1: no Qt, plant position, simulator faults or privileged state.
#pragma once
#include <stdint.h>
#if defined(_WIN32)
#define SLX_CUSTOMER_EXPORT __declspec(dllexport)
#else
#define SLX_CUSTOMER_EXPORT __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C"
{
#endif
#define SLX_CUSTOMER_ABI 1u
    typedef struct SlxControlInput
    {
        uint32_t abi;
        uint32_t size;
        double virtual_time_s;
        double dt_s;
        double requested_percent;
        double received_percent[3];
        double measurement_age_s[3];
        uint32_t trusted_mask;
        double fused_percent;
        uint32_t quorum_available;
        uint32_t controller_id;
    } SlxControlInput;
    typedef struct SlxControlOutput
    {
        uint32_t abi;
        uint32_t size;
        double motor_normalized; // [-1, +1]; enforced by host, including no-quorum stop.
    } SlxControlOutput;
    typedef void *(*SlxCreate)(uint32_t controller_id);
    typedef void (*SlxDestroy)(void *context);
    typedef int (*SlxStep)(void *context, const SlxControlInput *input, SlxControlOutput *output);
    typedef struct SlxCustomerApi
    {
        uint32_t abi;
        uint32_t size;
        const char *name;
        SlxCreate create;
        SlxDestroy destroy;
        SlxStep step;
    } SlxCustomerApi;
    typedef const SlxCustomerApi *(*SlxGetApi)(void);
    // Export exactly this symbol. Each redundant controller receives its own context.
    SLX_CUSTOMER_EXPORT const SlxCustomerApi *slx_customer_controller_v1(void);
#ifdef __cplusplus
}
#endif
