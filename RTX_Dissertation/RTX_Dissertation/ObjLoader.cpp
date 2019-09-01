#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Vertex.h"
#include <map>
#include <tuple>

using namespace std;

std::shared_ptr<Mesh> ObjLoader::LoadOBJMesh(string fileName)
{
	vector<vec3> pointList;
	vector<vec3> normalList;

	vector<Vertex> vertexList;

	map<tuple<int, int>, int> vertexMap;

	vector<uint32_t> indices;
	//1979-1020
	/*try
	{*/
		//Open file
		string line;
		ifstream objFile(fileName);
		if(objFile.is_open())
		{
			while(getline(objFile,line))
			{
				auto segmentEnd = line.find(' ');
				auto type = line.substr(0, segmentEnd);

				//Check if line starts with V
				if(type == "v")
				{
					//Add to vertex list
					pointList.push_back(GetVectorFromString(line, segmentEnd));
				}
				else if(type == "vn")
				{
					normalList.push_back(GetVectorFromString(line, segmentEnd));
				}
				else if(type == "f")
				{
					//Get Normal index
					int normalIndex = 0;
					{
						auto a = line.find('/',segmentEnd) + 2;
						auto b = line.find(' ', segmentEnd + 1) ;

						string s = line.substr(a, b);
						normalIndex = (stoi(s) - 1);
					}

					//          f 1//0 4//0 6//9

					//Find first vertex point
					auto segmentStart = segmentEnd + 1;
					segmentEnd = line.find('/', segmentStart);

					int pointAIndex = stoi(line.substr(segmentStart, segmentEnd)) - 1;

					auto vertexMapIndex = vertexMap.find(std::make_tuple(pointAIndex, normalIndex));
					
					if(vertexMapIndex != vertexMap.end())
					{
						indices.push_back(vertexMapIndex->second);
					}
					else
					{
						vertexMap.insert({ make_tuple(pointAIndex, normalIndex), vertexList.size() });
						indices.push_back(vertexList.size());
						vertexList.push_back(Vertex{ vec4(pointList[pointAIndex], 0), normalList[normalIndex], 0 });
					}


					


					segmentStart = line.find(' ', segmentEnd);
					segmentEnd = line.find('/', segmentStart);

					normalIndex = 0;
					{
						auto a = line.find('/', segmentEnd) + 2;
						auto b = line.find(' ', segmentEnd + 1);

						string s = line.substr(a, b);
						normalIndex = (stoi(s) - 1);
					}

					pointAIndex = stoi(line.substr(segmentStart, segmentEnd)) - 1;

					vertexMapIndex = vertexMap.find(std::make_tuple(pointAIndex, normalIndex));

					if (vertexMapIndex != vertexMap.end())
					{
						indices.push_back(vertexMapIndex->second);
					}
					else
					{
						vertexMap.insert({ make_tuple(pointAIndex, normalIndex), vertexList.size() });
						indices.push_back(vertexList.size());
						vertexList.push_back(Vertex{ vec4(pointList[pointAIndex], 0), normalList[normalIndex], 0 });
					}
					


					segmentStart = line.find(' ', segmentEnd);
					segmentEnd = line.find('/', segmentStart);

					normalIndex = 0;
					{
						auto a = line.find('/', segmentEnd) + 2;
						auto b = line.find(' ', segmentEnd + 1);

						string s = line.substr(a, b);
						normalIndex = (stoi(s) - 1);
					}

					pointAIndex = stoi(line.substr(segmentStart, segmentEnd)) - 1;

					vertexMapIndex = vertexMap.find(std::make_tuple(pointAIndex, normalIndex));

					if (vertexMapIndex != vertexMap.end())
					{
						indices.push_back(vertexMapIndex->second);
					}
					else
					{
						vertexMap.insert({ make_tuple(pointAIndex, normalIndex), vertexList.size() });
						indices.push_back(vertexList.size());
						vertexList.push_back(Vertex{ vec4(pointList[pointAIndex], 0), normalList[normalIndex], 0 });
					}


				}


			}
		}

		auto inst = Renderer::GetInstance();
		vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertexList) };
		vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertexList.size()) };
		vector<ID3D12ResourcePtr> indexBuffers = { inst->CreateIndexBuffer(indices) };
		vector<uint32_t> indexCounts = { static_cast<uint32_t>(indices.size()) };

		auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
		return mesh;
	/*}
	catch(...)
	{
	}*/



	return nullptr;

}

glm::vec3 ObjLoader::GetVectorFromString(string& input, string::size_type offset)
{
	auto segmentStart = offset + 1;
	offset = input.find(' ', segmentStart);

	const float x = std::stof(input.substr(segmentStart, offset));

	segmentStart = offset + 1;
	offset = input.find(' ', segmentStart);

	const float y = std::stof(input.substr(segmentStart, offset));
	
	segmentStart = offset + 1;
	offset = input.find(' ', segmentStart);
	
	const float z = std::stof(input.substr(segmentStart, offset));

	return vec3(x, y, z);
}


/*
 From Wikipedia

 # List of geometric vertices, with (x, y, z [,w]) coordinates, w is optional and defaults to 1.0.
  v 0.123 0.234 0.345 1.0
  v ...
  ...
  # List of texture coordinates, in (u, [v ,w]) coordinates, these will vary between 0 and 1, v and w are optional and default to 0.
  vt 0.500 1 [0]
  vt ...
  ...
  # List of vertex normals in (x,y,z) form; normals might not be unit vectors.
  vn 0.707 0.000 0.707
  vn ...
  ...
  # Parameter space vertices in ( u [,v] [,w] ) form; free form geometry statement ( see below )
  vp 0.310000 3.210000 2.100000
  vp ...
  ...
  # Polygonal face element (see below)
  f 1 2 3
  f 3/1 4/2 5/3
  f 6/4/1 3/5/3 7/6/5
  f 7//1 8//2 9//3
  f ...
  ...
  # Line element (see below)
  l 5 8 1 2 4 9


 */