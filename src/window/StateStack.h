#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <unordered_map>
#include <functional>

#include "State.h"
#include "States.h"

#include "../utilities/NonCopyable.h"

class StateStack : private NonCopyable
{
public:
	enum Action
	{
		Push,
		Pop,
		Clear
	};

public:
	explicit StateStack(State::Context context)
		: _context(context) { }

	template<class T, typename... Args>
	void register_state(const States::ID& state_id, Args&&... args);

	void pre_update(float dt);
	void update(float dt);
	void fixed_update(float dt);
	void post_update(float dt, float interp);

	void draw();
	void handle_event(const sf::Event& event);

	void push(const States::ID& state_id);
	void pop();
	void clear();

	bool is_empty() const { return _stack.empty(); }

private:
	State::ptr create_state(const States::ID& state_id);
	void apply_pending_changes();

private:
	struct PendingChange
	{
		explicit PendingChange(const Action& action, const States::ID& state_id = States::ID::None)
			: _action(action), _state_id(state_id) { }

		const Action _action;
		const States::ID _state_id;
	};

private:
	std::vector<State::ptr>		_stack;
	std::vector<PendingChange>	_pending_list;

	State::Context				_context;
	std::unordered_map<States::ID, std::function<State::ptr()>> _factories;
};

template<class T, typename... Args>
void StateStack::register_state(const States::ID& state_id, Args&&... args)
{
	_factories[state_id] = [this, &args...]()
	{
		return State::ptr(new T(*this, _context, std::forward<Args>(args)...));
	};
}