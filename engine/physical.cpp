#include "physical.h"

#include "../util/log.h"
#include "math/mathUtil.h"

#include "debug.h"
#include <algorithm>
#include <limits>



Physical::Physical(Part* part) : mainPart(part) {
	if (part->parent != nullptr) {
		throw "Attempting to re-add part to different physical!";
	}
	part->parent = this;
	//parts.push_back(AttachedPart{ CFrame(), part });

	refreshWithNewParts();
	//this->mass = part->mass;
	//this->inertia = part->inertia;
	//this->localCenterOfMass = part->localCenterOfMass;
	//Sphere s = part->hitbox.getCircumscribingSphere();
	//this->localCentroid = s.origin;
	//this->circumscribingSphere.radius = s.radius;
	//this->circumscribingSphere.origin = getCFrame().localToGlobal(localCentroid);
}

void Physical::makeMainPart(Part* newMainPart) {
	if (newMainPart == mainPart) {
		Log::debug("Attempted to replace mainPart with mainPart");
		return;
	}
	for (AttachedPart& atPart : parts) {
		if (atPart.part == newMainPart) {
			makeMainPart(atPart);
			return;
		}
	}

	throw "Attempting to make a part not in this physical the mainPart!";
}

void Physical::makeMainPart(AttachedPart& newMainPart) {
	if (parts.liesInList(&newMainPart)) {
		CFrame& newCenterCFrame = newMainPart.attachment;
		std::swap(mainPart, newMainPart.part);
		for (AttachedPart& atPart : parts) {
			if (&atPart != &newMainPart) {
				atPart.attachment = newCenterCFrame.globalToLocal(atPart.attachment);
			}
		}
		newCenterCFrame = ~newCenterCFrame;
	} else {
		throw "Attempting to make a part not in this physical the mainPart!";
	}
}

void Physical::attachPart(Part* part, CFrame attachment) {
	part->parent = this;

	parts.add(AttachedPart{attachment, part});

	part->cframe = getCFrame().localToGlobal(attachment);

	refreshWithNewParts();
}
void Physical::detachPart(Part* part) {
	if (part == mainPart) {
		part->parent = nullptr;
		if (parts.size >= 1) {
			makeMainPart(parts[parts.size - 1]);
			refreshWithNewParts();
			return;
		} else {
			this->~Physical();
		}
	} else {
		for (auto iter = parts.begin(); iter != parts.end(); ++iter) {
			AttachedPart& at = *iter;
			if (at.part == part) {
				part->parent = nullptr;
				parts.remove(iter);
				refreshWithNewParts();
				return;
			}
		}
	}

	throw "Error: could not find part to remove!";
}

void Physical::refreshWithNewParts() {
	double totalMass = mainPart->mass;
	SymmetricMat3 totalInertia = mainPart->inertia;
	Vec3 totalCenterOfMass = mainPart->localCenterOfMass;
	for (const AttachedPart& p : parts) {
		totalMass += p.part->mass;
		totalCenterOfMass += p.attachment.localToGlobal(p.part->localCenterOfMass) * p.part->mass;
	}
	totalCenterOfMass /= totalMass;
	for (const AttachedPart& p : parts) {
		totalInertia += transformBasis(p.part->inertia, p.attachment.getRotation()) + skewSymmetricSquared(p.attachment.getPosition() - totalCenterOfMass) * p.part->mass;
	}
	this->mass = totalMass;
	this->localCenterOfMass = totalCenterOfMass;
	this->inertia = totalInertia;

	Sphere s = getLocalCircumscribingSphere();
	this->localCentroid = s.origin;
	this->circumscribingSphere.radius = s.radius;
	this->circumscribingSphere.origin = getCFrame().localToGlobal(localCentroid);
}

