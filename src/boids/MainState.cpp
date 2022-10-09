#include "MainState.h"

MainState::MainState(StateStack& stack, Context context, Config& config) : 
	State(stack, context), _window(context.window), _config(&config), _debug(*_config), _fluid(*_config, _window->getSize())
{
    context.texture_holder->load(TextureID::Background, _config->background_texture);
    context.font_holder->load(FontID::F8Bit, "font_8bit.ttf");

#if defined(_WIN32)
	_audio_meter = std::make_unique<AudioMeterWin>(*_config, 1.0f);
#else
	_audio_meter = std::make_unique<AudioMeterEmpty>();
#endif

	_audio_meter->initialize();

    _debug.load(*context.font_holder);

    _background.load_texture(*context.texture_holder);
    _background.load_prop(*_config, sf::Vector2i(_window->getSize()));

    _min_distance = std::sqrtf(std::fmaxf(std::fmaxf(_config->sep_distance, _config->ali_distance), _config->coh_distance));

    _border = _window->get_border();

    RectFloat grid_border = _border + (_config->turn_at_border ?
        RectFloat(
            -_min_distance * _config->grid_extra_cells,
            -_min_distance * _config->grid_extra_cells,
            +_min_distance * _config->grid_extra_cells,
            +_min_distance * _config->grid_extra_cells) : RectFloat());

    _grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);

    _boids.reserve(_config->boid_count);
	_proxy.reserve(_config->boid_count);

    for (int i = 0; i < _config->boid_count; ++i)
    {
        sf::Vector2f pos = sf::Vector2f(
            util::random(0.0f, _border.width()) - _border.left,
            util::random(0.0f, _border.height()) - _border.top);

		_boids.emplace_back(*_config, pos);
		_proxy.emplace_back(i);
    }

	_vertices.resize(_boids.size() * 3);
	_vertices.setPrimitiveType(sf::Triangles);

	_fluid_mouse_pos = sf::Vector2i(context.camera->
		get_mouse_world_position(*_window)) / _config->fluid_scale;
	_fluid_mouse_pos_prev = _fluid_mouse_pos;

    _policy = _config->boid_count <= _config->policy_threshold ? Policy::unseq : Policy::par_unseq;
}

bool MainState::handle_event(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::Resized:
        {
			_background.load_prop(*_config, sf::Vector2i(_window->getSize()));

			_border = _window->get_border();

            RectFloat grid_border = _border + (_config->turn_at_border ?
                RectFloat(
                    -_min_distance * _config->grid_extra_cells,
                    -_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells) : RectFloat());

            _grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);
			_fluid = Fluid(*_config, _window->getSize());
        }
        break;
    }
    return false;
}

bool MainState::pre_update(float dt)
{
    _debug.update(*context().input_handler, dt);

	if (_debug.get_refresh()) // time to refresh data
	{
		Config prev = *_config;
		for (const Rebuild& rebuild : _config->refresh(prev)) // not very clean, but it does not matter much for small project
		{
			switch (rebuild)
			{
			case RB_Grid:
				{
					_min_distance = std::sqrtf(std::fmaxf(std::fmaxf(_config->sep_distance, _config->ali_distance), _config->coh_distance));

					RectFloat grid_border = _border + (_config->turn_at_border ?
						RectFloat(
							-_min_distance * _config->grid_extra_cells,
							-_min_distance * _config->grid_extra_cells,
							+_min_distance * _config->grid_extra_cells,
							+_min_distance * _config->grid_extra_cells) : RectFloat());

					_grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);
				}
				break;
			case RB_Boids:
				{
					if (_config->boid_count > prev.boid_count) // new is larger
					{
						_boids.reserve(_config->boid_count);
						_proxy.reserve(_config->boid_count);

						for (int i = prev.boid_count; i < _config->boid_count; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::random(0.0f, _border.width()) - _border.left,
								util::random(0.0f, _border.height()) - _border.top);

							_boids.emplace_back(*_config, pos);
							_proxy.emplace_back(i);
						}
					}
					else
					{
						_boids.erase(_boids.begin() + _config->boid_count, _boids.end());

						_proxy.erase(std::remove_if(
							_proxy.begin(), _proxy.end(),
							[this](const int index)
							{
								return index >= _boids.size();
							}), _proxy.end());
					}

					_vertices.resize(_boids.size() * 3);
					_policy = _boids.size() <= _config->policy_threshold ? Policy::unseq : Policy::par_unseq;
				}
				break;
			case RB_BoidsCycle:
				{
					for (Boid& boid : _boids)
					{
						boid.set_cycle_time(_config->boid_cycle_colors_random ?
							util::random(0.0f, 1.0f) : 0.0f);
					}
				}
				break;
			case RB_BackgroundTex:
				{
					context().texture_holder->load(TextureID::Background, _config->background_texture);

					_background.load_texture(*context().texture_holder);
					_background.load_prop(*_config, sf::Vector2i(_window->getSize()));
				}
				break;
			case RB_BackgroundProp:
				{
					_background.load_prop(*_config, sf::Vector2i(_window->getSize()));

					_window->set_clear_color(sf::Color(
						(sf::Uint8)(_config->background_color.x * 255.0f),
						(sf::Uint8)(_config->background_color.y * 255.0f),
						(sf::Uint8)(_config->background_color.z * 255.0f)));
				}
				break;
			case RB_Audio:
				_audio_meter->clear();
				break;
			case RB_Window:
				_window->set_framerate(_config->max_framerate);
				_window->set_vertical_sync(_config->vertical_sync);
				break;
			case RB_Camera:
				context().camera->set_scale(sf::Vector2f(_config->camera_zoom, _config->camera_zoom));
				break;
			case RB_Fluid:
				_fluid = Fluid(*_config, _window->getSize());
				break;
			}
		}
	}

    return true;
}

