
//////////////////////////////////////////////////////////////////////////////////////////////
/*
  ________   _________ ______ _____  _   _          _        _____       _______                  _____ _______ _____  _    _  _____ _______ _____ 
 |  ____\ \ / /__   __|  ____|  __ \| \ | |   /\   | |      |  __ \   /\|__   __|/\        _     / ____|__   __|  __ \| |  | |/ ____|__   __/ ____|
 | |__   \ V /   | |  | |__  | |__) |  \| |  /  \  | |      | |  | | /  \  | |  /  \     _| |_  | (___    | |  | |__) | |  | | |       | | | (___  
 |  __|   > <    | |  |  __| |  _  /| . ` | / /\ \ | |      | |  | |/ /\ \ | | / /\ \   |_   _|  \___ \   | |  |  _  /| |  | | |       | |  \___ \ 
 | |____ / . \   | |  | |____| | \ \| |\  |/ ____ \| |____  | |__| / ____ \| |/ ____ \    |_|    ____) |  | |  | | \ \| |__| | |____   | |  ____) |
 |______/_/ \_\  |_|  |______|_|  \_\_| \_/_/    \_\______| |_____/_/    \_\_/_/    \_\         |_____/   |_|  |_|  \_\\____/ \_____|  |_| |_____/ 
                                                                                                                                                   
*/
/////////////////////////////////////////////////////////////////////////////////////////////


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

#define albedo = matColour



cbuffer ColourBuffer : register(b1)
{
    float3 matColour; // aka albedo
    float pad1;
    float3 specularColour;
    float pad2;
    float specularPower;
}

cbuffer MetalBuffer : register(b2)
{
    float shine;
    float3 pad3;
    float scatter;
    float3 pad4;
}

struct SpotLight
{
	float4 position;
};

cbuffer WorldBuffer : register(b3)
{
    float3 sunDir;
    float pad5;
    float3 sunColour;
    float pad6;
    float time;
	float3 pad7;
	
	SpotLight  spotLights[5];
	int lightCount;
}


cbuffer TransformBuffer : register(b4)
{
	float4x4 transform;
}

cbuffer RaytraceSettingsBuffer : register(b5)
{
	int recursionLimit;
	int sampleCount;
}


struct STriVertex
{
    float4 vertex;
    float3 normal;
    float padding;
};

StructuredBuffer<STriVertex> BTriVertex : register(t1);
StructuredBuffer<int> indices : register(t2);

struct RayPayload
{
    float3 color;
};


struct ShadowPayload
{
    float hit;
};


struct SphereAttribs
{
	float3 normal;
};

float Random(float2 p)
{
	return frac(sin(dot(p.xy, float2(12.9898, 78.233))) * 31276.5122371);
}

float PerlinNoise(float2 p)
{
	float2 grid = floor(p);
	float2 f = frac(p);

	float2 uv = f * f * (3 - 2 * f);
	float n1, n2, n3, n4;
	n1 = Random(grid);
	n2 = Random(grid + float2(1, 0));
	n3 = Random(grid + float2(0, 1));
	n4 = Random(grid + float2(1, 1));

	n1 = lerp(n1, n2, uv.x);
	n2 = lerp(n3, n4, uv.x);
	n1 = lerp(n1, n2, uv.y);
	return n1;//2*(2.0*n1 - 1.0);

}

float SmoothPerlin(float2 p)
{
	float val = PerlinNoise(p);
	float val2 = PerlinNoise(p + float2(10, 0));
	val = lerp(val, val2, 0.5);

	return val;
}

//////////////////////////////////////////////////////////////////////////////////
/*
  ______ _    _ _   _  _____ _______ _____ ____  _   _  _____ 
 |  ____| |  | | \ | |/ ____|__   __|_   _/ __ \| \ | |/ ____|
 | |__  | |  | |  \| | |       | |    | || |  | |  \| | (___  
 |  __| | |  | | . ` | |       | |    | || |  | | . ` |\___ \ 
 | |    | |__| | |\  | |____   | |   _| || |__| | |\  |____) |
 |_|     \____/|_| \_|\_____|  |_|  |_____\____/|_| \_|_____/ 
                                                              
*/
//////////////////////////////////////////////////////////////////////////////////

float random(in float2 st)
{
    return frac(sin(dot(st.xy,
		float2(12.9898, 78.233))) *
		43758.5453123);
}

