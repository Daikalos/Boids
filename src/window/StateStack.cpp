#include "StateStack.h"

void StateStack::pre_update(float dt)
{
	for (auto it = _stack.rbegin(); it != _stack.rend(); ++it)
	{
		if (!(*it)->pre_update(dt))
			break;
	}
}

void StateStack::update(float dt)
{
	for (auto it = _stack.rbegin(); it != _stack.rend(); ++it)
	{
		if (!(*it)->update(dt))
			break;
	}
}

void StateStack::fixed_update(float dt)
{
	for (auto it = _stack.rbegin(); it != _stack.rend(); ++it)
	{
		if (!(*it)->fixed_update(dt))
			break;
	}
}

void StateStack::post_update(float dt, float interp)
{
	for (auto it = _stack.rbegin(); it != _stack.rend(); ++it)
	{
		if (!(*it)->post_update(dt, interp))
			break;
	}

	apply_pending_changes();
}

void StateStack::draw()
{
	for (State::ptr& state : _stack)
		state->draw();
}

void StateStack::handle_event(const sf::Event& event)
{
	for (auto it = _stack.rbegin(); it != _stack.rend(); ++it)
	{
		if (!(*it)->handle_event(event))
			break;
	}

	apply_pending_changes();
}

void StateStack::push(const States::ID& state_id)
{
	_pending_list.push_back(PendingChange(Action::Push, state_id));
}

void StateStack::pop()
{
	_pending_list.push_back(PendingChange(Action::Pop));
}

void StateStack::clear()
{
	_pending_list.push_back(PendingChange(Action::Clear));
}

State::ptr StateStack::create_state(const States::ID& state_id)
{
	auto found = _factories.find(state_id);
	assert(found != factories.end());

	return found->second();
}

void StateStack::apply_pending_changes()
{
	for (const PendingChange& change : _pending_list)
	{
		switch (change._action)
		{
		case Push:
			_stack.push_back(create_state(change._state_id));
			break;
		case Pop:
			{
				_stack.back()->on_destroy();
				_stack.pop_back();

				if (!_stack.empty())
					_stack.back()->on_activate();
			}
			break;
		case Clear:
			{
				for (State::ptr& state : _stack)
					state->on_destroy();

				_stack.clear();
			}
			break;
		}
	}

	_pending_list.clear();
}
