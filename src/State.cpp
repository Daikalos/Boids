#include "State.h"

State::State::Context::Context(
	Window& window, 
	Camera& camera, 
	InputHandler& inputHandler, 
	TextureHolder& textureHolder,
	FontHolder& fontHolder)
	: m_window(&window)
	, m_camera(&camera)
	, m_inputHandler(&inputHandler)
	, m_textureHolder(&textureHolder)
	, m_fontHolder(&fontHolder)
{

}

const Window& State::Context::GetWindow() const
{
	return *m_window;
}
Window& State::Context::GetWindow()
{
	return *m_window;
}

const Camera& State::Context::GetCamera() const
{
	return *m_camera;
}
Camera& State::Context::GetCamera()
{
	return *m_camera;
}

const InputHandler& State::Context::GetInputHandler() const
{
	return *m_inputHandler;
}
InputHandler& State::Context::GetInputHandler()
{
	return *m_inputHandler;
}

const TextureHolder& State::Context::GetTextureHolder() const
{
	return *m_textureHolder;
}
TextureHolder& State::Context::GetTextureHolder()
{
	return *m_textureHolder;
}

const FontHolder& State::Context::GetFontHolder() const
{
	return *m_fontHolder;
}
FontHolder& State::Context::GetFontHolder()
{
	return *m_fontHolder;
}

State::State(const Context& context)
	: m_context(context) { }

State::~State() = default;

auto State::GetContext() -> Context&
{
	return m_context;
}
auto State::GetContext() const -> const Context&
{
	return m_context;
};