float3 linearToSrgb(float3 c)
{
	// Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in float2 st)
{
    float2 i = floor(st);
    float2 f = frac(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return lerp(a, b, u.x) +
            (c - a) * u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm(in float2 st)
{
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++)
    {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}


//From the book of shaders.
float3 SkyboxColour(float3 rayDir, float t)
{
    rayDir.z *= -1;
	
    float3 outColour;
    float2 st = rayDir.xy;

    float3 color = float3(0.0, 0, 0);

    float2 q = float2(0., 0);
    q.x = fbm(st + 0.00 * t);
    q.y = fbm(st + float2(1.0, 1));

    st = rayDir.zy;


    float2 r = float2(0., 0);
    r.x = fbm(st + 1.0 * q + float2(1.7, 9.2) + 0.15 * t);
    r.y = fbm(st + 1.0 * q + float2(8.3, 2.8) + 0.126 * t);

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


//Raytracing in a weekend
float3 RandomUnitInSphere(float seed)
{
    float3 p;

    float x = random(float2(seed, seed * 10));
    float y = random(float2(seed * 10, 5216));
    float z = random(float2(1231, seed * 10));
    
    float3 dir = (2.0 * float3(x, y, z)) - float3(1, 1, 1);

    if (dot(dir, dir) >= 1)
    {
        dir = normalize(dir) * 0.999;
    }

    p = dir;
    return p;
}

float3 GetHitNormal(int vertId, float3 barycentrics)
{
    float3 hitnormal = BTriVertex[indices[vertId + 0]].normal.xyz * barycentrics.x +
		BTriVertex[indices[vertId + 1]].normal.xyz * barycentrics.y +
		BTriVertex[indices[vertId + 2]].normal.xyz * barycentrics.z;

	return hitnormal = normalize(mul(float3(hitnormal), WorldToObject3x4()).xyz);
}

float3 GetWorldHitPosition()
{
    float hitT = RayTCurrent();
    float3 rayDirW = WorldRayDirection();
    float3 rayOriginW = WorldRayOrigin();
    float3 posW = rayOriginW + hitT * rayDirW;
    return posW;
}

float3 RandomInDisk(float seed)
{
	float3 p = 2*float3(random(float2(seed, seed * 421.52)), random(float2((seed + 17312) * 2.124, seed * 421.52)), 0);

	if (dot(p, p) >= 1)
	{
		p = normalize(p) * 0.999999f;
	}
	return p;
}


float shlick(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

bool Refract(float3 v, float3 n, float ni_over_nt, inout float3 refracted)
{
	float3 uv = normalize(v);
	float dt = dot(uv, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
	{
		return false;
	}

}


ShadowPayload FireShadowRay(float3 origin, float3 dir)
{
    ShadowPayload payload;
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.0001;
    ray.TMax = 100000;
    ShadowPayload shadowPayload;
    TraceRay(gRtScene, 0, 0xFF, 1, 0, 1, ray, shadowPayload);
    return shadowPayload;
}



//////////////////////////////////////////////////////////////////////////////
/*
  _____        __     __   _____ ______ _   _    _____ _    _          _____  ______ _____   _____ 
 |  __ \     /\\ \   / /  / ____|  ____| \ | |  / ____| |  | |   /\   |  __ \|  ____|  __ \ / ____|
 | |__) |   /  \\ \_/ /  | |  __| |__  |  \| | | (___ | |__| |  /  \  | |  | | |__  | |__) | (___  
 |  _  /   / /\ \\   /   | | |_ |  __| | . ` |  \___ \|  __  | / /\ \ | |  | |  __| |  _  / \___ \ 
 | | \ \  / ____ \| |    | |__| | |____| |\  |  ____) | |  | |/ ____ \| |__| | |____| | \ \ ____) |
 |_|  \_\/_/    \_\_|     \_____|______|_| \_| |_____/|_|  |_/_/    \_\_____/|______|_|  \_\_____/ 
                                                                                                   
*/
//////////////////////////////////////////////////////////////////////////////

[shader("raygeneration")]
void rayGen()
{


    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
	/*
	if (length(((float3)launchIndex / (float3)launchDim).xy - 0.5) > 0.5)
	{
		return;
	}
	*/

	// #DXR Extra: Perspective Camera
	// Perspective
    float3 col = float3(0, 0, 0);


	

    for (int i = 0; i < sampleCount; i++)
    {
		
        float2 crd = float2(launchIndex.xy + float2(random(float2(launchIndex.x, launchIndex.y+ 43.135 * i)), random(float2(launchIndex.x + 43.135 * i, launchIndex.y + 24))));
        float2 dims = float2(launchDim.xy);

        float2 d = ((crd / dims) * 2.f - 1.f);
        float aspectRatio = dims.x / dims.y;


        RayDesc ray;
        ray.Origin = mul(viewI, float4(0, 0, 0, 1));
        float4 target = mul(projectionI, float4(d.x, -d.y, 1, 1));
        ray.Direction = normalize(mul(viewI, float4(target.xyz, 0)));

		
        ray.TMin = 0;
        ray.TMax = 100000;

        RayPayload payload;
        payload.color = float3(recursionLimit, 0, 0);
        TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
        col += payload.color;
    }

    col /= sampleCount;

	//col = sqrt(col);
    col = linearToSrgb(col);
    gOutput[launchIndex.xy] = float4(col, 1);


}







//////////////////////////////////////////////////////////////////////////////////////////////////
/*
  _    _ _____ _______    _____ _    _          _____  ______ _____   _____ 
 | |  | |_   _|__   __|  / ____| |  | |   /\   |  __ \|  ____|  __ \ / ____|
 | |__| | | |    | |    | (___ | |__| |  /  \  | |  | | |__  | |__) | (___  
 |  __  | | |    | |     \___ \|  __  | / /\ \ | |  | |  __| |  _  / \___ \ 
 | |  | |_| |_   | |     ____) | |  | |/ ____ \| |__| | |____| | \ \ ____) |
 |_|  |_|_____|  |_|    |_____/|_|  |_/_/    \_\_____/|______|_|  \_\_____/ 

*/
//////////////////////////////////////////////////////////////////////////////////////////////////

[shader("closesthit")]
 void chs(inout  RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r -= 1;


    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    uint vertId = PrimitiveIndex() * 3;
   
	float3 hitnormal = GetHitNormal(vertId, barycentrics);

    //float hitT = RayTCurrent();
    //float3 rayDirW = WorldRayDirection();
    //float3 rayOriginW = WorldRayOrigin();

	// Find the world-space hit position
	float3 posW = GetWorldHitPosition();

	
    float seed = random(posW.xy) + random(posW.yz) + random(posW.zx);
	
	
	//Shadow ray
	float softnessScatter = 0.02f;

	float factor = 1; 
	ShadowPayload shadowPayload;
	if (payload.color.r > 0)
	{
		shadowPayload = FireShadowRay(posW, sunDir + RandomUnitInSphere(seed) * softnessScatter);
		factor = shadowPayload.hit;
	}

    RayDesc ray;
    ray.Origin = posW;
    ray.Direction = float3(1, 1, 1);
    ray.TMin = 0.01;
    ray.TMax = 100000;

	//AO ray
    ray.Origin = posW;
    ray.TMin = 1;
    ray.TMax = 0.005;

	if(payload.color.r > 0)
    for (int i = 0; i < 1; ++i)
    {
        float3 rndRayDir = RandomUnitInSphere(seed * (i + 1));

        if (dot(rndRayDir, hitnormal) < 0)
        {
            rndRayDir *= -1;
        }

        ray.Direction = normalize(hitnormal + rndRayDir);
        TraceRay(gRtScene, 0, 0xFF, 1, 0, 1, ray, shadowPayload);

        factor *= shadowPayload.hit;
    }


	//float factor = 1;
    float3 lightColour = saturate(max(0.1, dot(hitnormal, sunDir))) * sunColour;
	float3 lightSpecular = saturate(pow(dot(reflect(sunDir, hitnormal), WorldRayDirection()), specularPower)) * sunColour;

	
	/*if (payload.color.r > 0)
	{
		float raydepth = payload.color.r;

		for (int i = 0; i < 1; i++)
		{
			ray.Origin = posW;
			ray.Direction = hitnormal + RandomUnitInSphere(seed * (i + 1 + raydepth)) * 0.25;

			//AO ray
			ray.Origin = posW;
			ray.TMin = 0.001;
			ray.TMax = 5;

			TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		}

		payload.color /= 10.f;
	}*/

    if (matColour.r < 0)
    {
        float3 funColour = (1).rrr - pow(SkyboxColour(hitnormal, time), 0.5f);
        funColour.yz *= 0.25;

        payload.color = lightColour * funColour + (lightSpecular * specularColour) *  factor;
    }
	else if (matColour.r > 1)
	{
		payload.color = matColour;
	}
    else
    {
        payload.color = (lightColour * matColour + (lightSpecular * specularColour)) * factor;
    }

    float3 rndDir = normalize(hitnormal + RandomUnitInSphere(seed) * 0.05);
    float3 sky = SkyboxColour(rndDir, 1);
    sky *= lerp(float3(1, 1, 1), float3(0, 0, 1), 1 - pow(1 - abs(dot(rndDir, float3(0, 1, 0))), 5));

    payload.color = saturate(lerp(payload.color, sky, 0.1)) * factor;

    //payload.color = factor;

    //Test specular

    //payload.color = specular.rrr;
}





float3 TerrainColour(float3 pos, float height)
{

	float3 col;

	//pos.z += time.x * 20;
	float noise = PerlinNoise(pos.xz * 0.5);

	//Grass
	float3 baseGrass = float3(0, .5 + (PerlinNoise(pos.xz * 0.1)) * .5, 0);
	float3 smallGrass = float3(0, .8 + (PerlinNoise(pos.xz * 10)) * .1, 0);
	col = baseGrass * smallGrass * .75;

	//Stone
	float3 baseStone = float3((0.3 + 0.02 * pow(PerlinNoise(pos.xz * 0.45), 2)).rrr);

	col = lerp(col, baseStone, smoothstep(0, 31 + noise, height + 2 * PerlinNoise(pos.xz * 0.3)));


	float snowStrength = smoothstep(3, 20, height);

	//Snow
	float3 baseSnow = float3(1, 1, 1) * (pow(PerlinNoise(pos.xz * 6), 5) + (snowStrength * (1 - pow(PerlinNoise(pos.xz * 2), height))));

	baseSnow = clamp(baseSnow, 0.3, 1);

	//col = lerp(col, baseSnow, smoothstep(10, 15, height + 5 * pow(PerlinNoise(-pos.xz * 0.7), 2)));



	return col;

	if (height > 12)
	{
		col = float3(1, 1, 1);

	}
	else
	{
		col = float3(0.3, 0.3, 0.3);
	}
	return col;
}

[shader("closesthit")]
void TerrainCHS(inout  RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r -= 1;


	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	// Find the world-space hit position
	float3 posW = GetWorldHitPosition();

	float seed = random(posW.xy) + random(posW.yz) + random(posW.zx);

	hitnormal += RandomUnitInSphere(seed) * 0.1;
	//Shadow ray
	float softnessScatter = 0;//0.02f;

	float factor = 1; 

	if (payload.color.r > 0)
	{
		ShadowPayload shadowPayload = FireShadowRay(posW, sunDir + RandomUnitInSphere(seed) * softnessScatter);
		factor = shadowPayload.hit;
	}

	RayDesc ray;
	ray.Origin = posW;
	ray.Direction = float3(1, 1, 1);
	ray.TMin = 1;
	ray.TMax = 100000;

	//AO ray
	ray.Origin = posW;
	ray.TMin = 1;
	ray.TMax = 0.005;


	//float factor = 1;
	float3 lightColour = saturate(max(0.0, dot(hitnormal, sunDir))) * sunColour * step(dot(sunDir, float3(0, 1, 0)), 1.);
	float3 lightSpecular = saturate(pow(dot(reflect(sunDir, hitnormal), WorldRayDirection()), specularPower)) * sunColour;

	//Texturing

	posW.y += fbm(posW.xz);

	float3 finalColour = TerrainColour(posW, posW.y);
	
	payload.color = lightColour * factor * finalColour;
	
}


[shader("closesthit")]
void metal (inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.color.r -= 1;

    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    uint vertId = PrimitiveIndex() * 3;
	float3 hitnormal = GetHitNormal(vertId, barycentrics);


	// Find the world-space hit position
	float3 posW = GetWorldHitPosition();
    float seed = dot(posW, posW);

    RayDesc ray;
    ray.Origin = posW;
    ray.Direction = reflect(WorldRayDirection(), hitnormal) + RandomUnitInSphere(seed) * scatter;

    ray.TMin = 0.001;
    ray.TMax = 100000;
	

	//Reflection ray
    if (payload.color.r > 0 && dot(ray.Direction, hitnormal))
    {
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);

		payload.color *= matColour;
    }
    else
    {
        payload.color = float3(0, 0, 0);
    }


	/*
//Shadow ray
	float softnessScatter = 0.02f;

	ShadowPayload shadowPayload = FireShadowRay(posW, normalize(sunDir + RandomUnitInSphere(seed) * softnessScatter));
	float factor = shadowPayload.hit;


   

//AO ray
        ray.Origin = posW;
        ray.TMin = 0.001;
        ray.TMax = 0.15;

	/*
    for (int i = 0; i < 1 && shadowPayload.hit == false; ++i)
    {
        ray.Direction = normalize(hitnormal + RandomUnitInSphere(seed * (i + 1))); //normalize(reflect(rayDirW, hitnormal)); //normalize(float3(0.25, 0.5, -0.35));
       // TraceRay(gRtScene, 0, 0xFF, 1, 0, 1, ray, shadowPayload);

        //factor *= shadowPayload.hit ? 0.1 : 1.0;
    }*/


    //float colour = saturate(dot(hitnormal, sunDir));
    
   // payload.color = factor * lerp(payload.color, colour * matColour, 0.25);
	//payload.color = lerp(payload.color, colour * factor * matColour, 0.75);

}

float3 rippleNormal(float2 uv, float scale)
{
	float r = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * 0.02f)), scale) * pow(fbm(uv - float2(time, time * 0.82f)), scale));
	float g = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * 0.02f) + float2(-0.24, .52)), scale) * pow(fbm(uv + float2(0.52, 1) - float2(time, time * 0.82f)), scale));
	float b = 1;//pow(fbm(-uv + float2(time, time * 0.02f)), 5) * pow(fbm(-uv - float2(time, time * 0.82f)), 5);


	float3 bumpMap = (float3(r, g, b) * 2) - 1;
	float3 normal = bumpMap.x * float3(0, 0, 1) + bumpMap.y * float3(1, 0, 0) + bumpMap.z * float3(0, 1, 0);
	return normalize(normal);
}

