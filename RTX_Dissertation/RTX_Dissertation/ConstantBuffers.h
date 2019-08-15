#pragma once
#include "Renderer.h"

//This buffer contains light and time information
struct WorldBuffer
{
	glm::vec3 sunDir;
	glm::vec3 sunColour;

	//TODO::An array of all light sources and positions

	float time;
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