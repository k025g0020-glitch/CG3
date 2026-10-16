#pragma once
#include <string>
#include "GraphicsTypes.h"

class ModelLoader {
public:
	static ModelData LoadObj(const std::string& directoryPath, const std::string& filename);
private:
	static MaterialData LoadMaterial(const std::string& directoryPath, const std::string& filename);
};