[shader("closesthit")]
void rippleSurface(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float hitT = RayTCurrent();
	float3 rayDirW = WorldRayDirection();
	float3 rayOriginW = WorldRayOrigin();

	// Find the world-space hit position
	float3 posW = rayOriginW + hitT * rayDirW;

	

	float2 uv = posW.xz * 0.5;
	float3 normal = rippleNormal(uv, 2.5);
	


	//normal = float3(0, 1, 0);
	payload.color.r -= 1;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	uint vertId = PrimitiveIndex() * 3;
	float3 hitnormal = normal;/*BTriVertex[indices[vertId + 0]].normal.xyz * barycentrics.x +
		BTriVertex[indices[vertId + 1]].normal.xyz * barycentrics.y +
		BTriVertex[indices[vertId + 2]].normal.xyz * barycentrics.z;
		*/
	// payload.color = hitnormal;
	//payload.color = float3(1,1,1);
	//return;
	
	float seed = dot(posW, posW);

	RayDesc ray;
	ray.Origin = posW;
	ray.Direction = reflect(rayDirW + RandomUnitInSphere(seed) * scatter, hitnormal);
	ray.TMin = 0.1;
	ray.TMax = 100000;

	float colour = saturate(dot(hitnormal, sunDir));
	//Reflection ray
	if (payload.color.r > 0)
	{
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		payload.color *= matColour;
	}
	else
	{
		payload.color = float3(0, 0, 0);
	}

	//Shadow ray
		//ShadowPayload shadowPayload = FireShadowRay(posW, sunDir);
		//float factor = shadowPayload.hit ? 0.1 : 1;


	//Shadow ray
	float softnessScatter = 0.02f;
	float factor = 1;

	if (payload.color.r > 0)
	{
		ShadowPayload shadowPayload = FireShadowRay(posW, normalize(sunDir + RandomUnitInSphere(seed) * softnessScatter));
		factor = shadowPayload.hit;
	}

	//AO ray
	ray.Origin = posW;
	ray.TMin = 0.001;
	ray.TMax = 0.15;

	/*
	for (int i = 0; i < 1 && shadowPayload.hit == false; ++i)
	{
		ray.Direction = normalize(hitnormal + RandomUnitInSphere(seed * (i + 1))); //normalize(reflect(rayDirW, hitnormal)); //normalize(float3(0.25, 0.5, -0.35));
	   // TraceRay(gRtScene, 0, 0xFF, 1, 0, 1, ray, shadowPayload);

		//factor *= shadowPayload.hit ? 0.1 : 1.0;
	}*/


	//float factor = 1;

	
	payload.color = factor * lerp(payload.color, matColour, 0);


	//payload.color = float3(r,g,b);
}

