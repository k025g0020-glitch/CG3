#include "ModelLoader.h"
#include <cassert>
#include <fstream>
#include <sstream>

MaterialData ModelLoader::LoadMaterial(const std::string& directoryPath, const std::string& filename) {
	MaterialData result{};
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string id;
		stream >> id;
		if (id == "map_Kd") {
			std::string name;
			stream >> name;
			result.textureFilePath = directoryPath + "/" + name;
		}
	}
	return result;
}

ModelData ModelLoader::LoadObj(const std::string& directoryPath, const std::string& filename) {
	ModelData result{};
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string id;
		stream >> id;
		if (id == "v") {
			Vector4 value{};
			stream >> value.x >> value.y >> value.z;
			value.x *= -1.0f;
			value.w = 1.0f;
			positions.push_back(value);
		} else if (id == "vt") {
			Vector2 value{};
			stream >> value.x >> value.y;
			value.y = 1.0f - value.y;
			texcoords.push_back(value);
		} else if (id == "vn") {
			Vector3 value{};
			stream >> value.x >> value.y >> value.z;
			value.x *= -1.0f;
			normals.push_back(value);
		} else if (id == "f") {
			VertexData triangle[3]{};
			for (uint32_t vertex = 0; vertex < 3; ++vertex) {
				std::string definition;
				stream >> definition;
				std::istringstream vertexStream(definition);
				uint32_t index[3]{};
				for (uint32_t element = 0; element < 3; ++element) {
					std::string number;
					std::getline(vertexStream, number, '/');
					index[element] = static_cast<uint32_t>(std::stoul(number));
				}
				triangle[vertex] = {positions[index[0] - 1], texcoords[index[1] - 1], normals[index[2] - 1]};
			}
			result.vertices.push_back(triangle[2]);
			result.vertices.push_back(triangle[1]);
			result.vertices.push_back(triangle[0]);
		} else if (id == "mtllib") {
			std::string name;
			stream >> name;
			result.material = LoadMaterial(directoryPath, name);
		}
	}
	return result;
}
