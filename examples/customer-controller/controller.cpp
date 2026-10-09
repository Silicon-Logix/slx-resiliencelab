// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Measurement-only redundant control and progress supervision.

#include <algorithm>
#include <cmath>
#include <slx/customer_api.h>
namespace
{
struct Context
{
    uint32_t id;
};
void *create(uint32_t id)
{
    return new Context{id};
}
void destroy(void *context)
{
    delete static_cast<Context *>(context);
}
int step(void *, const SlxControlInput *in, SlxControlOutput *out)
{
    if (!in || !out || in->abi != SLX_CUSTOMER_ABI || in->size != sizeof(*in))
        return 1;
    out->motor_normalized =
        in->quorum_available ? std::clamp((in->requested_percent - in->fused_percent) * .055, -1.0, 1.0) : 0;
    if (std::abs(in->requested_percent - in->fused_percent) < .3)
        out->motor_normalized = 0;
    return 0;
}
const SlxCustomerApi api{
    SLX_CUSTOMER_ABI, sizeof(SlxCustomerApi), "Example proportional controller", create, destroy, step};
} // namespace
extern "C" SLX_CUSTOMER_EXPORT const SlxCustomerApi *slx_customer_controller_v1()
{
    return &api;
}