[shader("closesthit")] 
void grid (inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r -= 1;
    float hitT = RayTCurrent();
    float3 rayDirW = WorldRayDirection();
    float3 rayOriginW = WorldRayOrigin();

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	uint vertId = PrimitiveIndex() * 3;
	float3 hitnormal = GetHitNormal(vertId, barycentrics);


	// Find the world-space hit position
    float3 pos = rayOriginW + hitT * rayDirW;


	float x = step(frac(pos.x), 0.9);
	float y = step(frac(pos.y), 0.9);
	float z = step(frac(pos.z), 0.9);

	float lighting = 1 - pow(1 - max(0, dot(sunDir, hitnormal)), 2);

	/*
	float3 reflectionColour = float3(1, 1, 1);
	if (payload.color.r > 1)
	{
		float seed = dot(pos, pos);

		RayDesc ray;
		ray.Origin = pos;
		ray.Direction = reflect(WorldRayDirection(), hitnormal + RandomUnitInSphere(seed) * 0.001);

		ray.TMin = 0.001;
		ray.TMax = 100000;

		

		//Reflection ray
		if (payload.color.r > 0 && dot(ray.Direction, hitnormal))
		{
			TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);

			reflectionColour = payload.color;
		}
	}*/

	payload.color = float3(x, y, z);// lerp(float3(x, y, z), reflectionColour, 0.25) * lighting ;
}

[shader("closesthit")]
void gridPuddle(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r -= 1;
	float hitT = RayTCurrent();
	float3 rayDirW = WorldRayDirection();
	float3 rayOriginW = WorldRayOrigin();
	float3 pos = rayOriginW + hitT * rayDirW;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	uint vertId = PrimitiveIndex() * 3;
	//float3 hitnormal = GetHitNormal(vertId, barycentrics);
	float2 uv = pos.xz;
	float3 hitnormal = float3(0, 1, 0); 

	// Find the world-space hit position


	float x = step(frac(pos.x), 0.9);
	float y = step(frac(pos.y), 0.9);
	float z = step(frac(pos.z), 0.9);

	float lighting = 1 - pow(1 - max(0, dot(sunDir, hitnormal)), 2);

	float3 reflectionColour = float3(1, 1, 1);
	if (payload.color.r > 1)
	{
		float seed = dot(pos, pos);
		hitnormal = rippleNormal(uv, 5);

		RayDesc ray;
		ray.Origin = pos;
		ray.Direction = reflect(WorldRayDirection(), hitnormal);//; +RandomUnitInSphere(seed) * 0.001);

		ray.TMin = 0.001;
		ray.TMax = 100000;



		//Reflection ray
		if (payload.color.r > 0 && dot(ray.Direction, hitnormal))
		{
			TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);

			reflectionColour = payload.color;
		}
	}

	payload.color = lerp(float3(x, y, z), reflectionColour, min(pow(smoothstep(SmoothPerlin(pos.xz), 0.25,0.5),2), 0.25)) * lighting;
}

