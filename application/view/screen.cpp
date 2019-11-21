#include "core.h"

#include "screen.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../graphics/renderUtils.h"
#include "../graphics/texture.h"
#include "shader/shaders.h"

#include "../graphics/mesh/indexedMesh.h"
#include "../graphics/mesh/primitive.h"
#include "../graphics/debug/visualDebug.h"
#include "../graphics/buffers/frameBuffer.h"
#include "../engine/options/keyboardOptions.h"
#include "../input/standardInputHandler.h"
#include "../graphics/meshLibrary.h"
#include "../graphics/visualShape.h"
#include "../worlds.h"
#include "../engine/event/windowEvent.h"

#include "../engine/layer/layerStack.h"
#include "layer/skyboxLayer.h"
#include "layer/modelLayer.h"
#include "layer/testLayer.h"
#include "layer/pickerLayer.h"
#include "layer/postprocessLayer.h"
#include "layer/guiLayer.h"
#include "layer/debugLayer.h"
#include "layer/debugOverlay.h"

#include "../physics/geometry/shapeClass.h"

#include "frames.h"

std::vector<IndexedMesh*> Screen::meshes;
std::map<const ShapeClass*, VisualData> Screen::shapeClassMeshIds;

bool initGLFW() {
	// Set window hints
	//Renderer::setGLFWMultisampleSamples(4);

	// Initialize GLFW
	if (!Renderer::initGLFW()) {
		Log::error("GLFW failed to initialize");
		return false;
	}

	//Renderer::enableMultisampling();

	Log::info("Initialized GLFW");

	return true;
}

bool initGLEW() {
	// Init GLEW after creating a valid rendering context
	if (!Renderer::initGLEW()) {
		terminateGLFW();

		Log::error("GLEW Failed to initialize!");

		return false;
	}

	Log::info("Initialized GLEW");
	return true;
}

void terminateGLFW() {
	Log::info("Closing GLFW");
	Renderer::terminateGLFW();
	Log::info("Closed GLFW");
}

Screen::Screen() {

};

Screen::Screen(int width, int height, PlayerWorld* world) {
	this->world = world;

	// Create a windowed mode window and its OpenGL context 
	Renderer::createGLFWContext(width, height, "Physics3D");

	if (!Renderer::validGLFWContext()) {
		Log::fatal("Invalid rendering context");
		terminateGLFW();
		exit(-1);
	}

	// Make the window's context current 
	Renderer::makeGLFWContextCurrent();

	Log::info("OpenGL vendor: (%s)", Renderer::getVendor());
	Log::info("OpenGL renderer: (%s)", Renderer::getRenderer());
	Log::info("OpenGL version: (%s)", Renderer::getVersion());
	Log::info("OpenGL shader version: (%s)", Renderer::getShaderVersion());
}


// Handler
StandardInputHandler* handler = nullptr;


// Layers
LayerStack layerStack;
SkyboxLayer skyboxLayer;
ModelLayer modelLayer;
TestLayer testLayer;
PickerLayer pickerLayer;
PostprocessLayer postprocessLayer;
GuiLayer guiLayer;
DebugLayer debugLayer;
DebugOverlay debugOverlay;


void Screen::onInit() {

	// Log init
	Log::setLogLevel(Log::Level::INFO);


	// Properties init
	properties = PropertiesParser::read("../res/.properties");


	// load options from properties
	KeyboardOptions::load(properties);


	// Library init
	Library::onInit();


	// Render mode init
	Renderer::enableCulling();
	Renderer::enableDepthTest();


	// InputHandler init
	handler = new StandardInputHandler(Renderer::getGLFWContext(), *this);


	// Screen size init
	dimension = Renderer::getGLFWWindowSize();


	// Framebuffer init
	quad = new Quad();
	screenFrameBuffer = new FrameBuffer(dimension.x, dimension.y);
	blurFrameBuffer = new FrameBuffer(dimension.x, dimension.y);


	// Shader init
	ApplicationShaders::onInit();


	// Layer creation
	skyboxLayer 	 = SkyboxLayer		(this);
	modelLayer 		 = ModelLayer		(this);
	testLayer		 = TestLayer		(this);
	debugLayer 		 = DebugLayer		(this);
	pickerLayer 	 = PickerLayer		(this);
	postprocessLayer = PostprocessLayer	(this);
	guiLayer		 = GuiLayer			(this);
	debugOverlay 	 = DebugOverlay		(this);

	layerStack.pushLayer(&skyboxLayer);
	layerStack.pushLayer(&modelLayer);
	layerStack.pushLayer(&testLayer);
	layerStack.pushLayer(&debugLayer);
	layerStack.pushLayer(&pickerLayer);
	layerStack.pushLayer(&postprocessLayer);
	layerStack.pushLayer(&guiLayer);
	layerStack.pushOverlay(&debugOverlay);


	// Layer init
	layerStack.onInit();


	// Eventhandler init
	eventHandler.setWindowResizeCallback([](Screen& screen, Vec2i dimension) {
		screen.camera.onUpdate(((float)dimension.x) / ((float)dimension.y));
		screen.dimension = dimension;

		screen.screenFrameBuffer->resize(screen.dimension);
	});


	// Camera init
	camera.setPosition(Position(1.0, 1.0, -2.0));
	camera.setRotation(Vec3(0, 3.1415, 0.0));
	camera.onUpdate(1.0, camera.aspect, 0.01, 10000.0);


	// Resize
	FrameBufferResizeEvent event(dimension.x, dimension.y);
	handler->onFrameBufferResize(event);
}


