#pragma once

#include "../engine/layer/layer.h"

namespace Application {

class Screen;

class PostprocessLayer : public Layer {
public:
	PostprocessLayer();
	PostprocessLayer(Screen* screen, char flags = NoUpdate | NoEvents);

	void onInit() override;
	void onUpdate() override;
	void onEvent(::Event& event) override;
	void onRender() override;
	void onClose() override;
};

};