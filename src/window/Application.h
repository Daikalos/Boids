#pragma once

#include "../boids/MainState.h"

#include "Camera.h"
#include "Window.h"
#include "ResourceHolder.hpp"

class Application
{
public:
	Application(std::string name);

public:
	void Run();

private:
	void ProcessInput();

	void PreUpdate(float dt);
	void Update(float dt);
	void FixedUpdate(float dt); // after update, before post_update
	void PostUpdate(float dt, float interp);

	void Draw();

private:
	Camera			m_camera;
	Window			m_window;
	InputHandler	m_inputHandler;
	TextureHolder	m_textureHolder;
	FontHolder		m_fontHolder;
	MainState		m_mainState;
};

