#pragma once

#include "../quad.h"
#include "../font.h"
#include "../shaderProgram.h"
#include "../screen.h"
#include "../texture.h"

#include "component.h"

#include "../engine/math/vec3.h"
#include "../engine/math/mat4.h"

#include <string>

namespace GUI {

	// Colors
	namespace COLOR {
		
		extern Vec4 NAVY;
		extern Vec4 BLUE;
		extern Vec4 AQUA;
		extern Vec4 TEAL;
		extern Vec4 OLIVE;
		extern Vec4 GREEN;
		extern Vec4 LIME;
		extern Vec4 YELLOW;
		extern Vec4 ORANGE;
		extern Vec4 RED;
		extern Vec4 MAROON;
		extern Vec4 FUCHSIA;
		extern Vec4 PURPLE;
		extern Vec4 BLACK;
		extern Vec4 GRAY;
		extern Vec4 SILVER;
		extern Vec4 WHITE;
			
		// format: 0xRRGGBBAA
		Vec4 get(int hex, bool alpha);
		Vec4 get(int hex);
	};

	// Events
	extern Component* intersectedComponent;
	extern Component* selectedComponent;

	// Shader
	extern Quad* defaultQuad;
	extern GUIShader* defaultShader;
	
	// Label
	extern double defaultLabelPadding;
	extern double defaultLabelMargin;
	extern Vec4 defaultLabelBackgroundColor;

	// Panel
	extern double defaultPanelPadding;
	extern double defaultPanelMargin;
	extern Vec4 defaultPanelBackgroundColor;

	// Frame
	extern Texture* defaultFrameCloseTexture;
	extern double defaultFrameTitleBarHeight;
	extern double defaultFrameCloseButtonOffset;
	extern double defaultFramePadding;
	extern double defaultFrameMargin;
	extern Vec4 defaultFrameTitleBarColor;
	extern Vec4 defaultFrameBackgroundColor;

	// Font
	extern Font* defaultFont;
	extern Vec4 defaultFontColor;
	extern double defaultFontSize;

	void add(Component* component);
	void remove(Component* component);

	Vec2 map(Screen& screen, Vec2 point);
	void intersect(Vec2 mouse);
	bool intersectsSquare(Vec2 point, Vec2 topleft, Vec2 dimension);

	void init(GUIShader* shader, Font* font);
	void update(Mat4f orthoMatrix);
	void render(Mat4f orthoMatrix);
};
