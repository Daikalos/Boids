#pragma once

#include <SFML/Window/Event.hpp>

#include <memory>

#include "ResourceHolder.hpp"

class Window;
class Camera;
class InputHandler;

class State
{
public:
	using Ptr = std::unique_ptr<State>;

	class Context // holds vital objects
	{
	public:
		Context(Window& window, Camera& camera, InputHandler& inputHandler, TextureHolder& textureHolder, FontHolder& fontHolder);

		const Window& GetWindow() const;
		Window& GetWindow();

		const Camera& GetCamera() const;
		Camera& GetCamera();

		const InputHandler& GetInputHandler() const;
		InputHandler& GetInputHandler();

		const TextureHolder& GetTextureHolder() const;
		TextureHolder& GetTextureHolder();

		const FontHolder& GetFontHolder() const;
		FontHolder& GetFontHolder();

	private:
		Window*			m_window;
		Camera*			m_camera;
		InputHandler*	m_inputHandler;
		TextureHolder*	m_textureHolder;
		FontHolder*		m_fontHolder;
	};

public:
	explicit State(const Context& context);
	virtual ~State();

public:
	virtual bool HandleEvent(const sf::Event& event) = 0;

	virtual bool PreUpdate(float dt) = 0;
	virtual bool Update(float dt) = 0;
	virtual bool FixedUpdate(float dt) = 0;
	virtual bool PostUpdate(float dt, float interp) = 0;

	virtual void Draw() = 0;

protected:
	auto GetContext() -> Context&;
	auto GetContext() const -> const Context&;

private:
	Context	m_context;
};