bool MainState::update(float dt)
{
	_audio_meter->update(dt);

	_mouse_pos = sf::Vector2f(context().camera->
		get_mouse_world_position(*_window));

	if (context().input_handler->get_button_held(sf::Mouse::Middle))
	{
		const sf::Vector2f amount = vu::abs(_mouse_pos - _mouse_pos_prev);
		if (vu::distance(amount) > _config->boid_add_mouse_diff)
		{
			for (int i = 0; i < _config->boid_add_amount; ++i)
			{
				_boids.emplace_back(*_config, _mouse_pos);
				_proxy.push_back((std::uint32_t)(_boids.size() - 1));
			}

			_vertices.resize(_boids.size() * 3);
			_policy = _boids.size() <= _config->policy_threshold ? Policy::unseq : Policy::par_unseq;
		}
	}
	if (context().input_handler->get_key_held(sf::Keyboard::Key::Delete))
	{
		if (_boids.size() > _config->boid_count)
		{
			std::size_t remove_amount = (_boids.size() - _config->boid_count) >= _config->boid_remove_amount ? 
				_config->boid_remove_amount : _boids.size() - _config->boid_count;

			for (std::size_t i = 0; i < remove_amount; ++i)
				_boids.pop_back();

			_proxy.erase(std::remove_if(
				_proxy.begin(), _proxy.end(),
				[this](const int index)
				{
					return index >= _boids.size();
				}), _proxy.end());

			_vertices.resize(_boids.size() * 3);
			_policy = _boids.size() <= _config->policy_threshold ? Policy::unseq : Policy::par_unseq;
		}
	}

	_mouse_pos_prev = _mouse_pos;

	if (_config->impulse_enabled && context().input_handler->get_button_pressed(sf::Mouse::Button::Left))
		_impulses.push_back(Impulse(_mouse_pos, _config->impulse_speed, _config->impulse_size, -_config->impulse_size));

	std::for_each(_impulses.rbegin(), _impulses.rend(),
		[&dt, this](Impulse& impulse)
		{
			impulse.update(dt);

			if (impulse.get_length() > _config->impulse_fade_distance)
				_impulses.erase(_impulses.begin() + (&impulse - _impulses.data()));
		});

	if (_config->color_flags & CF_Fluid)
	{
		_fluid_mouse_pos = sf::Vector2i(_mouse_pos / (float)_config->fluid_scale);
		const sf::Vector2i amount = vu::abs(_fluid_mouse_pos - _fluid_mouse_pos_prev);

		_fluid.step_line(
			_fluid_mouse_pos_prev.x, _fluid_mouse_pos_prev.y,
			_fluid_mouse_pos.x, _fluid_mouse_pos.y,
			amount.x, amount.y, _config->fluid_mouse_strength);

		_fluid_mouse_pos_prev = _fluid_mouse_pos;

		_fluid.update(dt);
	}

    return true;
}

bool MainState::fixed_update(float dt)
{
	_grid.reset_buffers();

	std::for_each(_boids.begin(), _boids.end(),
		[this](Boid& boid)
		{
			boid.pre_update(_grid);
		});

	std::sort(_proxy.begin(), _proxy.end(),
		[this](const std::uint32_t i0, const std::uint32_t i1)
		{
			return _boids[i0].get_cell_index() < _boids[i1].get_cell_index();
		});

	std::for_each(_proxy.begin(), _proxy.end(),
		[this](const std::uint32_t& index)
		{
			_boids[index].update_grid_cells(_grid, _boids, _proxy, (std::uint32_t)(&index - _proxy.data()));
		});

	policy_select(
		[this](auto& pol)
		{
			std::for_each(pol, _boids.begin(), _boids.end(),
				[this](Boid& boid)
				{
					bool hold_left = context().input_handler->get_button_held(sf::Mouse::Button::Left);
					bool hold_right = context().input_handler->get_button_held(sf::Mouse::Button::Right);

					if (_config->steer_enabled && (hold_left || hold_right))
					{
						sf::Vector2f dir = vu::direction(boid.get_position(), _mouse_pos);

						const float factor = hold_left ? 1.0f : 
							(hold_right ? -1.0f : 0.0f);

						const float length_opt = vu::distance_opt(dir);
						const float weight = 15.0f / (std::sqrtf(length_opt) + FLT_EPSILON); // hard coded because cant update config.. 

						boid.steer_towards(dir, length_opt, _config->steer_towards_factor * weight * factor);
					}
					else if (_config->predator_enabled)
					{
						sf::Vector2f dir = vu::direction(boid.get_position(), _mouse_pos);

						float length_sq = vu::distance_sq(dir);
						if (length_sq <= _config->predator_distance)
						{
							float weight = std::sqrtf(length_sq / _config->predator_distance);
							boid.steer_towards(dir, -_config->predator_factor / (weight + FLT_EPSILON));
						}
					}

					boid.flock(_grid, _boids, _proxy);
				});
		}, _policy);


	std::for_each(_boids.begin(), _boids.end(),
		[&dt, this](Boid& boid)
		{
			boid.update(_border, _audio_meter.get(), _impulses, dt);
		});

    return true;
}

