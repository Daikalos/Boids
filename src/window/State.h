#pragma once

#include <memory>

#include "Camera.h"
#include "Window.h"
#include "InputHandler.h"
#include "ResourceHolder.hpp"

#include "States.h"

class StateStack;

class State
{
public:
	typedef std::unique_ptr<State> ptr;

	struct Context // holds vital objects
	{
		Context(Window& window, Camera& camera, InputHandler& input_handler, TextureHolder& texture_holder, FontHolder& font_holder)
			: window(&window), camera(&camera), input_handler(&input_handler), texture_holder(&texture_holder), font_holder(&font_holder) { }

		Window*			window;
		Camera*			camera;
		InputHandler*	input_handler;
		TextureHolder*	texture_holder;
		FontHolder*		font_holder;
	};

public:
	explicit State(StateStack& _state_stack, Context context)
		: _state_stack(&_state_stack), _context(context) { }

	virtual ~State() {}

	virtual bool handle_event(const sf::Event& event) = 0;

	virtual bool pre_update(float dt)					{ return true; }
	virtual bool update(float dt) = 0;
	virtual bool fixed_update(float dt)					{ return true; }
	virtual bool post_update(float dt, float interp)	{ return true; }

	virtual void draw() = 0;

	virtual void on_activate() {}
	virtual void on_destroy() {}

protected:
	void request_stack_push(States::ID state_id);
	void request_stack_pop();
	void request_stack_clear();

	const Context& context() const
	{
		return _context;
	}

private:
	StateStack* _state_stack;
	Context		_context;
};