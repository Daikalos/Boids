#pragma once

#include <memory>

#include "Camera.h"
#include "Window.h"
#include "InputHandler.h"
#include "ResourceHolder.hpp"

class State
{
public:
	using Ptr = std::unique_ptr<State>;

	struct Context // holds vital objects
	{
		Context(Window& window, Camera& camera, InputHandler& inputHandler, TextureHolder& textureHolder, FontHolder& fontHolder)
			: window(&window), camera(&camera), inputHandler(&inputHandler), textureHolder(&textureHolder), fontHolder(&fontHolder) { }

		Window*			window;
		Camera*			camera;
		InputHandler*	inputHandler;
		TextureHolder*	textureHolder;
		FontHolder*		fontHolder;
	};

public:
	explicit State(Context context);
	virtual ~State();

public:
	virtual bool HandleEvent(const sf::Event& event) = 0;

	virtual bool PreUpdate(float dt) = 0;
	virtual bool Update(float dt) = 0;
	virtual bool FixedUpdate(float dt) = 0;
	virtual bool PostUpdate(float dt, float interp) = 0;

	virtual void Draw() = 0;

protected:
	auto GetContext() const -> const Context&;

private:
	Context	m_context;
};