//Any hit would be faster but it's currently disabled in the TLAS
[shader("closesthit")] 
void shadowChs (inout  ShadowPayload payload, in BuiltInTriangleIntersectionAttributes  attribs)
{
    payload.hit = 0;
}


[shader("closesthit")]
void translucent(inout  RayPayload payload, in BuiltInTriangleIntersectionAttributes  attribs)
{
	payload.color.r--;

	if (payload.color.r < 1) { payload.color = matColour * 0.01; return; }

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 outwardNormal;
	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = random(posW.xy) + random(posW.yz) + random(posW.zx);

	float3 rayDirN = (WorldRayDirection());
	float3 reflected = reflect(rayDirN, hitnormal);
	
	float ni_over_nt;

	float3 attenuation = float3(1, 1, 1);

	float3 refracted;
	float reflect_prob;
	float cosine;

	if (dot(rayDirN, hitnormal) > 0)
	{
		outwardNormal = -hitnormal;
		ni_over_nt = scatter;
		cosine = scatter * dot(rayDirN, hitnormal) / length(rayDirN);
	}
	else
	{
		outwardNormal = hitnormal;
		ni_over_nt = 1.0 / scatter;
		cosine = -dot(rayDirN, hitnormal) / length(rayDirN);
	}

	if (Refract(rayDirN, outwardNormal, ni_over_nt, refracted))
	{
		reflect_prob = shlick(cosine, scatter);
	}
	else
	{
		reflect_prob = 1.0;
	}

	RayDesc ray;
	ray.Origin = posW;
	ray.TMin = 0.25;
	ray.TMax = 100000;
	if (random(float2(seed * (payload.color.r + 412), seed)) < reflect_prob)
	{
		//Fire reflection ray
		ray.Direction = reflected;
	}
	else
	{
		//Fire refraction ray
		ray.Direction = refracted;

		//payload.color = float3(1, 0, 0);
	}

	TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
	payload.color *= matColour;
}

float3 rippleNormal2(float2 uv, float scale)
{
	float r = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * scale)), 1.5) * pow(fbm(uv - float2(time, time * 0.82f)), 2.5));
	float g = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * scale) + float2(-0.24, .52)), 2.5) * pow(fbm(uv + float2(0.52, 1) - float2(time, time * 0.82f)), 2.5));
	float b = 1;//pow(fbm(-uv + float2(time, time * 0.02f)), 5) * pow(fbm(-uv - float2(time, time * 0.82f)), 5);
	b = 0;



	float3 bumpMap = (float3(r, g, b) * 2) - 1;
	float3 normal = bumpMap.x * float3(0, 0, 10) + bumpMap.y * float3(10, 0, 0) + bumpMap.z * float3(0, 10, 0) * step(r, 0.5);
	return normalize(normal);
}


[shader("closesthit")]
void rippleTranslucent(inout  RayPayload payload, in BuiltInTriangleIntersectionAttributes  attribs)
{
	payload.color.r--;

	if (payload.color.r < 1) { payload.color = float3(0, 0, 0); return; }

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 outwardNormal;
	//float3 hitnormal = GetHitNormal(vertId, barycentrics);


	float3 posW = GetWorldHitPosition();
	float2 uv = posW.xz * 0.5;
	float3 hitnormal = rippleNormal2(uv, 1.5) * sign(GetHitNormal(vertId, barycentrics).y);
	float seed = random(posW.xy) + random(posW.yz) + random(posW.zx);
	

	float3 rayDirN = (WorldRayDirection());
	float3 reflected = reflect(rayDirN, hitnormal);

	float ni_over_nt;

	float3 attenuation = float3(1, 1, 1);

	float3 refracted;
	float reflect_prob;
	float cosine;

	if (dot(rayDirN, hitnormal) > 0)
	{
		outwardNormal = -hitnormal;
		ni_over_nt = scatter;
		cosine = scatter * dot(rayDirN, hitnormal) / length(rayDirN);
	}
	else
	{
		outwardNormal = hitnormal;
		ni_over_nt = 1.0 / scatter;
		cosine = -dot(rayDirN, hitnormal) / length(rayDirN);
	}

	if (Refract(rayDirN, outwardNormal, ni_over_nt, refracted))
	{
		reflect_prob = shlick(cosine, scatter);
	}
	else
	{
		reflect_prob = 1.0;
	}

	RayDesc ray;
	ray.Origin = posW;
	ray.TMin = 0.25;
	ray.TMax = 100000;
	if (random(float2(seed * (payload.color.r + 412), seed)) < reflect_prob)
	{
		//Fire reflection ray
		ray.Direction = reflected;
	}
	else
	{
		//Fire refraction ray
		ray.Direction = refracted;

		//payload.color = float3(1, 0, 0);
	}

	TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
	payload.color *= matColour;
}

struct onb
{
	float3 axis[9];
};


float3 random_cosine_direction(float seed)
{
	float r1 = random(float2(seed, seed));
	seed += 2.4124;

	float r2 = random(float2(seed, seed));

	float z = sqrt(1 - r2);

	float phi = 2 * 3.141592f * r1;
	float x = cos(phi) * 2 * sqrt(r2);
	float y = sin(phi) * 2 * sqrt(r2);

	return float3(x, y, z);
}

onb BuildFromW(float3 hitNormal)
{
	onb outONB;

	float3 a;
	outONB.axis[2] = hitNormal;
	if (abs(hitNormal.x) > 0.9) 
	{
		a = float3(0, 1, 0);
	}
	else
	{
		a = float3(1, 0, 0);
	}

	outONB.axis[1] = normalize(cross(hitNormal, a));
	outONB.axis[0] = (cross(hitNormal, outONB.axis[1]));

	return outONB;
}

float3 onbLocal(onb inOnb, float3 a)
{
	return float3(inOnb.axis[0] * a.x + inOnb.axis[1] * a.y + inOnb.axis[2] * a.z);
}

float Scattering_PDF(float3 hitNormal, RayDesc scatteredRay)
{
	float cosine = dot(hitNormal, scatteredRay.Direction);
	if (cosine < 0) cosine = 0;
	return cosine / 3.141592f;
}

