#include "Include.h"

int main()
{
	srand(time(NULL));

	std::vector<sf::VideoMode> fullscreen_modes = sf::VideoMode::getFullscreenModes();

	if (fullscreen_modes.size() == 0)
		return -1;

	sf::VideoMode video_mode = fullscreen_modes.front();

	if (!video_mode.isValid())
		return -1;

	sf::RenderWindow window(video_mode, "Boids", sf::Style::Fullscreen);
	
	if (!window.setActive(true))
		return -1;

	Config config;
	config.load();

	hide_taskbar_icon(window);

	window.setVerticalSyncEnabled(config.vertical_sync);
	window.setFramerateLimit(config.vertical_sync ? 0 : config.max_framerate);

	sf::Vector2f mouse_pos;
	RectInt border(0, 0, video_mode.size.x, video_mode.size.y);

	sf::Clock clock;
	float dt = FLT_EPSILON;

	float physics_dt = 1.0f / std::fmaxf(config.physics_update_freq, 1.0f);
	float accumulator = FLT_EPSILON;

	int ticks = 0;
	int death_spiral = 10;

	TextureHolder texture_holder;
	FontHolder font_holder;

	texture_holder.load(TextureID::Background, "content/" + config.background_texture);
	font_holder.load(FontID::F8Bit, "content/font_8bit.ttf");

	Camera camera(config);
	camera.set_size(sf::Vector2f(window.getSize().x, window.getSize().y));
	camera.set_position(sf::Vector2f(window.getSize()) / 2.0f);

	InputHandler input_handler;
	input_handler.set_button_binding(Binding::Button::Drag, sf::Mouse::Button::Middle);

	Debug debug(config);
	debug.load(font_holder);

	Background background;
	background.load_texture(texture_holder);
	background.load_prop(config, border.size());

	AudioMeter audio_meter(config, 1.0f);
	audio_meter.initialize();

	int min_distance = std::ceilf(std::sqrtf(std::fmaxf(std::fmaxf(config.sep_distance, config.ali_distance), config.coh_distance)));

	Grid grid(
		{
			border.left		- min_distance * (config.grid_extra_cells + 1),
			border.top		- min_distance * (config.grid_extra_cells + 1),
			border.right	+ min_distance * (config.grid_extra_cells + 1),
			border.bot		+ min_distance * (config.grid_extra_cells + 1) 
		}, 
		border, { min_distance * 2, min_distance * 2 });

	GLsizei vertex_count = config.boid_count * 3;
	State state(vertex_count);

	std::vector<Impulse> impulses;

	std::vector<Boid> boids;
	std::vector<int> sorted_boids;

	boids.reserve(config.boid_count);
	sorted_boids.reserve(config.boid_count);

	for (int i = 0; i < config.boid_count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0, border.width()) - border.left,
			util::random(0, border.height()) - border.top);

		boids.emplace_back(grid, config, audio_meter, border, pos);
		sorted_boids.emplace_back(i);
	}

	glViewport(0, 0, window.getSize().x, window.getSize().y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	glOrtho(0, window.getSize().x, 0, window.getSize().y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	Policy policy = config.boid_count <= 3000 ? Policy::unseq : Policy::par_unseq;

	sf::Vector3f vc = config.background_color * 255.0f;
	sf::Color background_color = sf::Color(vc.x, vc.y, vc.z, 255.0f);

	while (window.isOpen())
	{
		dt = std::fminf(clock.restart().asSeconds(), 0.075f);
		accumulator += dt;

		input_handler.update(dt);
		debug.update(input_handler, dt);

		if (debug.get_refresh())
		{
			Config prev = config;
			for (const Reconstruct& reconstruct : config.refresh(prev)) // not clean, but it does not matter much for small project
			{
				switch (reconstruct)
				{
				case Reconstruct::RGrid:
					{
						min_distance = std::sqrtf(std::fmaxf(std::fmaxf(config.sep_distance, config.ali_distance), config.coh_distance));

						grid = Grid(
							{
								border.left		- min_distance * (config.grid_extra_cells + 1),
								border.top		- min_distance * (config.grid_extra_cells + 1),
								border.right	+ min_distance * (config.grid_extra_cells + 1),
								border.bot		+ min_distance * (config.grid_extra_cells + 1)
							},
							border, { min_distance * 2, min_distance * 2 });
					}
					break;
				case Reconstruct::RBoids:
					{
						policy = config.boid_count <= 3000 ? Policy::unseq : Policy::par_unseq;

						vertex_count = config.boid_count * 3;
						state.resize(vertex_count);

						if (config.boid_count > prev.boid_count) // new is larger
						{
							boids.reserve(config.boid_count);
							sorted_boids.reserve(config.boid_count);

							for (int i = prev.boid_count; i < config.boid_count; ++i)
							{
								sf::Vector2f pos = sf::Vector2f(
									util::random(0, border.width()) - border.left,
									util::random(0, border.height()) - border.top);

								boids.emplace_back(grid, config, audio_meter, border, pos);
								sorted_boids.emplace_back(i);
							}
						}
						else
						{
							boids.erase(boids.begin() + config.boid_count, boids.end());

							sorted_boids.erase(std::remove_if(
								sorted_boids.begin(), sorted_boids.end(),
								[&config](const int& index)
								{
									return index >= config.boid_count;
								}), sorted_boids.end());
						}
					}
					break;
				case Reconstruct::RBoidsCycle:
					{
						for (Boid& boid : boids)
							boid.set_cycle_time(config.boid_cycle_colors_random ? util::random(0.0f, 1.0f) : 0.0f);
					}
					break;
				case Reconstruct::RBackgroundTex:
					texture_holder.load(TextureID::Background, "content/" + config.background_texture);
					background.load_texture(texture_holder);
					background.load_prop(config, border.size());
					break;
				case Reconstruct::RBackgroundProp:
					background.load_prop(config, border.size());

					vc = config.background_color * 255.0f;
					background_color = sf::Color(vc.x, vc.y, vc.z, 255.0f);
					break;
				case Reconstruct::RAudio:
					audio_meter.clear();
					break;
				case Reconstruct::RWindow:
					window.setVerticalSyncEnabled(config.vertical_sync);
					window.setFramerateLimit(config.vertical_sync ? 0 : config.max_framerate);
					break;
				case Reconstruct::RCamera:
					camera.set_scale(sf::Vector2f(config.camera_zoom, config.camera_zoom));
					break;
				case Reconstruct::RPhysics:
					physics_dt = 1.0f / std::fmaxf(config.physics_update_freq, 1.0f);
					break;
				case Reconstruct::RDebug:
					debug.set_update_freq(config.debug_update_freq);
					break;
				}
			}
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			input_handler.handle_event(event);
			camera.handle_event(event);

			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					{
						glViewport(0, 0, window.getSize().x, window.getSize().y);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						glScalef(1.0f, -1.0f, 1.0f);
						glOrtho(0, window.getSize().x, 0, window.getSize().y, -1.0, 1.0);
						glMatrixMode(GL_MODELVIEW);

						border = RectInt(0, 0, window.getSize().x, window.getSize().y);

						camera.set_size(sf::Vector2f(window.getSize()));
						camera.set_position(sf::Vector2f(window.getSize()) / 2.0f);

						grid = Grid(
							{
								border.left		- min_distance * (config.grid_extra_cells + 1),
								border.top		- min_distance * (config.grid_extra_cells + 1),
								border.right	+ min_distance * (config.grid_extra_cells + 1),
								border.bot		+ min_distance * (config.grid_extra_cells + 1)
							},
							border, { min_distance * 2, min_distance * 2 });

						background.load_prop(config, border.size());
					}
					break;
			}
		}

		camera.update(input_handler, window);
		audio_meter.update(dt);

		mouse_pos = sf::Vector2f(camera.get_mouse_world_position(window));

		if (config.impulse_enabled && input_handler.get_button_pressed(sf::Mouse::Button::Left))
			impulses.push_back(Impulse(mouse_pos, config.impulse_speed, config.impulse_size, -config.impulse_size));

		for (int i = impulses.size() - 1; i >= 0; --i)
		{
			Impulse& impulse = impulses[i];

			impulse.update(dt);

			if (impulse.get_length() > config.impulse_fade_distance)
				impulses.erase(impulses.begin() + i);
		}

		ticks = 0;
		while (accumulator >= physics_dt && ticks++ < death_spiral)
		{
			accumulator -= physics_dt;

			grid.reset_buffers();

			maybe_parallel(
				[&boids, &sorted_boids, &config, &input_handler, &impulses, &mouse_pos, &physics_dt](auto& pol)
				{
					std::for_each(pol, boids.begin(), boids.end(),
						[](Boid& boid) { boid.set_cell_index(); });

					std::sort(pol, sorted_boids.begin(), sorted_boids.end(),
						[&boids](const int& i0, const int& i1) { return boids[i0].get_cell_index() < boids[i1].get_cell_index(); });

					std::for_each(pol, sorted_boids.begin(), sorted_boids.end(),
						[&boids, &sorted_boids](const int& index) { boids[index].update_grid_cells(boids, sorted_boids, &index - sorted_boids.data()); });

					std::for_each(pol, boids.begin(), boids.end(),
						[&](Boid& boid)
						{
							boid.steer_towards(mouse_pos, config.steer_towards_factor * config.steer_enabled * input_handler.get_button_held(sf::Mouse::Button::Left));
							boid.steer_towards(mouse_pos, -config.steer_away_factor * config.steer_enabled * input_handler.get_button_held(sf::Mouse::Button::Right));

							if (config.predator_enabled && !(config.steer_enabled && (input_handler.get_button_held(sf::Mouse::Button::Left) || input_handler.get_button_held(sf::Mouse::Button::Right))))
							{
								float dist = v2f::length_sq(boid.get_origin(), mouse_pos);

								if (dist <= config.predator_distance)
								{
									float factor = (dist > FLT_EPSILON) ? std::sqrtf(dist / config.predator_distance) : FLT_EPSILON;
									boid.steer_towards(mouse_pos, -config.predator_factor / factor);
								}
							}

							boid.update(boids, sorted_boids, impulses, physics_dt);
						});

				}, policy);
		}

		float interp = accumulator / physics_dt;
		state.update(boids, config, interp);

		window.clear(background_color);
		window.setView(camera);

		background.draw(window);

		glPushMatrix();
		glLoadMatrixf(camera.get_world_matrix());
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		glPopMatrix();

		debug.draw(window);

		window.display();
	}

	return 0;
}