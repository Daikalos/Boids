#include "MainState.h"

MainState::MainState(StateStack& stack, Context context, Config& config) : 
	State(stack, context), _config(&config), _debug(*_config)
{
    context.texture_holder->load(TextureID::Background, _config->background_texture);
    context.font_holder->load(FontID::F8Bit, "font_8bit.ttf");

#if defined(_WIN32)
	_audio_meter = AudioMeterInfoBase::ptr(new AudioMeterWin(*_config, 1.0f));
#else
	_audio_meter = AudioMeterInfoBase::ptr(new AudioMeterInfoBase());
#endif

	_audio_meter->initialize();

    _debug.load(*context.font_holder);

    _background.load_texture(*context.texture_holder);
    _background.load_prop(*_config, sf::Vector2i(context.window->getSize()));

    _min_distance = std::sqrtf(std::fmaxf(std::fmaxf(_config->sep_distance, _config->ali_distance), _config->coh_distance));

    _border = context.window->get_border();

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

        _boids.emplace_back(_grid, *_config, pos);
        _sorted_boids.emplace_back(i);
    }

	_vertices.resize(_config->boid_count * 3);
	_colors.resize(_vertices.size());

	glVertexPointer(2, GL_FLOAT, 0, _vertices.data());
	glColorPointer(3, GL_FLOAT, 0, _colors.data());

    _policy = _config->boid_count <= 2500 ? Policy::unseq : Policy::par_unseq;
}

bool MainState::handle_event(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::Resized:
        {
			_border = context().window->get_border();

            RectFloat grid_border = (RectFloat)_border + (_config->turn_at_border ?
                RectFloat(
                    -_min_distance * _config->grid_extra_cells,
                    -_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells,
                    +_min_distance * _config->grid_extra_cells) : RectFloat());

            _grid = Grid(*_config, grid_border, sf::Vector2f(_min_distance, _min_distance) * 2.0f);

            _background.load_prop(*_config, sf::Vector2i(context().window->getSize()));
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
					_policy = _config->boid_count <= 2500 ? Policy::unseq : Policy::par_unseq;

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

							_boids.emplace_back(_grid, *_config, pos);
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
					context().texture_holder->load(TextureID::Background, _config->background_texture);

					_background.load_texture(*context().texture_holder);
					_background.load_prop(*_config, sf::Vector2i(context().window->getSize()));
				}
				break;
			case Reconstruct::RBackgroundProp:
				{
					_background.load_prop(*_config, sf::Vector2i(context().window->getSize()));

					sf::Vector3f vc = _config->background_color * 255.0f;
					context().window->set_clear_color(sf::Color(vc.x, vc.y, vc.z, 255.0f));
				}
				break;
			case Reconstruct::RAudio:
				_audio_meter->clear();
				break;
			case Reconstruct::RWindow:
				context().window->set_framerate(_config->max_framerate);
				context().window->set_vertical_sync(_config->vertical_sync);
				break;
			case Reconstruct::RCamera:
				context().camera->set_scale(sf::Vector2f(_config->camera_zoom, _config->camera_zoom));
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
		get_mouse_world_position(*context().window));

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
				[](Boid& boid) 
				{ 
					boid.pre_update(); 
				});

			std::sort(pol, _sorted_boids.begin(), _sorted_boids.end(),
				[this](const int& i0, const int& i1)
				{ 
					return _boids[i0].get_cell_index() < _boids[i1].get_cell_index(); 
				});

			std::for_each(pol, _sorted_boids.begin(), _sorted_boids.end(),
				[this](const int& index) 
				{ 
					_boids[index].update_grid_cells(_boids, _sorted_boids, &index - _sorted_boids.data());
				});

			std::for_each(pol, _boids.begin(), _boids.end(),
				[&dt, this](Boid& boid)
				{
					bool hold_left = context().input_handler->get_button_held(sf::Mouse::Button::Left);
					bool hold_right = context().input_handler->get_button_held(sf::Mouse::Button::Right);

					if (_config->steer_enabled && (hold_left || hold_right))
					{
						sf::Vector2f dir = vu::direction(boid.get_position(), _mouse_pos);

						const float factor = hold_left ? 1.0f : (hold_right ? -1.0f : 0.0f);

						const float length_opt = vu::distance_opt(dir);
						const float weight = 15.0f / std::sqrtf(length_opt); // hard coded because cant update _config->.. 

						boid.steer_towards(dir, length_opt, _config->steer_towards_factor * weight * factor);
					}
					else if (_config->predator_enabled)
					{
						sf::Vector2f dir = vu::direction(boid.get_position(), _mouse_pos);

						float length_sq = vu::distance_sq(dir);
						if (length_sq <= _config->predator_distance)
						{
							float weight = std::sqrtf(length_sq / _config->predator_distance);
							boid.steer_towards(dir, -_config->predator_factor / weight);
						}
					}

					boid.flock(_boids, _sorted_boids);
				});

			std::for_each(pol, _boids.begin(), _boids.end(),
				[&dt, this](Boid& boid)
				{
					boid.update(_border, _audio_meter, _impulses, dt);
				});

		}, _policy);

    return true;
}

bool MainState::post_update(float dt, float interp)
{
	policy_select(
		[&interp, this](auto& pol)
		{
			std::for_each(
				pol, _boids.begin(), _boids.end(),
				[&interp, this](const Boid& boid)
				{
					const auto v = (&boid - _boids.data()) * 3;

					sf::Vector3f color = boid.get_color();

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

					_vertices[v + 0] = p0;
					_vertices[v + 1] = p1;
					_vertices[v + 2] = p2;

					_colors[v + 0] = color;
					_colors[v + 1] = color;
					_colors[v + 2] = color;
				});
		}, _policy);

    return true;
}

void MainState::draw()
{
	_background.draw(*context().window);

	glPushMatrix();
	glLoadMatrixf(context().camera->get_world_matrix());
	glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
	glPopMatrix();

	_debug.draw(*context().window);
}
