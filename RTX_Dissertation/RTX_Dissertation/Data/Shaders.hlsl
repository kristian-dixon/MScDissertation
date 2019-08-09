
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);


// #DXR Extra: Perspective Camera
cbuffer CameraParams : register(b0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 viewI;
	float4x4 projectionI;
}


cbuffer ColourBuffer : register(b1)
{
	float3 matColour;
}


struct STriVertex
{
	float4 vertex;
	float3 normal;
	float padding;
};

StructuredBuffer<STriVertex> BTriVertex : register(t1);
StructuredBuffer<int> indices: register(t2);


float3 linearToSrgb(float3 c)
{
	// Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
	float3 sq1 = sqrt(c);
	float3 sq2 = sqrt(sq1);
	float3 sq3 = sqrt(sq2);
	float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
	return srgb;
}

struct RayPayload
{
	float3 color;
};



struct ShadowPayload
{
	bool hit;
};

[shader("raygeneration")]
void rayGen()
{
	uint3 launchIndex = DispatchRaysIndex();
	uint3 launchDim = DispatchRaysDimensions();

	float2 crd = float2(launchIndex.xy);
	float2 dims = float2(launchDim.xy);

	float2 d = ((crd / dims) * 2.f - 1.f);
	float aspectRatio = dims.x / dims.y;

	// #DXR Extra: Perspective Camera
	// Perspective
	RayDesc ray;
	ray.Origin = mul(viewI, float4(0, 0, 0, 1));
	float4 target = mul(projectionI, float4(d.x, -d.y, 1, 1));
	ray.Direction = mul(viewI, float4(target.xyz, 0));
	
	/*
	RayDesc ray;
	ray.Origin = float3(0, 0, -2);
	ray.Direction = normalize(float3(d.x * aspectRatio, -d.y, 1));
	*/
	ray.TMin = 0;
	ray.TMax = 100000;

	RayPayload payload;
	payload.color = float3(0,0,0);
	TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
	float3 col = linearToSrgb(payload.color);
	gOutput[launchIndex.xy] = float4(col, 1);
}





float random (in float2 st) {
    return frac(sin(dot(st.xy,
                         float2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in float2 st) {
    float2 i = floor(st);
    float2 f = frac(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return lerp(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in float2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}


//From the book of shaders.
float3 SkyboxColour(float3 rayDir)
{
	rayDir.z *= -1;
	
	float3 outColour;
	float2 st = rayDir.xy;

	float3 color = float3(0.0, 0, 0);

	float2 q = float2(0., 0);
	q.x = fbm(st + 0.00 * 1);
	q.y = fbm(st + float2(1.0, 1));

	st = rayDir.zy;


	float2 r = float2(0., 0);
	r.x = fbm(st + 1.0 * q + float2(1.7, 9.2) + 0.15 * 1);
	r.y = fbm(st + 1.0 * q + float2(8.3, 2.8) + 0.126 * 1);

	float f = fbm(st + r);

	color = lerp(float3(1, 0.019608, 1),
		float3(0.666667, 0.666667, 0.498039),
		clamp((f * f) * 4.0, 0.0, 1.0));

	color = lerp(color,
		float3(0, 0, 0.164706),
		clamp(length(q), 0.0, 1.0));

	color = lerp(color,
		float3(0.666667, 1, 1),
		clamp(length(r.x), 0.0, 1.0));

	outColour = float4((f * f * f + .6 * f * f + .5 * f) * color, 1.);
	return outColour;
}





[shader("miss")]
void miss(inout RayPayload payload)
{
	payload.color = SkyboxColour(normalize(WorldRayDirection()));
}



[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	uint vertId = PrimitiveIndex() * 3;
	float3 hitnormal = BTriVertex[indices[vertId + 0]].normal.xyz * barycentrics.x +
		BTriVertex[indices[vertId + 1]].normal.xyz * barycentrics.y +
		BTriVertex[indices[vertId + 2]].normal.xyz * barycentrics.z;



	float hitT = RayTCurrent();
	float3 rayDirW = WorldRayDirection();
	float3 rayOriginW = WorldRayOrigin();

	// Find the world-space hit position
	float3 posW = rayOriginW + hitT * rayDirW;
	// Fire a shadow ray. The direction is hard-coded here, but can be fetched from a constant-buffer
	
	RayDesc ray;
	ray.Origin = posW;
	ray.Direction = normalize(reflect(rayDirW, hitnormal)); //normalize(float3(0.25, 0.5, -0.35));
	ray.TMin = 0.001;
	ray.TMax = 100000;
	ShadowPayload shadowPayload;
	
	if (payload.color.r > -1)
	{
		payload.color.r = -1;
		TraceRay(gRtScene, 0  /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);

	}



		//ray.Origin = posW;
		//ray.Direction = normalize(float3(-0.25, 0.5, -0.35));
		//ray.TMin = 0.001;
		//ray.TMax = 100000;
		//ShadowPayload shadowPayload2;
		//TraceRay(gRtScene, 0  /*rayFlags*/, 0xFF, 1 /* ray index*/, 0, 1, ray, shadowPayload2);

		//ray.Origin = posW;
		//ray.Direction = normalize(float3(0.25, 0.5, 0.35));
		//ray.TMin = 0.001;
		//ray.TMax = 100000;
		//ShadowPayload shadowPayload3;
		//TraceRay(gRtScene, 0  /*rayFlags*/, 0xFF, 1 /* ray index*/, 0, 1, ray, shadowPayload3);

		//ray.Origin = posW;
		//ray.Direction = normalize(float3(-0.25, 0.5, -0.35));
		//ray.TMin = 0.001;
		//ray.TMax = 100000;
		//ShadowPayload shadowPayload4;
		//TraceRay(gRtScene, 0  /*rayFlags*/, 0xFF, 1 /* ray index*/, 0, 1, ray, shadowPayload4);

		//float factor = 1;//shadowPayload.hit ? 0.1 : 1.0;
		//factor *= shadowPayload2.hit ? 0.1 : 1.0;
		/*factor *= shadowPayload3.hit ? 0.1 : 1.0;
		factor *= shadowPayload4.hit ? 0.1 : 1.0;
		*/

		float factor = 1;
		float colour = saturate(dot(hitnormal, ray.Direction));

		if (matColour.r < 0)
		{
			float3 funColour = (1).rrr - pow(SkyboxColour(hitnormal), 0.5f);
			funColour.yz *= 0.25;

			payload.color = lerp(payload.color, colour * factor * funColour, 0.5f);
		}
		else
		{
			payload.color = lerp(payload.color, colour * factor * matColour, 0.9f);
		}
	
}


[shader("closesthit")]
void grid(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float hitT = RayTCurrent();
	float3 rayDirW = WorldRayDirection();
	float3 rayOriginW = WorldRayOrigin();

	// Find the world-space hit position
	float3 posW = rayOriginW + hitT * rayDirW;

	float x = sin(posW.x * 10);
	float z = sin(posW.y * 10);



	payload.color = float3(x * z, 0, x * z);
}


//Any hit would be faster but it's currently disabled in the TLAS
[shader("closesthit")]
void shadowChs(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.hit = true;
}

[shader("miss")]
void shadowMiss(inout ShadowPayload payload)
{
	payload.hit = false;
}

//FOR WHEN WE IMPLEMENT INDEX BASED 
//https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial/Extra/dxr_tutorial_extra_indexed_geometry