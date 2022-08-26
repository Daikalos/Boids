#include "MainState.h"

MainState::MainState(StateStack& stack, Context context, Config& config) 
    : State(stack, context), _config(&config), _debug(*_config), _audio_meter(*_config, 1.0f)
{
    context._texture_holder->load(TextureID::Background, _config->background_texture);
    context._font_holder->load(FontID::F8Bit, "font_8bit.ttf");

    _debug.load(*context._font_holder);
    _audio_meter.initialize();

    _background.load_texture(*context._texture_holder);
    _background.load_prop(*_config, sf::Vector2i(context._window->getSize()));

    _min_distance = std::sqrtf(std::fmaxf(std::fmaxf(_config->sep_distance, _config->ali_distance), _config->coh_distance));

    _border = context._window->get_border();

    RectFloat grid_border = (RectFloat)_border + (_config->turn_at_border ?
        RectFloat(
            -_min_distance * _config->grid_extra_cells,
            -_min_distance * _config->grid_extra_cells,
            +_min_distance * _config->grid_extra_cells,
            +_min_distance * _config->grid_extra_cells) : RectFloat());

    _grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);

    _boids.reserve(_config->boid_count);
    _sorted_boids.reserve(_config->boid_count);

    for (int i = 0; i < _config->boid_count; ++i)
    {
        sf::Vector2f pos = sf::Vector2f(
            util::random(0, _border.width()) - _border.left,
            util::random(0, _border.height()) - _border.top);

        _boids.emplace_back(_grid, *_config, _audio_meter, _border, pos);
        _sorted_boids.emplace_back(i);
    }

	_vertices.resize(_config->boid_count * 3);
	_colors.resize(_vertices.size());

	glVertexPointer(2, GL_FLOAT, 0, _vertices.data());
	glColorPointer(3, GL_FLOAT, 0, _colors.data());

    _policy = _config->boid_count <= 1500 ? Policy::unseq : Policy::par_unseq;
}

bool MainState::handle_event(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::Resized:
        {
			_border = context()._window->get_border();

            RectFloat grid_border = (RectFloat)_border + (_config->turn_at_border ?
                RectFloat(
                    -_min_distance * _config->grid_extra_cells,
                    -_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells) : RectFloat());

            _grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);

            _background.load_prop(*_config, sf::Vector2i(context()._window->getSize()));
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
		for (const Reconstruct& reconstruct : _config->refresh(prev)) // not very clean, but it does not matter much for small project
		{
			switch (reconstruct)
			{
			case Reconstruct::RGrid:
				{
					_min_distance = std::sqrtf(std::fmaxf(std::fmaxf(_config->sep_distance, _config->ali_distance), _config->coh_distance));

					RectFloat grid_border = (RectFloat)_border + (_config->turn_at_border ?
						RectFloat(
							-_min_distance * _config->grid_extra_cells,
							-_min_distance * _config->grid_extra_cells,
							+_min_distance * _config->grid_extra_cells,
							+_min_distance * _config->grid_extra_cells) : RectFloat());

					_grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);
				}
				break;
			case Reconstruct::RBoids:
				{
					_policy = _config->boid_count <= 1500 ? Policy::unseq : Policy::par_unseq;

					_vertices.resize(_config->boid_count * 3);
					_colors.resize(_vertices.size());

					glVertexPointer(2, GL_FLOAT, 0, _vertices.data()); // attach pointers again in case of reallocation
					glColorPointer(3, GL_FLOAT, 0, _colors.data());

					if (_config->boid_count > prev.boid_count) // new is larger
					{
						_boids.reserve(_config->boid_count);
						_sorted_boids.reserve(_config->boid_count);

						for (int i = prev.boid_count; i < _config->boid_count; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::random(0, _border.width()) - _border.left,
								util::random(0, _border.height()) - _border.top);

							_boids.emplace_back(_grid, *_config, _audio_meter, _border, pos);
							_sorted_boids.emplace_back(i);
						}
					}
					else
					{
						_boids.erase(_boids.begin() + _config->boid_count, _boids.end());

						_sorted_boids.erase(std::remove_if(
							_sorted_boids.begin(), _sorted_boids.end(),
							[this](const int index)
							{
								return index >= _config->boid_count;
							}), _sorted_boids.end());
					}
				}
				break;
			case Reconstruct::RBoidsCycle:	
				{
					for (Boid& boid : _boids)
						boid.set_cycle_time(_config->boid_cycle_colors_random ? util::random(0.0f, 1.0f) : 0.0f);
				}
				break;
			case Reconstruct::RBackgroundTex:
				{
					context()._texture_holder->load(TextureID::Background, _config->background_texture);

					_background.load_texture(*context()._texture_holder);
					_background.load_prop(*_config, sf::Vector2i(context()._window->getSize()));
				}
				break;
			case Reconstruct::RBackgroundProp:
				{
					_background.load_prop(*_config, sf::Vector2i(context()._window->getSize()));

					sf::Vector3f vc = _config->background_color * 255.0f;
					context()._window->set_clear_color(sf::Color(vc.x, vc.y, vc.z, 255.0f));
				}
				break;
			case Reconstruct::RAudio:
				_audio_meter.clear();
				break;
			case Reconstruct::RWindow:
				context()._window->setVerticalSyncEnabled(_config->vertical_sync);
				context()._window->setFramerateLimit(_config->vertical_sync ? 0 : _config->max_framerate);
				break;
			case Reconstruct::RCamera:
				context()._camera->set_scale(sf::Vector2f(_config->camera_zoom, _config->camera_zoom));
				break;
			}
		}
	}

    return true;
}

