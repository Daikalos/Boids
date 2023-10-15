#include "State.h"

State::State(const Context& context)
	: m_context(context) { }

State::~State() = default;

auto State::GetContext() const -> const Context&
{
	return m_context;
};