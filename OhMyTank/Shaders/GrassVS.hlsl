#include "Shaders/Common/CBufferData.hlsl"
#include "Shaders/Common/CBufferView.hlsl"
#include "Shaders/Common/CBufferProjection.hlsl"

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"
#include "Shaders/Common/InstantiableVSInput.hlsl"
#include "Shaders/Common/Utils.hlsl"

#define HALF_PI 1.57079632679
#define PI 3.14159265359

vsoutput vsmain(vsinput input)
{
	vsoutput output = (vsoutput)0;
	
    output.position = mul(input.position, input.iModel);
    float2 bladePos = float2(input.iModel._41, input.iModel._43) + input.color.rg;

    float randomVal = random(bladePos);

// Blade offset
    float rotVal = input.uv.x * 2 - 1;
    output.position.x += lerp(-0.5, 0.5, randomVal); + sin(randomVal * PI) * rotVal * 0.05;
    output.position.z += lerp(-0.5, 0.5, randomVal) + cos(randomVal * PI) * rotVal * 0.05;
    float2 topPos = output.position.xz;

// Tip height
    output.color = gnoise(bladePos / 2);

    float2 topUvs = output.position.xz - d_grassfieldPos;
    float2 heightmapUvs = topUvs / d_grassfieldSize;
    float2 distanceUvs = heightmapUvs * 2;
    heightmapUvs += 0.5;

    float4 grassMap = Texture.SampleLevel(TextureSampler, heightmapUvs, 0);
    float heightMap = grassMap.r;
    float height = lerp(2, 1, randomVal * output.color.r) * input.uv.y;
    height = lerp(0, height, heightMap);

    float distanceToTank = length(d_tankPos - topPos) / d_grassfieldSize * 2;
    float farVal = smoothstep(1, 0.05, distanceToTank);

    height *= farVal;

    output.position.y = height;
    output.color.a = heightMap;

    // Fold
    float2 hMove = grassMap.gb * sin(HALF_PI - 0.15) * height;
    float vMove = cos(HALF_PI - 0.15);

    // float foldFieldOne = length(d_tankPos - topPos + d_tankDir * 0.75) / d_grassfieldSize * 2;
    // float foldFieldTwo = length(d_tankPos - topPos - d_tankDir * 0.75) / d_grassfieldSize * 2;
    // float foldIntensity = smoothstep(0.02, 0.010, foldFieldOne);
    // foldIntensity = max(foldIntensity, smoothstep(0.02, 0.010, foldFieldTwo)) * input.uv.y;
    float foldIntensity = grassMap.a;

// Tip swing
    float noise = gnoise(bladePos + d_elapsedTime);
    float swing = sin(d_elapsedTime + noise * 4);
    swing += sin(d_elapsedTime * 3.7 + randomVal);

    float swingAmount = (swing * 0.3 + 0.5) * input.uv.y * lerp(.3, .45, randomVal) * height * (1 - foldIntensity);
    output.position.x += swingAmount;
    
    output.position.y = lerp(output.position.y, output.position.y * vMove, foldIntensity);
    output.position.xz += hMove * foldIntensity;

    input.normal = normalize(input.normal);
    output.normal = input.normal;
    output.normal = normalize(output.normal);

// Curve
    //float val = (output.position.z - d_tankPos.y);
    //val = inverseLerp(5, 300, val);
    //val *= val;
    //output.position.y += val * 60;

// View Proj transformation
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    output.uv = input.uv;

    return output;
}