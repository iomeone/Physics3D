#include "core.h"

#include "meshLibrary.h"

#include <sstream>

#include "mesh/indexedMesh.h"

#include "../engine/meshRegistry.h"
#include "../engine/io/import.h"
#include "visualShape.h"

namespace P3D::Graphics {

namespace Library {
IndexedMesh* cube = nullptr;
IndexedMesh* sphere = nullptr;

void onInit() {
	// Cube
	cube = new IndexedMesh(Engine::MeshRegistry::createCube(1));

	// Sphere
	sphere = new IndexedMesh(Engine::MeshRegistry::createSphere(1, 3));
}

void onClose() {
	cube->close();
	sphere->close();
}
}

};