BoundingBox Physical::getLocalBounds() const {
	const double inf = std::numeric_limits<double>::infinity();

	BoundingBox best(mainPart->hitbox.getBounds());
	//BoundingBox best(inf, inf, inf, -inf, -inf, -inf);


	for (const AttachedPart& p : parts) {
		double xmax = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(1, 0, 0)))).x;
		double xmin = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(-1, 0, 0)))).x;
		double ymax = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(1, 0, 0)))).y;
		double ymin = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(-1, 0, 0)))).y;
		double zmax = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(1, 0, 0)))).z;
		double zmin = p.attachment.localToGlobal(p.part->hitbox.furthestInDirection(p.attachment.relativeToLocal(Vec3(-1, 0, 0)))).z;

		best.xmax = std::max(best.xmax, xmax);
		best.xmin = std::min(best.xmin, xmin);
		best.ymax = std::max(best.ymax, ymax);
		best.ymin = std::min(best.ymin, ymin);
		best.zmax = std::max(best.zmax, zmax);
		best.zmin = std::min(best.zmin, zmin);
	}

	return best;
}
Sphere Physical::getLocalCircumscribingSphere() const {
	BoundingBox b = getLocalBounds();
	Vec3 localCentroid = b.getCenter();
	double maxRadiusSq = mainPart->hitbox.getMaxRadiusSq(localCentroid);
	for (const AttachedPart& p : parts) {
		double radiusSq = p.part->hitbox.getMaxRadiusSq(p.attachment.globalToLocal(localCentroid));
		maxRadiusSq = std::max(maxRadiusSq, radiusSq);
	}
	return Sphere{ localCentroid, sqrt(maxRadiusSq) };
}

void Physical::rotateAroundCenterOfMassUnsafe(const RotMat3& rotation) {
	Vec3 relCenterOfMass = getCFrame().localToRelative(localCenterOfMass);
	Vec3 relativeRotationOffset = rotation * relCenterOfMass - relCenterOfMass;
	mainPart->cframe.rotate(rotation);
	translateUnsafe(-relativeRotationOffset);
}
void Physical::translateUnsafe(const Vec3& translation) {
	mainPart->cframe.translate(translation);
}
void Physical::updateAttachedPartCFrames() {
	for (AttachedPart& attachment : parts) {
		attachment.part->cframe = getCFrame().localToGlobal(attachment.attachment);
	}

	this->circumscribingSphere.origin = getCFrame().localToGlobal(localCentroid);
}
void Physical::rotateAroundCenterOfMass(const RotMat3& rotation) {
	rotateAroundCenterOfMassUnsafe(rotation);
	updateAttachedPartCFrames();
}
void Physical::translate(const Vec3& translation) {
	translateUnsafe(translation);
	updateAttachedPartCFrames();
}

void Physical::update(double deltaT) {
	Vec3 accel = totalForce * (deltaT/mass);
	
	Vec3 localMoment = getCFrame().relativeToLocal(totalMoment);
	Vec3 localRotAcc = ~inertia * localMoment * deltaT;
	Vec3 rotAcc = getCFrame().localToRelative(localRotAcc);

	totalForce = Vec3();
	totalMoment = Vec3();

	velocity += accel;
	angularVelocity += rotAcc;

	Vec3 movement = velocity * deltaT + accel * deltaT * deltaT / 2;

	Mat3 rotation = fromRotationVec(angularVelocity * deltaT);

	rotateAroundCenterOfMassUnsafe(rotation);
	translateUnsafe(movement);

	updateAttachedPartCFrames();
}

void Physical::applyForceAtCenterOfMass(Vec3 force) {
	totalForce += force;

	Debug::logVec(getCenterOfMass(), force, Debug::FORCE);
}

void Physical::applyForce(Vec3Relative origin, Vec3 force) {
	totalForce += force;

	Debug::logVec(origin + getCenterOfMass(), force, Debug::FORCE);

	applyMoment(origin % force);
}

void Physical::applyMoment(Vec3 moment) {
	totalMoment += moment;
	Debug::logVec(getCenterOfMass(), moment, Debug::MOMENT);
}

