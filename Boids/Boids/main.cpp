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

	sf::RenderWindow window(video_mode, "Boids"); //sf::Style::Fullscreen);
	
	if (!window.setActive(true))
		return -1;

	Config config;
	config.load();

	if (config.vertical_sync)
		window.setVerticalSyncEnabled(config.vertical_sync);
	else
		window.setFramerateLimit(config.max_framerate);

	sf::Vector2f mouse_pos;
	Rect_i border(0, 0, video_mode.size.x, video_mode.size.y);

	sf::Clock clock;
	float deltaTime = 1.0f / std::fmaxf(config.physics_update_freq, 1.0f);
	float rDeltaTime = FLT_EPSILON;
	float accumulator = FLT_EPSILON;

	ResourceManager resourceManager;
	resourceManager.load_texture("background", "content/" + config.background_texture);
	resourceManager.load_font("8bit", "content/font_8bit.ttf");

	Camera camera(&window, &config);
	InputHandler inputHandler;

	Debug debug(&config);
	debug.load(resourceManager);

	Background background;
	background.load_texture(resourceManager);
	background.load_prop(&config, video_mode);

	AudioMeter audioMeter(&config, 1.0f);
	audioMeter.initialize();

	Grid grid(
		border.left	 - config.min_distance * (config.grid_extra_cells + 1),
		border.top	 - config.min_distance * (config.grid_extra_cells + 1),
		border.right + config.min_distance * (config.grid_extra_cells + 1),
		border.bot   + config.min_distance * (config.grid_extra_cells + 1),
		config.min_distance * 2.0f, config.min_distance * 2.0f);

	GLsizei vertex_count = config.boid_count * 3;
	State state(vertex_count, &config);

	std::vector<Boid> boids;
	boids.reserve(config.boid_count);

	for (int i = 0; i < config.boid_count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0, border.width()) - border.left,
			util::random(0, border.height()) - border.top);

		boids.emplace_back(Boid(&grid, &config, pos));
	}

	glClearColor(
		config.background_color.x, 
		config.background_color.y,
		config.background_color.z, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	glOrtho(0, window.getSize().x, 0, window.getSize().y, -1.0, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, state.get_vertices());
	glColorPointer(3, GL_FLOAT, 0, state.get_colors());

	while (window.isOpen())
	{
		rDeltaTime = std::fminf(clock.restart().asSeconds(), 0.075f);
		accumulator += rDeltaTime;

		inputHandler.update();

		if (debug.update(inputHandler, rDeltaTime))
		{
			Config prev = config;

			std::vector<Reconstruct> reconstruct = config.refresh(prev);
			for (const Reconstruct& r : reconstruct)
			{
				switch (r)
				{
				case Reconstruct::RGrid:
					{
						grid = Grid(
							border.left  - config.min_distance * (config.grid_extra_cells + 1),
							border.top   - config.min_distance * (config.grid_extra_cells + 1),
							border.right + config.min_distance * (config.grid_extra_cells + 1),
							border.bot   + config.min_distance * (config.grid_extra_cells + 1),
							config.min_distance * 2.0f, config.min_distance * 2.0f);
					}
					break;
				case Reconstruct::RBoids:
					{
						vertex_count = config.boid_count * 3;

						state.resize(vertex_count);

						boids.clear();
						boids.reserve(config.boid_count);

						for (int i = 0; i < config.boid_count; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::random(0, border.width()) - border.left,
								util::random(0, border.height()) - border.top);

							boids.emplace_back(Boid(&grid, &config, pos));
						}

						glVertexPointer(2, GL_FLOAT, 0, state.get_vertices());
						glColorPointer(3, GL_FLOAT, 0, state.get_colors());
					}
					break;
				case Reconstruct::RBackgroundTex:
					resourceManager.load_texture("background", "content/" + config.background_texture);
					background.load_texture(resourceManager);
					background.load_prop(&config, video_mode);
					break;
				case Reconstruct::RBackgroundProp:
					background.load_prop(&config, video_mode);
					break;
				case Reconstruct::RAudio:
					audioMeter.clear();
					break;
				case Reconstruct::RWindow:
					if (config.vertical_sync)
						window.setVerticalSyncEnabled(config.vertical_sync);
					else
						window.setFramerateLimit(config.max_framerate);
					break;
				case Reconstruct::RCamera:
					camera.set_scale(config.camera_zoom);
					break;
				case Reconstruct::RPhysics:
					deltaTime = 1.0f / std::fmaxf(config.physics_update_freq, 1.0f);
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

						border = Rect_i(0, 0, window.getSize().x, window.getSize().y);
						camera.set_position((sf::Vector2f)window.getSize() / 2.0f);

						grid = Grid(
							border.left  - config.min_distance * (config.grid_extra_cells + 1),
							border.top   - config.min_distance * (config.grid_extra_cells + 1),
							border.right + config.min_distance * (config.grid_extra_cells + 1),
							border.bot   + config.min_distance * (config.grid_extra_cells + 1),
							config.min_distance * 2.0f, config.min_distance * 2.0f);
					}
					break;
			}
		}

		camera.update(inputHandler);
		audioMeter.update(rDeltaTime);

		mouse_pos = sf::Vector2f(camera.get_mouse_world_position());

		if (inputHandler.get_left_pressed())
			config.impulses.push_back(Impulse(mouse_pos, config.impulse_speed, config.impulse_size, 0.0f));

		for (int i = config.impulses.size() - 1; i >= 0; --i)
		{
			Impulse& impulse = config.impulses[i];

			impulse.update(rDeltaTime);

			if (impulse.get_length() > config.impulse_fade_distance)
				config.impulses.erase(config.impulses.begin() + i);
		}

		while (accumulator >= deltaTime)
		{
			grid.reset_buffers();

			std::for_each(std::execution::par_unseq, boids.begin(), boids.end(),
				[](Boid& boid)
				{
					boid.set_cell_index();
				});

			std::sort(std::execution::par_unseq, boids.begin(), boids.end(),
				[](const Boid& b0, const Boid& b1)
				{
					return b0.get_cell_index() < b1.get_cell_index();
				});

			std::for_each(std::execution::par_unseq, boids.begin(), boids.end(),
				[&boids](const Boid& boid)
				{
					boid.update_grid_cells(boids);
				});

			std::for_each(
				std::execution::par_unseq, boids.begin(), boids.end(),
				[&](Boid& boid)
				{
					boid.steer_towards(mouse_pos, config.steer_towards_factor * config.steer_enabled * inputHandler.get_left_held());
					boid.steer_towards(mouse_pos, -config.steer_away_factor * config.steer_enabled * inputHandler.get_right_held());

					if (config.predator_enabled && !(config.steer_enabled && (inputHandler.get_left_held() || inputHandler.get_right_held())))
					{
						float dist = v2f::distance_squared(boid.get_origin(), mouse_pos);

						if (dist <= config.predator_distance)
						{
							float factor = (dist > FLT_EPSILON) ? std::sqrtf(dist / config.predator_distance) : FLT_EPSILON;
							boid.steer_towards(mouse_pos, -config.predator_factor / factor);
						}
					}

					boid.update(boids, deltaTime, border);
				});

			accumulator -= deltaTime;
		}

		float interp = accumulator / deltaTime;
		state.update(boids, interp);

		glClear(GL_COLOR_BUFFER_BIT);

		window.pushGLStates();

		background.draw(window);
		debug.draw(window);

		window.popGLStates();

		glPushMatrix();

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.get_world_matrix());

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		glPopMatrix();

		window.display();
	}

	return 0;
}