bool MainState::update(float dt)
{
	_audio_meter.update(dt);

	_mouse_pos = sf::Vector2f(context()._camera->
		get_mouse_world_position(*context()._window));

	if (_config->impulse_enabled && context().input_handler->get_button_pressed(sf::Mouse::Button::Left))
		_impulses.push_back(Impulse(_mouse_pos, _config->impulse_speed, _config->impulse_size, -_config->impulse_size));

	std::for_each(_impulses.rbegin(), _impulses.rend(),
		[&dt, this](Impulse& impulse)
		{
			impulse.update(dt);

			if (impulse.get_length() > _config->impulse_fade_distance)
				_impulses.erase(_impulses.begin() + (&impulse - _impulses.data()));
		});

    return true;
}

bool MainState::fixed_update(float dt)
{
	_grid.reset_buffers();

	policy_select(
		[&dt, this](auto& pol)
		{
			std::for_each(pol, _boids.begin(), _boids.end(),
				[](Boid& boid) { boid.set_cell_index(); });

			std::sort(pol, _sorted_boids.begin(), _sorted_boids.end(),
				[this](const int& i0, const int& i1) { return _boids[i0].get_cell_index() < _boids[i1].get_cell_index(); });

			std::for_each(pol, _sorted_boids.begin(), _sorted_boids.end(),
				[this](const int& index) { _boids[index].update_grid_cells(_boids, _sorted_boids, &index - _sorted_boids.data()); });

			std::for_each(pol, _boids.begin(), _boids.end(),
				[&dt, this](Boid& boid)
				{
					bool hold_left = context().input_handler->get_button_held(sf::Mouse::Button::Left);
					bool hold_right = context().input_handler->get_button_held(sf::Mouse::Button::Right);

					if (_config->steer_enabled || _config->predator_enabled)
					{
						sf::Vector2f dir = vu::direction(boid.get_saved_origin(), _mouse_pos);

						if (_config->steer_enabled)
						{
							float weight = 0.0f;
							float length_opt = 0.0f;

							if (hold_left || hold_right)
							{
								length_opt = vu::distance_opt(dir);
								weight = 15.0f / std::sqrtf(length_opt); // hard coded because cant update _config->.. 
							}

							if (hold_left)
								boid.steer_towards(dir, length_opt, _config->steer_towards_factor * weight);
							if (hold_right)
								boid.steer_towards(dir, length_opt, -_config->steer_away_factor * weight);
						}

						if (_config->predator_enabled && !(_config->steer_enabled && (hold_left || hold_right)))
						{
							float length_sq = vu::distance_sq(dir);
							if (length_sq <= _config->predator_distance)
							{
								float weight = std::sqrtf(length_sq / _config->predator_distance);
								boid.steer_towards(dir, -_config->predator_factor / weight);
							}
						}
					}
				});

			std::for_each(pol, _boids.begin(), _boids.end(),
				[&dt, this](Boid& boid)
				{
					boid.update(_boids, _sorted_boids, _impulses, dt);
				});

		}, _policy);

    return true;
}

bool MainState::post_update(float dt, float interp)
{
	std::for_each(
		std::execution::par_unseq, _boids.begin(), _boids.end(),
		[&interp, this](const Boid& boid)
		{
			int v = (&boid - _boids.data()) * 3;

			sf::Vector2f pos = boid.get_position();
			sf::Vector2f prev_pos = boid.get_prev_position();

			sf::Vector2f origin = boid.get_origin();
			sf::Vector2f prev_origin = boid.get_prev_origin();

			float rot = vu::angle(boid.get_velocity());
			float prev_rot = vu::angle(boid.get_prev_velocity());

			sf::Vector2f pointA = vu::rotate_point({ pos.x + _config->boid_size_width	, pos.y + (_config->boid_size_height / 2)	}, origin, rot); // middle right tip
			sf::Vector2f pointB = vu::rotate_point({ pos.x								, pos.y										}, origin, rot); // top left corner
			sf::Vector2f pointC = vu::rotate_point({ pos.x								, pos.y + _config->boid_size_height			}, origin, rot); // bot left corner

			sf::Vector2f prev_pointA = vu::rotate_point({ prev_pos.x + _config->boid_size_width	, prev_pos.y + (_config->boid_size_height / 2)	}, prev_origin, prev_rot); // middle right tip
			sf::Vector2f prev_pointB = vu::rotate_point({ prev_pos.x							, prev_pos.y									}, prev_origin, prev_rot); // top left corner
			sf::Vector2f prev_pointC = vu::rotate_point({ prev_pos.x							, prev_pos.y + _config->boid_size_height			}, prev_origin, prev_rot); // bot left corner

			sf::Vector2f p0 = pointA * interp + prev_pointA * (1.0f - interp);
			sf::Vector2f p1 = pointB * interp + prev_pointB * (1.0f - interp);
			sf::Vector2f p2 = pointC * interp + prev_pointC * (1.0f - interp);

			_vertices[v + 0] = *(Vertex*)(&p0);
			_vertices[v + 1] = *(Vertex*)(&p1);
			_vertices[v + 2] = *(Vertex*)(&p2);

			sf::Vector3f color = boid.get_color();

			_colors[v + 0] = *(Color*)(&color);
			_colors[v + 1] = *(Color*)(&color);
			_colors[v + 2] = *(Color*)(&color);
		});

    return true;
}

void MainState::draw()
{
	_background.draw(*context()._window);

	glPushMatrix();
	glLoadMatrixf(context()._camera->get_world_matrix());
	glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
	glPopMatrix();

	_debug.draw(*context()._window);
}