/*
void Scatter(float3 worldRayHitPosition, float3 hitNormal, out RayDesc scatteredRay, out float pdf, float seed) 
{
	//onb uvw = BuildFromW(hitNormal);

	//Get random direction for firing a ray like before
	float3 target = RandomUnitInSphere(seed);


	scatteredRay.Origin = worldRayHitPosition;
	scatteredRay.Direction = normalize(target);
	scatteredRay.TMin = 0.01;
	scatteredRay.TMax = 100000;

	//return (dot product of normal against scattered direction) / pi
	pdf = dot(hitNormal, scatteredRay.Direction) / 3.141592f;
}
*/

void Scatter(float3 worldRayHitPosition, float3 hitNormal, out RayDesc scatteredRay, out float pdf, float seed)
{
	onb uvw = BuildFromW(hitNormal);
	float3 direction = onbLocal(uvw, random_cosine_direction(seed));
	scatteredRay.Origin = worldRayHitPosition;
	scatteredRay.Direction = normalize(direction);
	scatteredRay.TMin = 0.01;
	scatteredRay.TMax = 100000;

	//return (dot product of normal against scattered direction) / pi
	pdf = dot(hitNormal, scatteredRay.Direction) / 3.141592f;
}



[shader("closesthit")]
void lambertian(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r--;
	float payloadDepth = payload.color.r;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float pdf = 0;
	RayDesc ray;
	Scatter(posW, hitnormal, ray, pdf, seed);

	if (payload.color.r > 0)
	{
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		float3 colour = saturate(matColour * Scattering_PDF(hitnormal, ray) * payload.color / pdf);
		payload.color = colour;
	}
	else
	{
		payload.color = float3(0, 0, 0);
		return;
	}

}


[shader("closesthit")]
void lambertianHeavy(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r--;
	float payloadDepth = payload.color.r;
	
	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float pdf = 0;
	RayDesc ray;
	Scatter(posW, hitnormal, ray, pdf, seed);

	
	if (payload.color.r > 0)
	{
		//GI
		//payload.color.r = payloadDepth - abs(payloadDepth - 4);
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		float3 colour = saturate(matColour * Scattering_PDF(hitnormal, ray) * payload.color / pdf);

		//Spotlight
		payload.color.r = payloadDepth - abs(payloadDepth - 1);
		ray.Direction = normalize(float3(25, 0, 0) - posW) + RandomUnitInSphere(seed) * pdf;
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		payload.color *= matColour * max(0, dot(ray.Direction, hitnormal));
		payload.color += colour;
	}
	else
	{
		//float horizon = 1 - pow(1 - abs(dot(ray.Direction, float3(0, 1, 0))), 5);
		//payload.color = lerp(float3(1, 1, 1), float3(.5, .5, 1), horizon);
		payload.color = float3(0,0,0);
		return;
	}
}

[shader("closesthit")]
void lambertianPointLighting(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r--;
	float payloadDepth = payload.color.r;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float pdf = 0;
	RayDesc ray;
	Scatter(posW, hitnormal, ray, pdf, seed);

	//ray.Direction = normalize(float3(25, 0, 0) - posW) + RandomUnitInSphere(seed) * pdf;// +(pdf * randomInUnitSphere(seed));

	if (payload.color.r > 0)
	{
		ray.Direction = normalize(spotLights[0].position.xyz - posW) + RandomUnitInSphere(seed) * pdf;// +(pdf * randomInUnitSphere(seed));
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		payload.color *= matColour * max(0, dot(ray.Direction, hitnormal));// payload.color += colour;*/
	}
	else
	{
		//float horizon = 1 - pow(1 - abs(dot(ray.Direction, float3(0, 1, 0))), 5);
		//payload.color = lerp(float3(1, 1, 1), float3(.5, .5, 1), horizon);
		payload.color = float3(0, 0, 0);
		return;
	}

}



[shader("closesthit")]
void emissive(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color = matColour;
}

[shader("closesthit")]
void AO_OneShot(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float payloadDepth = payload.color.r;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float pdf = 0;
	RayDesc ray;
	ray.TMin = 0.01;
	ray.TMax = 10000;
	ray.Origin = posW;
	ray.Direction = RandomUnitInSphere(seed) + hitnormal;


	if (payload.color.r > 0)
	{
		ShadowPayload sPayload;
		TraceRay(gRtScene, 0, 0xFF, 1, 0, 1, ray, sPayload);
		payload.color = float3(1, 1, 1) * sPayload.hit;
	}
	else
	{
		payload.color = float3(0, 0, 0);
		return;
	}
	//payload.color = matColour;
}

[shader("closesthit")]
void AO_OneShot_IgnoreCHS(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float payloadDepth = payload.color.r;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint vertId = PrimitiveIndex() * 3;

	float3 hitnormal = GetHitNormal(vertId, barycentrics);

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float pdf = 0;
	RayDesc ray;
	ray.TMin = 0.01;
	ray.TMax = 10000;
	ray.Origin = posW;
	ray.Direction = RandomUnitInSphere(seed) + hitnormal;
	

	if (payload.color.r > 0)
	{
		ShadowPayload sPayload;
		//sPayload.hit = 0.25;
		TraceRay(gRtScene, RAY_FLAG_FORCE_OPAQUE
			| RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
			| RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, ray, sPayload);
		payload.color = float3(1, 1, 1) * sPayload.hit;
	}
	else
	{
		payload.color = float3(0, 0, 0);
		return;
	}
	//payload.color = matColour;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  __  __ _____  _____ _____    _____ _    _          _____  ______ _____   _____ 
 |  \/  |_   _|/ ____/ ____|  / ____| |  | |   /\   |  __ \|  ____|  __ \ / ____|
 | \  / | | | | (___| (___   | (___ | |__| |  /  \  | |  | | |__  | |__) | (___  
 | |\/| | | |  \___ \\___ \   \___ \|  __  | / /\ \ | |  | |  __| |  _  / \___ \ 
 | |  | |_| |_ ____) |___) |  ____) | |  | |/ ____ \| |__| | |____| | \ \ ____) |
 |_|  |_|_____|_____/_____/  |_____/|_|  |_/_/    \_\_____/|______|_|  \_\_____/ 
                                                                                 
