#include "core.h"

#include "material.h"

#include "../graphics/texture.h"

namespace Application {

void Material::setTexture(Texture* texture) {
	texture->setUnit(0);
	this->texture = texture;
}

void Material::setNormalMap(Texture* normalMap) {
	normalMap->setUnit(1);
	this->normal = normalMap;
}

bool Material::isUnique() const {
	return true;
}

bool Material::operator==(const Material& other) const {
	return
		other.ambient == ambient &&
		other.diffuse == diffuse &&
		other.specular == specular &&
		other.reflectance == reflectance &&
		other.texture == texture &&
		other.normal == normal;
}

};