
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


cbuffer ColourBuffer : register(b1)
{
    float3 matColour;
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


cbuffer WorldBuffer : register(b3)
{
    float3 sunDir;
    float pad5;
    float3 sunColour;
    float pad6;
    float time;
}

cbuffer TransformBuffer : register(b4)
{
	float4x4 transform;
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

    return hitnormal = normalize(mul(transform, float4(hitnormal, 0)));

}

float3 GetWorldHitPosition()
{
    float hitT = RayTCurrent();
    float3 rayDirW = WorldRayDirection();
    float3 rayOriginW = WorldRayOrigin();
    float3 posW = rayOriginW + hitT * rayDirW;
    return posW;
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

	

	// #DXR Extra: Perspective Camera
	// Perspective

    float3 col = float3(0, 0, 0);

	int sampleCount = 4;
    for (int i = 0; i < sampleCount; i++)
    {
        float2 crd = float2(launchIndex.xy + float2(random(float2(0, 43.135 * i)), random(float2(43.135 * i, 24))));
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
        payload.color = float3(8, 0, 0);
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

    ShadowPayload shadowPayload = FireShadowRay(posW, sunDir + RandomUnitInSphere(seed) * softnessScatter);
    float factor = shadowPayload.hit;


    RayDesc ray;
    ray.Origin = posW;
    ray.Direction = float3(1, 1, 1);
    ray.TMin = 0.01;
    ray.TMax = 100000;

	//AO ray
    ray.Origin = posW;
    ray.TMin = 1;
    ray.TMax = 0.005;

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
    ray.Direction = reflect(WorldRayDirection() + RandomUnitInSphere(seed) * scatter, hitnormal);
    //ray.Direction = reflect(WorldRayDirection(), hitnormal + RandomUnitInSphere(seed) * scatter);

    ray.TMin = 0.1;
    ray.TMax = 100000;
	

	//Reflection ray
    if (payload.color.r > 0)
    {
		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
    }
    else
    {
        payload.color = float3(0, 0, 0);
    }

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


    float colour = saturate(dot(hitnormal, sunDir));
    
    payload.color = factor * lerp(payload.color, colour * matColour, 0.25);
	//payload.color = lerp(payload.color, colour * factor * matColour, 0.75);

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

	float r = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * 0.02f)), 2.5) * pow(fbm(uv - float2(time, time * 0.82f)), 2.5));
	float g = 0.5 + 0.5 * (pow(fbm(uv + float2(time, time * 0.02f) + float2(-0.24, .52)), 2.5) * pow(fbm(uv + float2(0.52, 1) - float2(time, time * 0.82f)), 2.5));
	float b = 1;//pow(fbm(-uv + float2(time, time * 0.02f)), 5) * pow(fbm(-uv - float2(time, time * 0.82f)), 5);


	float3 bumpMap = (float3(r, g, b) * 2) - 1;
	float3 normal = bumpMap.x * float3(0, 0, 1) + bumpMap.y * float3(1, 0, 0) + bumpMap.z * float3(0, 1, 0);
	normal = normalize(normal);
	//normal = float3(0, 1, 0);
	payload.color.r -= 1;

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	uint vertId = PrimitiveIndex() * 3;
	float3 hitnormal = normal;/*BTriVertex[indices[vertId + 0]].normal.xyz * barycentrics.x +
		BTriVertex[indices[vertId + 1]].normal.xyz * barycentrics.y +
		BTriVertex[indices[vertId + 2]].normal.xyz * barycentrics.z;
		*/
	// payload.color = hitnormal;

	
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


	//float factor = 1;

	
	payload.color = factor * lerp(payload.color, matColour, 0);


	//payload.color = float3(r,g,b);
}


[shader("closesthit")] 
void grid (inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
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
void shadowChs (inout  ShadowPayload payload, in BuiltInTriangleIntersectionAttributes  attribs)
{
    payload.hit = 0.25;
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

[shader("closesthit")]
void translucent(inout  RayPayload payload, in BuiltInTriangleIntersectionAttributes  attribs)
{
	payload.color.r--;

	if (payload.color.r < 1) { payload.color = float3(0, 0, 0); return; }

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
}


[shader("closesthit")]
void lambertian(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	payload.color.r--;

	if (payload.color.r > 0)
	{
		float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
		uint vertId = PrimitiveIndex() * 3;

		float3 hitnormal = GetHitNormal(vertId, barycentrics);

		float3 posW = GetWorldHitPosition();
		//float seed = random(posW.xy) + random(posW.yz) + random(posW.zx);

		RayDesc ray;
		ray.Origin = posW;
		ray.Direction = hitnormal;// +RandomUnitInSphere(seed) * 0.05;

		ray.TMin = 0.01;
		ray.TMax = 100000;

		TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, ray, payload);
		payload.color *= 0.5;
	}
	else
	{
		payload.color = float3(0, 0, 0);
	}
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
	float3 rayDirNormalized = normalize(WorldRayDirection());
    
	float brightness = dot(sunDir, float3(0, 1, 0));


	float sunAngle = dot(rayDirNormalized, normalize(sunDir));
	float sunStrength = pow(sunAngle, 100 - (saturate(1-brightness)) * 90);
	float3 sunCol = lerp(float3(.75, .4, .4), float3(1, 1, 1), abs(sunStrength)) * sunStrength;


	float3 smallClouds = SkyboxColour(rayDirNormalized * 5, time * 0.1);
	float cloudMult = smallClouds.r + smallClouds.b;
	smallClouds = lerp(float3(0.5, 1, 0.52), float3(.12, 0.031, 1), 1 - pow(1 - frac(cloudMult), 5)) * step(1, cloudMult);


	float horizon = 1 - pow(1 - abs(dot(rayDirNormalized, float3(0, 1, 0))), 5);

	float starsDense = (pow(fbm(rayDirNormalized.xz * 500), 20))* (horizon) * (1-brightness);


	payload.color = SkyboxColour(rayDirNormalized, time * 0.01) * (sunAngle + 1);


	payload.color *= lerp(float3(1, 1, 1), float3(0, 0, 1), horizon);


	payload.color *= max(0.0025,(brightness + .25));

	payload.color += max(0.025, (brightness + .25)) * float3(smallClouds.xx, smallClouds.z);

	payload.color += max(float3(0, 0, 0), sunCol) + (starsDense * (clamp(1 - smallClouds.z, 0, 1)) * smoothstep(0.2,0.8, rayDirNormalized.y));

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



