*/                                                                                 
///////////////////////////////////////////////////////////////////////////////////////////////////////

[shader("miss")]
void miss(inout RayPayload payload)
{
	payload.color = float3(0.1, 0.1, 0.1);
}


[shader("miss")]
void missSky(inout RayPayload payload)
{
	payload.color = float3(0, 0, 0);
	//return;

	float3 unit_dir = WorldRayDirection();
	float t = 0.5 * (clamp(unit_dir.y, -1, 1) + 1.0);
	payload.color = lerp(float3(1, 1, 1), float3(0.5, 0.7, 1.0), t) * 0.1f;

	float horizon = 1 - pow(1 - abs(dot(WorldRayDirection(), float3(0, 1, 0))), 5);
	//payload.color = lerp(float3(1, 1, 1), float3(.5, .5, 1), horizon);

	

	float3 rayDirNormalized = normalize(WorldRayDirection());
    
	float brightness = dot(sunDir, float3(0, 1, 0));


	float sunAngle = dot(rayDirNormalized, normalize(sunDir));
	float sunStrength = pow(sunAngle, 100 - (saturate(1-brightness)) * 90);
	float3 sunCol = lerp(float3(.75, .4, .4), float3(1, 1, 1), abs(sunStrength)) * sunStrength;


	float3 smallClouds = SkyboxColour(rayDirNormalized * 5, time * 0.1);
	float cloudMult = smallClouds.r + smallClouds.b;
	smallClouds = lerp(float3(0.5, 1, 0.52), float3(.12, 0.031, 1), 1 - pow(1 - frac(cloudMult), 5)) * step(1, cloudMult);


	//float horizon = 1 - pow(1 - abs(dot(rayDirNormalized, float3(0, 1, 0))), 5);

	float starsDense = (pow(fbm(rayDirNormalized.xz * 500), 20))* (horizon) * (1-brightness);


	payload.color = SkyboxColour(rayDirNormalized, time * 0.01) * (sunAngle + 1);


	payload.color *= lerp(float3(1, 1, 1), float3(0, 0, 1), horizon);


	payload.color *= max(0.0025,(brightness + .25));

	payload.color += max(0.025, (brightness + .25)) * float3(smallClouds.xx, smallClouds.z);

	payload.color += max(float3(0, 0, 0), sunCol) + (starsDense * (clamp(1 - smallClouds.z, 0, 1)) * smoothstep(0.2,0.8, rayDirNormalized.y));
	

	//payload.color = float3(0, 0, 0);
}

[shader("miss")]
void skyrim(inout RayPayload payload)
{
	float3 rayDirN = normalize(WorldRayDirection());

	float horizon = smoothstep(0.2, 0.75, rayDirN.y);

	float starsDense = ( pow(fbm(rayDirN.xz * 500), 20)) * horizon;
	float3 clouds = SkyboxColour(normalize(WorldRayDirection()), time);

	float fun = clouds.r + clouds.b;
	float fun2 = sin(clouds.g + clouds.r);


	float3 aurora = lerp(float3(0.5, 1, 0.52), float3(.12, 0.031, 0.5 + 0.5 * abs(cos(time))), 1 - pow(1 - frac(fun), 5)) * step(1, fun);





	//payload.color = //starsDense * (1 - frac(clouds.r + clouds.b));	
		//float3(0, 0, step(1, fun) * frac(fun));

	payload.color = (starsDense * (rayDirN.y + 1)) + aurora;

}




[shader("miss")] 
void shadowMiss (inout ShadowPayload payload)
{
	/*float3 rayDirNormalized = normalize(WorldRayDirection());

	float3 smallClouds = SkyboxColour(rayDirNormalized * 5, time * 0.1);
	float cloudMult = smallClouds.r + smallClouds.b;
	smallClouds = lerp(float3(0.5, 1, 0.52), float3(.12, 0.031, 1), 1 - pow(1 - frac(cloudMult), 5)) * step(1, cloudMult);

    payload.hit = 1 - saturate(smallClouds.b);*/

	payload.hit = 1;

}



//Any hit would be faster but it's currently disabled in the TLAS
[shader("closesthit")]
void shadowChsB(inout  ShadowPayload payload, in SphereAttribs b)
{
	payload.hit = 0.25;
}

















/********************************/
/*Intersection shaders - Get big words or this is sad*/
bool IntersectAABB(in RayDesc ray, out float3 hitPos, float3 boxCenter, float3 boxDimensions)
{
	float3 origin = ray.Origin;
	float3 rayEnd = origin * ray.Direction * ray.TMax;
	float3 rayDir = normalize(ray.Direction);

	
	float tMin = (-boxDimensions.x - origin.x) / rayDir.x;

	float tMax = (boxDimensions.x - origin.x) / rayDir.x;

	//Swap if ordered incorrectly
	if (tMax < tMin)
	{
		float temp = tMax;
		tMax = tMin;
		tMin = temp;
	}

	float tyMin = (-boxDimensions.y - origin.y) / rayDir.y;
	float tyMax = (boxDimensions.y - origin.y) / rayDir.y;

	//Swap if ordered incorrectly
	if (tyMax < tyMin)
	{
		float temp = tyMax;
		tyMax = tyMin;
		tyMin = temp;
	}

	if ((tMin > tyMax) || (tyMin > tMax)) return false;

	if (tyMin > tMin)
		tMin = tyMin;

	if (tyMax < tMax)
		tMax = tyMax;

	//

	float tzMin = (-boxDimensions.z - origin.z) / rayDir.z;
	float tzMax = (boxDimensions.z - origin.z) / rayDir.z;

	if (tzMax < tzMin)
	{
		float temp = tzMax;
		tzMax = tzMin;
		tzMin = temp;
	}

	if ((tMin > tzMax) || (tzMin > tMax)) return false;

	if (tzMin > tMin)
		tMin = tzMin;

	if (tzMax < tMax)
		tMax = tzMax;

	//Return initial collsiion point (min)
	hitPos = tMin * rayDir + origin;

	return true;
}


