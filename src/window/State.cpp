#include "State.h"
#include "StateStack.h"

void State::request_stack_push(States::ID state_id)
{
	_state_stack->push(state_id);
}

void State::request_stack_pop()
{
	_state_stack->pop();
}

void State::request_stack_clear()
{
	_state_stack->clear();
}