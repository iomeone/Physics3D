#include "testsMain.h"

#include "compare.h"
#include "../physics/misc/toString.h"

#include "../physics/constraintGroup.h"

#include "estimateMotion.h"

#include "../physics/geometry/shape.h"
#include "../physics/geometry/shapeCreation.h"
#include "../physics/part.h"
#include "../physics/physical.h"
#include "../physics/constraints/fixedConstraint.h"
#include "../physics/constraints/motorConstraint.h"
#include "../physics/constraints/sinusoidalPistonConstraint.h"
#include "../physics/math/linalg/trigonometry.h"

#define ASSERT(cond) ASSERT_TOLERANT(cond, 0.05)

#define DELTA_T 0.0001

/*TEST_CASE(testBallConstraint) {
	Part part1(boxShape(2.0, 2.0, 2.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	part1.ensureHasParent();
	Part part2(boxShape(2.0, 2.0, 2.0), GlobalCFrame(6.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	part2.ensureHasParent();
	ConstraintGroup group;
	group.ballConstraints.push_back(BallConstraint{Vec3(3.0, 0.0, 0.0), part1.parent->mainPhysical, Vec3(-3.0, 0.0, 0.0), part2.parent->mainPhysical});

	part1.parent->mainPhysical->applyForceAtCenterOfMass(Vec3(2, 0, 0));

	group.apply();

	ASSERT(part1.getMotion().getAcceleration() == Vec3(0.125, 0.0, 0.0));
	ASSERT(part2.getMotion().getAcceleration() == Vec3(0.125, 0.0, 0.0));
}*/

