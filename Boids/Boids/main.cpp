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

	Config::load();

	if (Config::vertical_sync)
		window.setVerticalSyncEnabled(Config::vertical_sync);
	else
		window.setFramerateLimit(Config::max_framerate);

	sf::Clock clock;
	float deltaTime = 1.0f / std::fmaxf(Config::physics_update_freq, 1.0f);
	float rDeltaTime = FLT_EPSILON;
	float accumulator = FLT_EPSILON;

	ResourceManager resourceManager;
	resourceManager.load_textures();
	resourceManager.load_fonts();

	Camera camera(&window);
	InputHandler inputHandler;

	Debug debug;
	debug.load(resourceManager);

	Background background;
	background.load(resourceManager, video_mode);

	AudioMeter audioMeter(1.0f);
	audioMeter.initialize();

	sf::Vector2f mouse_pos;

	Rect_i border(0, 0, video_mode.size.x, video_mode.size.y);
	Grid grid(
		border.left	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.top	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.right + Config::min_distance * (Config::grid_extra_cells + 1),
		border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
		Config::min_distance * 2.0f, Config::min_distance * 2.0f);

	GLsizei vertex_count = Config::boid_count * 3;
	State state(vertex_count);

	Boid* boids = (Boid*)::operator new(Config::boid_count * sizeof(Boid));
	for (int i = 0; i < Config::boid_count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0, border.width()) - border.left,
			util::random(0, border.height()) - border.top);

		new(boids + i) Boid(&grid, boids, pos);
	}

	state.update(boids, 1.0f);

	glClearColor(
		Config::background_color.x, 
		Config::background_color.y,
		Config::background_color.z, 1.0f);

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
							border.left  - Config::min_distance * (Config::grid_extra_cells + 1),
							border.top   - Config::min_distance * (Config::grid_extra_cells + 1),
							border.right + Config::min_distance * (Config::grid_extra_cells + 1),
							border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
							Config::min_distance * 2.0f, Config::min_distance * 2.0f);
					}
					break;
			}
		}

		audioMeter.update(rDeltaTime);
		debug.update(inputHandler, rDeltaTime);
		camera.update(inputHandler);

		mouse_pos = sf::Vector2f(camera.get_mouse_world_position());

		if (inputHandler.get_left_pressed())
			Config::impulses.push_back(Impulse(mouse_pos, Config::impulse_speed, Config::impulse_size, 0.0f));

		for (int i = Config::impulses.size() - 1; i >= 0; --i)
		{
			Impulse& impulse = Config::impulses[i];

			impulse.update(rDeltaTime);

			if (impulse.get_length() > Config::impulse_fade_distance)
				Config::impulses.erase(Config::impulses.begin() + i);
		}

		while (accumulator >= deltaTime)
		{
			grid.reset_buffers();

			std::for_each(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](Boid& boid)
				{
					boid.set_cell_index();
				});

			std::sort(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](const Boid& b0, const Boid& b1)
				{
					return b0.get_cell_index() < b1.get_cell_index();
				});

			std::for_each(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](const Boid& boid)
				{
					boid.update_grid_cells();
				});

			std::for_each(
				std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[&](Boid& boid)
				{
					boid.steer_towards(mouse_pos, Config::steer_towards_factor * Config::steer_enabled * inputHandler.get_left_held());
					boid.steer_towards(mouse_pos, -Config::steer_away_factor * Config::steer_enabled * inputHandler.get_right_held());

					if (Config::predator_enabled && !(Config::steer_enabled && (inputHandler.get_left_held() || inputHandler.get_right_held())))
					{
						float dist = v2f::distance_squared(boid.get_origin(), mouse_pos);

						if (dist <= Config::predator_distance)
						{
							float factor = (dist > FLT_EPSILON) ? std::sqrtf(dist / Config::predator_distance) : FLT_EPSILON;
							boid.steer_towards(mouse_pos, -Config::predator_factor / factor);
						}
					}

					boid.update(deltaTime, border);
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