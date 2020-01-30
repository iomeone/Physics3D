#include "core.h"
#include "model.h"

#include "extendedPart.h"

namespace Application {

ExtendedPart* Model::getExtendedPart() const {
	return extendedPart;
}

void Model::setExtendedPart(ExtendedPart* extendedPart) {
	this->extendedPart = extendedPart;
}

TransformComp Model::getTransform() {
	ExtendedPart* extendedPart = getExtendedPart();

	if (extendedPart == nullptr)
		return TransformComp(GlobalCFrame());

	return TransformComp(getExtendedPart()->getCFrame());
}

void Model::setTransform(const TransformComp& transform) {
	getExtendedPart()->setCFrame(transform.getCFrame());
}

};