TEST_CASE(testMotionOfPhysicalSinglePart) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});

	p1.ensureHasParent();

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();

	Position p1PosBefore = p1.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosMid = p1.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosAfter = p1.getCenterOfMass();

	TranslationalMotion estimatedVelAccel1 = estimateMotion(p1PosBefore, p1PosMid, p1PosAfter, DELTA_T);

	ASSERT(estimatedVelAccel1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedVelAccel1.getAcceleration() == p1calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalPartsBasic) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(0,0,0));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();
	Vec3 p2calculatedVelBefore = p2.getMotion().getVelocity();

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();
	Vec3 p2calculatedAccelBefore = p2.getMotion().getAcceleration();

	Position p1PosBefore = p1.getCenterOfMass();
	Position p2PosBefore = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosMid = p1.getCenterOfMass();
	Position p2PosMid = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosAfter = p1.getCenterOfMass();
	Position p2PosAfter = p2.getCenterOfMass();

	TranslationalMotion estimatedVelAccel1 = estimateMotion(p1PosBefore, p1PosMid, p1PosAfter, DELTA_T);
	TranslationalMotion estimatedVelAccel2 = estimateMotion(p2PosBefore, p2PosMid, p2PosAfter, DELTA_T);

	ASSERT(estimatedVelAccel1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedVelAccel2.getVelocity() == p2calculatedVelBefore);
	ASSERT(estimatedVelAccel1.getAcceleration() == p1calculatedAccelBefore);
	ASSERT(estimatedVelAccel2.getAcceleration() == p2calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalPartsRotation) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Motion COMMotion(Vec3(0, 0, 0), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();
	Vec3 p2calculatedVelBefore = p2.getMotion().getVelocity();

	logStream << p1.parent->getMotion() << "\n";
	logStream << p1.getMotion() << "\n";
	logStream << p2.getMotion() << "\n";

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();
	Vec3 p2calculatedAccelBefore = p2.getMotion().getAcceleration();

	GlobalCFrame p1CFrameBefore = p1.getCFrame();
	GlobalCFrame p2CFrameBefore = p2.getCFrame();

	p1.parent->mainPhysical->update(DELTA_T);

	GlobalCFrame p1CFrameMid = p1.getCFrame();
	GlobalCFrame p2CFrameMid = p2.getCFrame();

	p1.parent->mainPhysical->update(DELTA_T);

	GlobalCFrame p1CFrameAfter = p1.getCFrame();
	GlobalCFrame p2CFrameAfter = p2.getCFrame();

	Motion estimatedMotion1 = estimateMotion(p1CFrameBefore, p1CFrameMid, p1CFrameAfter, DELTA_T);
	Motion estimatedMotion2 = estimateMotion(p2CFrameBefore, p2CFrameMid, p2CFrameAfter, DELTA_T);

	ASSERT(estimatedMotion1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedMotion2.getVelocity() == p2calculatedVelBefore);
	ASSERT(estimatedMotion1.getAcceleration() == p1calculatedAccelBefore);
	ASSERT(estimatedMotion2.getAcceleration() == p2calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalPartsBasicFixedConstraint) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	
	p1.attach(&p2, new FixedConstraint(), CFrame(1.0, 0.0, 0.0), CFrame(0,0,0));

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(0, 0, 0));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();
	Vec3 p2calculatedVelBefore = p2.getMotion().getVelocity();

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();
	Vec3 p2calculatedAccelBefore = p2.getMotion().getAcceleration();

	Position p1PosBefore = p1.getCenterOfMass();
	Position p2PosBefore = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosMid = p1.getCenterOfMass();
	Position p2PosMid = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosAfter = p1.getCenterOfMass();
	Position p2PosAfter = p2.getCenterOfMass();

	TranslationalMotion estimatedVelAccel1 = estimateMotion(p1PosBefore, p1PosMid, p1PosAfter, DELTA_T);
	TranslationalMotion estimatedVelAccel2 = estimateMotion(p2PosBefore, p2PosMid, p2PosAfter, DELTA_T);

	ASSERT(estimatedVelAccel1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedVelAccel2.getVelocity() == p2calculatedVelBefore);
	ASSERT(estimatedVelAccel1.getAcceleration() == p1calculatedAccelBefore);
	ASSERT(estimatedVelAccel2.getAcceleration() == p2calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalPartsRotationFixedConstraint) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	
	p1.attach(&p2, new FixedConstraint(), CFrame(1.0, 0.0, 0.0), CFrame(0, 0, 0));

	Motion COMMotion(Vec3(0, 0, 0), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();
	Vec3 p2calculatedVelBefore = p2.getMotion().getVelocity();

	logStream << p1.parent->getMotion() << "\n";
	logStream << p1.getMotion() << "\n";
	logStream << p2.getMotion() << "\n";

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();
	Vec3 p2calculatedAccelBefore = p2.getMotion().getAcceleration();

	Position p1PosBefore = p1.getCenterOfMass();
	Position p2PosBefore = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosMid = p1.getCenterOfMass();
	Position p2PosMid = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosAfter = p1.getCenterOfMass();
	Position p2PosAfter = p2.getCenterOfMass();

	TranslationalMotion estimatedVelAccel1 = estimateMotion(p1PosBefore, p1PosMid, p1PosAfter, DELTA_T);
	TranslationalMotion estimatedVelAccel2 = estimateMotion(p2PosBefore, p2PosMid, p2PosAfter, DELTA_T);

	ASSERT(estimatedVelAccel2.getVelocity() == p2calculatedVelBefore);
	ASSERT(estimatedVelAccel1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedVelAccel2.getAcceleration() == p2calculatedAccelBefore);
	ASSERT(estimatedVelAccel1.getAcceleration() == p1calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalParts) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	Vec3 p1calculatedVelBefore = p1.getMotion().getVelocity();
	Vec3 p2calculatedVelBefore = p2.getMotion().getVelocity();

	Vec3 p1calculatedAccelBefore = p1.getMotion().getAcceleration();
	Vec3 p2calculatedAccelBefore = p2.getMotion().getAcceleration();

	Position p1PosBefore = p1.getCenterOfMass();
	Position p2PosBefore = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosMid = p1.getCenterOfMass();
	Position p2PosMid = p2.getCenterOfMass();

	p1.parent->mainPhysical->update(DELTA_T);

	Position p1PosAfter = p1.getCenterOfMass();
	Position p2PosAfter = p2.getCenterOfMass();

	TranslationalMotion estimatedVelAccel1 = estimateMotion(p1PosBefore, p1PosMid, p1PosAfter, DELTA_T);
	TranslationalMotion estimatedVelAccel2 = estimateMotion(p2PosBefore, p2PosMid, p2PosAfter, DELTA_T);

	ASSERT(estimatedVelAccel1.getVelocity() == p1calculatedVelBefore);
	ASSERT(estimatedVelAccel2.getVelocity() == p2calculatedVelBefore);
	ASSERT(estimatedVelAccel1.getAcceleration() == p1calculatedAccelBefore);
	ASSERT(estimatedVelAccel2.getAcceleration() == p2calculatedAccelBefore);
}