void Screen::onUpdate() {
	std::chrono::time_point<std::chrono::steady_clock> curUpdate = std::chrono::steady_clock::now();
	std::chrono::nanoseconds deltaTnanos = curUpdate - this->lastUpdate;
	this->lastUpdate = curUpdate;

	double speedAdjustment = deltaTnanos.count() * 0.000000001 * 60.0;

	// IO events
	if (handler->anyKey) {
		bool leftDragging = handler->leftDragging;
		if (handler->getKey(KeyboardOptions::Move::forward))  camera.move(*this, 0, 0, -1 * speedAdjustment, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::backward)) camera.move(*this, 0, 0, 1 * speedAdjustment, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::right))	  camera.move(*this, 1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::left))     camera.move(*this, -1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::ascend))
			if (camera.flying) camera.move(*this, 0, 1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::descend))
			if (camera.flying) camera.move(*this, 0, -1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::left))  camera.rotate(*this, 0, 1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::right)) camera.rotate(*this, 0, -1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::up))    camera.rotate(*this, 1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::down))  camera.rotate(*this, -1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Application::close)) Renderer::closeGLFWWindow();
		if (handler->getKey(KeyboardOptions::Debug::frame)) { guiLayer.debugFrame->visible = true; guiLayer.debugFrame->position = Vec2(0.8); GUI::select(guiLayer.debugFrame); }
	}

	// Update camera
	camera.onUpdate();


	// Update layers
	layerStack.onUpdate();

}

void Screen::onEvent(Event& event) {
	camera.onEvent(event);

	layerStack.onEvent(event);
}

void Screen::onRender() {
	// Render to screen Framebuffer
	screenFrameBuffer->bind();
	Renderer::clearColor();
	Renderer::clearDepth();

	
	// Render layers
	layerStack.onRender();

	graphicsMeasure.mark(GraphicsProcess::FINALIZE);

	// Finalize
	Renderer::swapGLFWInterval(0);
	Renderer::swapGLFWBuffers();
	Renderer::pollGLFWEvents();

	graphicsMeasure.mark(GraphicsProcess::OTHER);
}

void Screen::onClose() {
	screenFrameBuffer->close();
	blurFrameBuffer->close();

	layerStack.onClose();

	Library::onClose();

	ApplicationShaders::onClose();

	KeyboardOptions::save(properties);

	PropertiesParser::write("../res/.properties", properties);

	terminateGLFW();
}

bool Screen::shouldClose() {
	return Renderer::isGLFWWindowClosed();
}

VisualData Screen::addMeshShape(const VisualShape& s) {
	int size = (int) meshes.size();
	//Log::error("Mesh %d added!", size);
	meshes.push_back(new IndexedMesh(s));
	return VisualData{size, s.uvs != nullptr, s.normals != nullptr};
}
VisualData Screen::registerMeshFor(const ShapeClass* shapeClass, const VisualShape& mesh) {
	if(shapeClassMeshIds.find(shapeClass) != shapeClassMeshIds.end()) throw "Attempting to re-register existing ShapeClass!";

	VisualData meshData = addMeshShape(mesh);

	//Log::error("Mesh %d registered!", meshData);

	shapeClassMeshIds.insert(std::pair<const ShapeClass*, VisualData>(shapeClass, meshData));
	return meshData;
}
VisualData Screen::registerMeshFor(const ShapeClass* shapeClass) {
	return registerMeshFor(shapeClass, VisualShape(shapeClass->asPolyhedron()));
}
VisualData Screen::getOrCreateMeshFor(const ShapeClass* shapeClass) {
	auto found = shapeClassMeshIds.find(shapeClass);

	if(found != shapeClassMeshIds.end()) {
		// mesh found!
		VisualData meshData = (*found).second;
		//Log::error("Mesh %d reused!", meshData);
		return meshData;
	} else {
		// mesh not found :(
		return registerMeshFor(shapeClass);
	}
}