bool MainState::post_update(float dt, float interp)
{
	policy_select(
		[&interp, this](auto& pol)
		{
			std::for_each(pol, _boids.begin(), _boids.end(),
				[&interp, this](const Boid& boid)
				{
					const sf::Vector2f ori = boid.get_origin();
					const sf::Vector2f prev_ori = boid.get_prev_origin();

					const float rot = vu::angle(boid.get_velocity());
					const float prev_rot = vu::angle(boid.get_prev_velocity());

					const sf::Vector2f pointA = vu::rotate_point({ ori.x + (_config->boid_size_width / 2), ori.y									}, ori, rot); // middle right tip
					const sf::Vector2f pointB = vu::rotate_point({ ori.x - (_config->boid_size_width / 2), ori.y - (_config->boid_size_height / 2)	}, ori, rot); // top left corner
					const sf::Vector2f pointC = vu::rotate_point({ ori.x - (_config->boid_size_width / 2), ori.y + (_config->boid_size_height / 2)	}, ori, rot); // bot left corner

					const sf::Vector2f prev_pointA = vu::rotate_point({ prev_ori.x + (_config->boid_size_width / 2), prev_ori.y										}, prev_ori, prev_rot); // middle right tip
					const sf::Vector2f prev_pointB = vu::rotate_point({ prev_ori.x - (_config->boid_size_width / 2), prev_ori.y - (_config->boid_size_height / 2)	}, prev_ori, prev_rot); // top left corner
					const sf::Vector2f prev_pointC = vu::rotate_point({ prev_ori.x - (_config->boid_size_width / 2), prev_ori.y + (_config->boid_size_height / 2)	}, prev_ori, prev_rot); // bot left corner

					const sf::Vector2f p0 = pointA * interp + prev_pointA * (1.0f - interp);
					const sf::Vector2f p1 = pointB * interp + prev_pointB * (1.0f - interp);
					const sf::Vector2f p2 = pointC * interp + prev_pointC * (1.0f - interp);

					sf::Vector3f bc0 = boid.get_color();
					sf::Vector3f bc1 = boid.get_color();
					sf::Vector3f bc2 = boid.get_color();

					if (_config->color_flags & CF_Fluid)
					{
						bc0 += _fluid.get_color(p0);
						bc1 += _fluid.get_color(p1);
						bc2 += _fluid.get_color(p2);
					}

					bc0.x = std::clamp(bc0.x, 0.0f, 1.0f);
					bc0.y = std::clamp(bc0.y, 0.0f, 1.0f);
					bc0.z = std::clamp(bc0.z, 0.0f, 1.0f);

					bc1.x = std::clamp(bc1.x, 0.0f, 1.0f);
					bc1.y = std::clamp(bc1.y, 0.0f, 1.0f);
					bc1.z = std::clamp(bc1.z, 0.0f, 1.0f);

					bc2.x = std::clamp(bc2.x, 0.0f, 1.0f);
					bc2.y = std::clamp(bc2.y, 0.0f, 1.0f);
					bc2.z = std::clamp(bc2.z, 0.0f, 1.0f);

					const sf::Color c0 = sf::Color(
						(sf::Uint8)(bc0.x * 255.0f), (sf::Uint8)(bc0.y * 255.0f), (sf::Uint8)(bc0.z * 255.0f));
					const sf::Color c1 = sf::Color(
						(sf::Uint8)(bc1.x * 255.0f), (sf::Uint8)(bc1.y * 255.0f), (sf::Uint8)(bc1.z * 255.0f));
					const sf::Color c2 = sf::Color(
						(sf::Uint8)(bc2.x * 255.0f), (sf::Uint8)(bc2.y * 255.0f), (sf::Uint8)(bc2.z * 255.0f));

					const auto v = (&boid - _boids.data()) * 3;

					_vertices[v + 0].position = p0;
					_vertices[v + 1].position = p1;
					_vertices[v + 2].position = p2;

					_vertices[v + 0].color = c0;
					_vertices[v + 1].color = c1;
					_vertices[v + 2].color = c2;
				});
		}, _policy);

    return true;
}

void MainState::draw()
{
	_background.draw(*_window);
	_window->draw(_vertices);
	_debug.draw(*_window);
}