TEST_CASE(testMotionOfPhysicalJointsBasic) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Part p1e(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2e(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	
	p1e.attach(&p2e, new FixedConstraint(), CFrame(1.0, 0.0, 0.0), CFrame(0, 0, 0));

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(0,0,0));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;
	p1e.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	ASSERT(p1.getMotion() == p1e.getMotion());
	ASSERT(p2.getMotion() == p2e.getMotion());
}

TEST_CASE(testMotionOfPhysicalJointsRotation) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Part p1e(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2e(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	
	p1e.attach(&p2e, new FixedConstraint(), CFrame(1.0, 0.0, 0.0), CFrame(0.0, 0.0, 0.0));

	Motion COMMotion(Vec3(0,0,0), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;
	p1e.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	logStream << p1.parent->getMotion() << "\n";
	logStream << p1.getMotion() << "\n";
	logStream << p2.getMotion() << "\n";
	logStream << p1e.parent->getMotion() << "\n";
	logStream << p1e.getMotion() << "\n";
	logStream << p2e.getMotion() << "\n";

	ASSERT(p1.getMotion() == p1e.getMotion());
	ASSERT(p2.getMotion() == p2e.getMotion());
}

TEST_CASE(testMotionOfPhysicalJoints) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Part p1e(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2e(sphereShape(1.0), GlobalCFrame(1.0, 0.0, 0.0), {3.0, 1.0, 1.0});
	
	p1e.attach(&p2e, new FixedConstraint(), CFrame(1.0, 0.0, 0.0), CFrame(0, 0, 0));

	Motion COMMotion(Vec3(1.0, 0.7, 1.3), Vec3(-0.3, 1.7, -1.1));

	p1.parent->mainPhysical->motionOfCenterOfMass = COMMotion;
	p1e.parent->mainPhysical->motionOfCenterOfMass = COMMotion;

	logStream << p1.parent->getMotion() << "\n";
	logStream << p1.getMotion() << "\n";
	logStream << p2.getMotion() << "\n";
	logStream << p1e.parent->getMotion() << "\n";
	logStream << p1e.getMotion() << "\n";
	logStream << p2e.getMotion() << "\n";

	ASSERT(p1.getMotion() == p1e.getMotion());
	ASSERT(p2.getMotion() == p2e.getMotion());
}

TEST_CASE(testFixedConstraintProperties) {
	Part p1(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2(sphereShape(1.0), GlobalCFrame(), {3.0, 1.0, 1.0});
	p1.attach(&p2, CFrame(1.0, 0.0, 0.0));

	Part p1e(sphereShape(1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part p2e(sphereShape(1.0), GlobalCFrame(), {3.0, 1.0, 1.0});
	
	p1e.attach(&p2e, new FixedConstraint(), CFrame(0.3, -0.8, 0.0), CFrame(-0.7, -0.8, 0));

	MotorizedPhysical* phys1 = p1.parent->mainPhysical;
	MotorizedPhysical* phys1e = p1e.parent->mainPhysical;

	ASSERT(p1.getCFrame() == GlobalCFrame(0.0, 0.0, 0.0));
	ASSERT(p2.getCFrame() == GlobalCFrame(1.0, 0.0, 0.0));
	ASSERT(p1e.getCFrame() == GlobalCFrame(0.0, 0.0, 0.0));
	ASSERT(p2e.getCFrame() == GlobalCFrame(1.0, 0.0, 0.0));
	ASSERT(phys1->totalMass == phys1e->totalMass);
	ASSERT(phys1->getCenterOfMass() == phys1e->getCenterOfMass());
	ASSERT(phys1->forceResponse == phys1e->forceResponse);
	ASSERT(phys1->momentResponse == phys1e->momentResponse);

}

TEST_CASE(testApplyForceToFixedConstraint) {
	CFrame attach(Vec3(1.3, 0.7, 0.9), Rotation::fromEulerAngles(0.3, -0.7, 0.9));
	
	Part p1(boxShape(1.0, 2.0, 3.0), GlobalCFrame(), {1.0, 1.0, 1.0});
	Part p2(boxShape(1.5, 2.3, 1.2), GlobalCFrame(), {1.0, 1.0, 1.0});
	
	p1.attach(&p2, attach);


	Part p1e(boxShape(1.0, 2.0, 3.0), GlobalCFrame(), {1.0, 1.0, 1.0});
	Part p2e(boxShape(1.5, 2.3, 1.2), GlobalCFrame(), {1.0, 1.0, 1.0});
	
	p1e.attach(&p2e, new FixedConstraint(), attach, CFrame());

	MotorizedPhysical* phys1 = p1.parent->mainPhysical;
	MotorizedPhysical* phys1e = p1e.parent->mainPhysical;

	phys1->applyForceAtCenterOfMass(Vec3(2.7, 3.9, -2.3));
	phys1e->applyForceAtCenterOfMass(Vec3(2.7, 3.9, -2.3));

	phys1->update(DELTA_T);
	phys1e->update(DELTA_T);

	ASSERT(phys1->getMotion() == phys1e->getMotion());

	ASSERT(p1.getCFrame() == p1e.getCFrame());
	ASSERT(p2.getCFrame() == p2e.getCFrame());

	ASSERT(phys1->forceResponse == phys1e->forceResponse);
	ASSERT(phys1->momentResponse == phys1e->momentResponse);

	phys1->applyImpulseAtCenterOfMass(Vec3(2.7, 3.9, -2.3));
	phys1e->applyImpulseAtCenterOfMass(Vec3(2.7, 3.9, -2.3));

	phys1->update(DELTA_T);
	phys1e->update(DELTA_T);

	ASSERT(phys1->getMotion() == phys1e->getMotion());

	ASSERT(p1.getCFrame() == p1e.getCFrame());
	ASSERT(p2.getCFrame() == p2e.getCFrame());

	ASSERT(phys1->forceResponse == phys1e->forceResponse);
	ASSERT(phys1->momentResponse == phys1e->momentResponse);
}

TEST_CASE(testPlainAttachAndFixedConstraintIndistinguishable) {
	Part firstPart(boxShape(1.0, 1.0, 1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part secondPart(boxShape(0.5, 0.5, 0.5), GlobalCFrame(1.0, 0.0, 0.0), {1.0, 1.0, 1.0});

	firstPart.attach(&secondPart, CFrame(1.0, 0.0, 0.0));

	Part firstPart2(boxShape(1.0, 1.0, 1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part secondPart2(boxShape(0.5, 0.5, 0.5), GlobalCFrame(1.0, 0.0, 0.0), {1.0, 1.0, 1.0});

	firstPart2.attach(&secondPart2, new FixedConstraint(), CFrame(0.5, 0.0, 0.0), CFrame(-0.5, 0.0, 0.0));

	Vec3 impulse(1.3, 4.7, 0.2);
	Vec3 impulsePos(2.6, 1.7, 2.8);

	MotorizedPhysical* main1 = firstPart.parent->mainPhysical;
	MotorizedPhysical* main2 = firstPart2.parent->mainPhysical;

	main1->applyImpulse(impulsePos, impulse);
	main2->applyImpulse(impulsePos, impulse);

	main1->update(0.5);
	main2->update(0.5);

	ASSERT(main1->getMotionOfCenterOfMass() == main2->getMotionOfCenterOfMass());
	ASSERT(main1->getCFrame() == main2->getCFrame());
}

TEST_CASE(testInternalMotionOfCenterOfMass) {
	Part firstPart(boxShape(1.0, 1.0, 1.0), GlobalCFrame(0.0, 0.0, 0.0), {1.0, 1.0, 1.0});
	Part secondPart(boxShape(0.5, 0.5, 0.5), GlobalCFrame(1.0, 0.0, 0.0), {1.0, 1.0, 1.0});

	SinusoidalPistonConstraint* piston = new SinusoidalPistonConstraint(0.3, 1.0, 1.0);

	piston->currentStepInPeriod = 0.7;

	firstPart.attach(&secondPart, piston, CFrame(0.0, 0.0, 1.0), CFrame(0.0, 0.0, -1.0));

	MotorizedPhysical* phys = firstPart.parent->mainPhysical;

	Vec3 original = phys->totalCenterOfMass;

	ALLOCA_InternalMotionTree(cache, phys, size);

	TranslationalMotion motionOfCom = std::get<2>(cache.getInternalMotionOfCenterOfMass());

	phys->update(DELTA_T);
	Vec3 v2 = phys->totalCenterOfMass;

	phys->update(DELTA_T);
	Vec3 v3 = phys->totalCenterOfMass;

	TranslationalMotion estimatedMotion = estimateMotion(original, v2, v3, DELTA_T);

	ASSERT(motionOfCom == estimatedMotion);
}