void Physical::applyImpulseAtCenterOfMass(Vec3 impulse) {
	Debug::logVec(getCenterOfMass(), impulse, Debug::IMPULSE);
	velocity += impulse / mass;
}
void Physical::applyImpulse(Vec3Relative origin, Vec3Relative impulse) {
	Debug::logVec(origin + getCenterOfMass(), impulse, Debug::IMPULSE);
	velocity += impulse / mass;
	Vec3 angularImpulse = origin % impulse;
	applyAngularImpulse(angularImpulse);
}
void Physical::applyAngularImpulse(Vec3 angularImpulse) {
	Debug::logVec(getCenterOfMass(), angularImpulse, Debug::ANGULAR_IMPULSE);
	Vec3 localAngularImpulse = getCFrame().relativeToLocal(angularImpulse);
	Vec3 localRotAcc = ~inertia * localAngularImpulse;
	Vec3 rotAcc = getCFrame().localToRelative(localRotAcc);
	angularVelocity += rotAcc;
}

Vec3 Physical::getCenterOfMass() const {
	return getCFrame().localToGlobal(localCenterOfMass);
}

Vec3 Physical::getVelocityOfPoint(const Vec3Relative& point) const {
	return velocity + angularVelocity % point;
}

Vec3 Physical::getAcceleration() const {
	return totalForce / mass;
}

Vec3 Physical::getAngularAcceleration() const {
	return ~inertia * getCFrame().relativeToLocal(totalMoment);
}

Vec3 Physical::getAccelerationOfPoint(const Vec3Relative& point) const {
	return getAcceleration() + getAngularAcceleration() % point;
}

void Physical::setCFrame(const CFrame& newCFrame) {
	this->mainPart->cframe = newCFrame;
	for (const AttachedPart& p : parts) {
		p.part->cframe = newCFrame.localToGlobal(p.attachment);
	}

	this->circumscribingSphere.origin = getCFrame().localToGlobal(localCentroid);
}

/*
	Computes the force->acceleration transformation matrix
	Such that:
	a = M*F
*/
SymmetricMat3 Physical::getPointAccelerationMatrix(const Vec3Local& r) const {
	DiagonalMat3 movementFactor(1 / mass, 1 / mass, 1 / mass);

	Mat3 crossMat = createCrossProductEquivalent(r);

	SymmetricMat3 invInertia = ~inertia;

	SymmetricMat3 rotationFactor = multiplyLeftRight(invInertia , crossMat);

	return movementFactor + rotationFactor;
}
double Physical::getInertiaOfPointInDirectionLocal(const Vec3Local& localPoint, const Vec3Local& localDirection) const {
	SymmetricMat3 accMat = getPointAccelerationMatrix(localPoint);

	Vec3 force = localDirection;
	Vec3 accel = accMat * force;
	double accelInForceDir = accel * localDirection / localDirection.lengthSquared();

	return 1 / accelInForceDir;

	/*SymmetricMat3 accelToForceMat = ~accMat;
	Vec3 imaginaryForceForAcceleration = accelToForceMat * direction;
	double forcePerAccelRatio = imaginaryForceForAcceleration * direction / direction.lengthSquared();
	return forcePerAccelRatio;*/
}

double Physical::getInertiaOfPointInDirectionRelative(const Vec3Relative& relPoint, const Vec3Relative& relDirection) const {
	return getInertiaOfPointInDirectionLocal(getCFrame().relativeToLocal(relPoint), getCFrame().relativeToLocal(relDirection));
}

double Physical::getVelocityKineticEnergy() const {
	return mass * velocity.lengthSquared() / 2;
}
double Physical::getAngularKineticEnergy() const {
	Vec3 localAngularVel = getCFrame().relativeToLocal(angularVelocity);
	return (inertia * localAngularVel) * localAngularVel / 2;
}
double Physical::getKineticEnergy() const {
	return getVelocityKineticEnergy() + getAngularKineticEnergy();
}
