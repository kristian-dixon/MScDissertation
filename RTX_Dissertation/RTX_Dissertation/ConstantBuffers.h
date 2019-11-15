#pragma once
#include "Renderer.h"

struct PointLight
{
	glm::vec4 position;
};

//This buffer contains light and time information
struct WorldBuffer
{
	glm::vec3 sunDir;
	float pad1;
	glm::vec3 sunColour;
	float pad2;
	//TODO::An array of all light sources and positions

	float time;
	glm::vec3 pad3;

	PointLight pointLights[5];

	int pointLightCount = 0;
	//Seed?
};

//Should be applied to any object that uses basic lighting
struct MaterialBuffer
{
	glm::vec3 diffuseColour;
	float pad1;
	glm::vec3 specularColour;
	float pad2;
	float specularPower;
};

//Should be applied to reflective shaders
struct MetalBuffer
{
	float reflectivity;
	glm::vec3 padding1;
	//pretty much sets the roughness of the object.
	float scatter;
	glm::vec3 padding2;
};

struct TransformBuffer
{
	glm::mat4 transform;
	glm::mat4 transformInv;
};