[shader("intersection")]
void SphereIntersect()
{
	float3 sphereCenter = mul(ObjectToWorld3x4(), float4(0, 0, 0,1)).xyz;
	float sphereRadius = 0.5f;// +sin(time) * 1;

	float3 toCenter = WorldRayOrigin() - sphereCenter;
	float a = dot(WorldRayDirection(), WorldRayDirection());
	float b = 2.0f * dot(WorldRayDirection(), toCenter);
	float c = dot(toCenter, toCenter) - sphereRadius * sphereRadius;

	if (b * b >= 4.0f * a * c)
	{
		float sqrtVal = sqrt(b * b - 4.0f * a * c);
		SphereAttribs sphereAttr = { sphereCenter };
		ReportHit((-b - sqrtVal) / (2.0f * a), 0, sphereAttr);
		ReportHit((-b + sqrtVal) / (2.0f * a), 0, sphereAttr);
	}
}


[shader("intersection")]
void VolumetricFogIntersection()
{
	float3 posW = GetWorldHitPosition();

	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));
	float3 rayDir = normalize(WorldRayDirection());


	float3 origin = WorldRayOrigin();// -rayDir * 100000;

	float3 rayEnd = rayDir * 1000;



	//Box dimensions
	float3 dim = float3(50, 50, 50);

	float tMin = (-dim.x - origin.x) / rayDir.x;
	float tMax = (dim.x - origin.x) / rayDir.x;

	if (tMax < tMin)
	{
		float temp = tMax;
		tMax = tMin;
		tMin = temp;
	}

	float tyMin = (-dim.y - origin.y) / rayDir.y;
	float tyMax = (dim.y - origin.y) / rayDir.y;

	if (tyMax < tyMin)
	{
		float temp = tyMax;
		tyMax = tyMin;
		tyMin = temp;
	}

	if ((tMin > tyMax) || (tyMin > tMax)) return;

	if (tyMin > tMin)
		tMin = tyMin;

	if (tyMax < tMax)
		tMax = tyMax;

	//

	float tzMin = (-dim.z - origin.z) / rayDir.z;
	float tzMax = (dim.z - origin.z) / rayDir.z;

	if (tzMax < tzMin)
	{
		float temp = tzMax;
		tzMax = tzMin;
		tzMin = temp;
	}

	if ((tMin > tzMax) || (tzMin > tMax)) return;

	if (tzMin > tMin)
		tMin = tzMin;

	if (tzMax < tMax)
		tMax = tzMax;

	float density = 0.025;

	//Distance inside bounds
	tMin = max(tMin, 1);
	float distInsideBounds = (tMax - tMin);
	
	SphereAttribs sphereAttr = { float3(distInsideBounds,0,0) };


	float3 hitPos = origin + rayDir * (tMin);


	//if (hitDistance < distInsideBounds)
	{
		float hit = distance(WorldRayOrigin(), hitPos);
		if(hit > 0)
			ReportHit(hit, 0, sphereAttr);
	}

}

[shader("closesthit")]
void SphereClosestHit(inout RayPayload payload, SphereAttribs attribs)
{
	payload.color = attribs.normal;
	
	return;
	payload.color.r--;

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	float3 target = (RandomUnitInSphere(seed));


	RayDesc ray;
	ray.Origin = posW;
	ray.Direction = RandomUnitInSphere(seed);

	if (payload.color.r > 0)
	{
		ray.TMin = 0.01;
		ray.TMax = 100000;
		//ray.Direction = sunDir;
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 1, ray, payload);
		payload.color *= float3(0,0.75,0);
	}
	else
	{
		//float horizon = 1 - pow(1 - abs(dot(ray.Direction, float3(0, 1, 0))), 5);
		//payload.color = lerp(float3(1, 1, 1), float3(.5, .5, 1), horizon);
		payload.color = float3(1, 0, 0);
		return;
	}

	//payload.color = dot(-WorldRayDirection(), sunDir);
}

[shader("closesthit")]
void VolumetricClosestHit(inout RayPayload payload, SphereAttribs attribs)
{
	payload.color.r--;
	float3 posW = GetWorldHitPosition();
	
	if (payload.color.r > 0)
	{

		float shadowing = 0; //Miss will make it = 1
		float3 valToAdd = WorldRayDirection() * (attribs.normal.x / 10.f);

		RayDesc shadowRay;
		shadowRay.Origin = posW;
		shadowRay.Direction = sunDir;
		shadowRay.TMin = 0.1;//attribs.normal.r + 0.1;
		shadowRay.TMax = 10000;
		for (int i = 0; i < 10; i++)
		{
			shadowRay.Origin += valToAdd * i + valToAdd * (noise(DispatchRaysIndex().xy));

			ShadowPayload shadowPayload;
			shadowPayload.hit = 0;

			TraceRay(gRtScene, RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH| RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
				0xff, 0, 0, 1, shadowRay, shadowPayload);

			shadowing += ((1 - shadowPayload.hit) / 10.f);
		}
	
		RayDesc ray;
		ray.Origin = posW;
		ray.Direction = WorldRayDirection();
		ray.TMin = 0.1;//attribs.normal.r + 0.1;
		ray.TMax = 10000;

		TraceRay(gRtScene, RAY_FLAG_CULL_NON_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

		payload.color *= (1 - saturate(shadowing));// *= pow(1 - saturate(attribs.normal.r / 50.f), 5) + 0.01;
	}
	else
	{
		payload.color = float3(1, 1, 0);
	}


	/*

	float3 posW = GetWorldHitPosition();
	float seed = noise(DispatchRaysIndex().xy + random(posW.xy) + random(posW.yz) + random(posW.zx));

	RayDesc ray;
	ray.Origin = posW;
	ray.Direction = sunDir;//RandomUnitInSphere(seed);
	
	if (payload.color.r > 0)
	{
		//payload.color = float3(1, 0, 0);
		ray.TMin = 0.01;
		ray.TMax = 100000;
		//ray.Direction = sunDir;
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		float3 colour = payload.color;

		ray.Direction = WorldRayDirection();
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);


		//payload.color = normalize(sunDir);
		payload.color *= colour;
	}
	else
	{
		//float horizon = 1 - pow(1 - abs(dot(ray.Direction, float3(0, 1, 0))), 5);
		//payload.color = lerp(float3(1, 1, 1), float3(.5, .5, 1), horizon);
		payload.color = float3(1, 0, 0);
		return;
	}
	//payload.color = dot(-WorldRayDirection(), sunDir);

	*/
}










