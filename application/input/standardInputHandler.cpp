#include "core.h"

#include "standardInputHandler.h"

#include "../engine/input/keyboard.h"
#include "../engine/input/mouse.h"

#include "../application/worldBuilder.h"
#include "../engine/options/keyboardOptions.h"
#include "../graphics/renderer.h"
#include "../physics/misc/toString.h"
#include "../application.h"
#include "../picker/picker.h"
#include "../graphics/gui/gui.h"
#include "../graphics/gui/guiUtils.h"
#include "../graphics/debug/visualDebug.h"
#include "../worlds.h"
#include "../view/screen.h"
#include "../view/camera.h"
#include <random>

namespace Application {

StandardInputHandler::StandardInputHandler(GLFWwindow* window, Screen& screen) : InputHandler(window), screen(screen) {}

void StandardInputHandler::onEvent(::Event& event) {
	screen.onEvent(event);

	EventDispatcher dispatcher(event);

	if (dispatcher.dispatch<KeyPressEvent>(BIND_EVENT_METHOD(StandardInputHandler::onKeyPress)))
		return;

	if (dispatcher.dispatch<DoubleKeyPressEvent>(BIND_EVENT_METHOD(StandardInputHandler::onDoubleKeyPress)))
		return;

	if (dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_METHOD(StandardInputHandler::onWindowResize)))
		return;

}

bool StandardInputHandler::onWindowResize(WindowResizeEvent& event) {
	Vec2i dimension = Vec2i(event.getWidth(), event.getHeight());

	Graphics::Renderer::viewport(Vec2i(), dimension);
	Log::debug("Window resize: %s", str(dimension).c_str());
	(*screen.eventHandler.windowResizeHandler) (screen, dimension);

	return true;
}

bool StandardInputHandler::onFrameBufferResize(FrameBufferResizeEvent& event) {
	Vec2i dimension = Vec2i(event.getWidth(), event.getHeight());

	Graphics::Renderer::viewport(Vec2i(), dimension);
	Log::debug("Framebuffer resize: %s", str(dimension).c_str());
	(*screen.eventHandler.windowResizeHandler) (screen, dimension);

	return true;
}

bool StandardInputHandler::onKeyPressOrRepeat(KeyPressEvent& event) {
	int key = event.getKey();

	if (KeyboardOptions::Tick::Speed::up == key) {
		setSpeed(getSpeed() * 1.5);
		Log::info("TPS is now: %f", getSpeed());
	} else if (KeyboardOptions::Tick::Speed::down == key) {
		setSpeed(getSpeed() / 1.5);
		Log::info("TPS is now: %f", getSpeed());
	} else if (KeyboardOptions::Tick::run == key) {
		if (isPaused()) runTick();
	} else if (Keyboard::O == key) {
		world.asyncModification([]() {
			Position pos(0.0 + (rand() % 100) * 0.001, 1.0 + (rand() % 100) * 0.001, 0.0 + (rand() % 100) * 0.001);

			WorldBuilder::createDominoAt(GlobalCFrame(pos, Rotation::fromEulerAngles(0.2, 0.3, 0.7)));
		}); 
		Log::info("Created domino! There are %d objects in the world! ", screen.world->getPartCount());
	}

	return true;
}

bool StandardInputHandler::onKeyPress(KeyPressEvent& event) {
	using namespace Graphics::Debug;

	int key = event.getKey();

	if (KeyboardOptions::Tick::pause == key) {
		togglePause();
	} else if (KeyboardOptions::Part::remove == key) {
		if (screen.selectedPart != nullptr) {
			screen.world->asyncModification([world = screen.world, selectedPart = screen.selectedPart]() {
				world->removePart(selectedPart);
			});
			screen.world->selectedPart = nullptr;
			screen.selectedPart = nullptr;
		}
	} else if (KeyboardOptions::Debug::pies == key) {
		renderPiesEnabled = !renderPiesEnabled;
	} else if (KeyboardOptions::Part::anchor == key) {
		throw "Not implemented!";
	} else if(KeyboardOptions::Part::makeMainPart == key) {
		Log::info("Made %s the main part of it's physical", screen.selectedPart->name.c_str());
		screen.selectedPart->makeMainPart();
	} else if(KeyboardOptions::Part::makeMainPhysical == key) {
		if(screen.selectedPart->parent != nullptr) {
			if(!screen.selectedPart->parent->isMainPhysical()) {
				Log::info("Made %s the main physical", screen.selectedPart->name.c_str());
				((ConnectedPhysical*) screen.selectedPart->parent)->makeMainPhysical();
			} else {
				Log::warn("This physical is already the main physical!");
			}
		} else {
			Log::warn("This part has no physical!");
		}
	} else if (KeyboardOptions::World::valid == key) {
		Log::debug("Checking World::isValid()");
		screen.world->asyncReadOnlyOperation([world = screen.world]() {
			world->isValid();
		});
	} else if (KeyboardOptions::Edit::rotate == key) {
		Picker::editTools.editMode = EditTools::EditMode::ROTATE;
	} else if (KeyboardOptions::Edit::translate == key) {
		Picker::editTools.editMode = EditTools::EditMode::TRANSLATE;
	} else if (KeyboardOptions::Edit::scale == key) {
		Picker::editTools.editMode = EditTools::EditMode::SCALE;
	} else if (KeyboardOptions::Debug::spheres == key) {
		colissionSpheresMode = static_cast<SphereColissionRenderMode>((static_cast<int>(colissionSpheresMode) + 1) % 3);
	} else if (KeyboardOptions::Debug::tree == key) {
		colTreeRenderMode = static_cast<ColTreeRenderMode>((static_cast<int>(colTreeRenderMode) + 1) % 5);
	}

	if (Keyboard::F1 <= key && Keyboard::F9 >= key) {
		toggleVectorType(static_cast<Debug::VectorType>(key - Keyboard::F1.code));
	}

	if (Keyboard::NUMBER_1 <= key && Keyboard::NUMBER_3 >= key) {
		togglePointType(static_cast<Debug::PointType>(key - Keyboard::NUMBER_1.code));
	}

	return onKeyPressOrRepeat(event);
};

bool StandardInputHandler::onDoubleKeyPress(DoubleKeyPressEvent& event) {
	int key = event.getKey();

	if (KeyboardOptions::Move::fly == key) {
		toggleFlying();
	}

	return true;
}

};