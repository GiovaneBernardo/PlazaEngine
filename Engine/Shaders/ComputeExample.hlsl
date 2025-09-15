// PassThroughCompute.hlsl

// Input structured buffer (read-only)
StructuredBuffer<float4> InputData : register(t0);

// Output RW structured buffer (read-write)
RWStructuredBuffer<float4> OutputData : register(u0);

// Thread group size
[numthreads(64, 1, 1)]
void csMain(uint3 DTid : SV_DispatchThreadID)
{
    // Simply copy input element at the thread ID to output
    OutputData[DTid.x] = InputData[DTid.x];
}
