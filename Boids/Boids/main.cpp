#include <execution>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Impulse.h"
#include "Boid.h"
#include "Grid.h"
#include "VecUtil.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Config.h"
#include "State.h"
#include "AudioMeter.h"
#include "ResourceManager.h"

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

	ResourceManager resourceManager;
	resourceManager.load_textures();
	resourceManager.load_fonts();

	sf::Texture* background_texture = resourceManager.request_texture("background");
	sf::Sprite background;

	if (background_texture != nullptr)
		background.setTexture(*background_texture);

	// update background
	{
		sf::Vector2f desired_scale = sf::Vector2f(1.0f, 1.0f);

		if (Config::background_fit_screen)
			desired_scale = sf::Vector2f(
				video_mode.size.x / background.getLocalBounds().width,
				video_mode.size.y / background.getLocalBounds().height);
		else if (Config::background_override_size)
			desired_scale = sf::Vector2f(
				Config::background_width / background.getLocalBounds().width,
				Config::background_height / background.getLocalBounds().height);

		background.setScale(desired_scale);
		background.setPosition(sf::Vector2f(
			Config::background_position_x,
			Config::background_position_y));

		if (Config::background_color.x > FLT_EPSILON || Config::background_color.y > FLT_EPSILON || Config::background_color.z > FLT_EPSILON)
			background.setColor(sf::Color(
				Config::background_color.x * 255,
				Config::background_color.y * 255,
				Config::background_color.z * 255));
	}

	sf::Text debugText;
	bool debug_toggle = false;

	float debug_freq_max = 0.5f;
	float debug_freq = 0.0f;

	sf::Font* font = resourceManager.request_font("8bit");

	if (font != nullptr)
		debugText.setFont(*font);

	debugText.setPosition(sf::Vector2f(32, 32));
	debugText.setCharacterSize(26);

	AudioMeter audioMeter;
	audioMeter.initialize();

	sf::Clock clock;
	float deltaTime = 1.0f / std::fmaxf(Config::physics_update_freq, 1.0f);
	float rDeltaTime = FLT_EPSILON;
	float accumulator = FLT_EPSILON;

	Camera camera(&window);
	InputHandler inputHandler;

	sf::Vector2f mouse_pos;

	Rect_i border(0, 0, window.getSize().x, window.getSize().y);
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
		audioMeter.update(rDeltaTime);

		if (debug_toggle)
		{
			debug_freq -= rDeltaTime;

			if (debug_freq <= 0.0f)
			{
				std::string text = "FPS: " + std::to_string(1.0f / rDeltaTime);
				debugText.setString(text);

				debug_freq = debug_freq_max;
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
							border.left  - Config::min_distance * (Config::grid_extra_cells + 1),
							border.top   - Config::min_distance * (Config::grid_extra_cells + 1),
							border.right + Config::min_distance * (Config::grid_extra_cells + 1),
							border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
							Config::min_distance * 2.0f, Config::min_distance * 2.0f);
					}
					break;
			}
		}

		camera.update(inputHandler);
		mouse_pos = sf::Vector2f(camera.get_mouse_world_position());

		if (Config::debug_enabled && inputHandler.get_key_pressed(static_cast<sf::Keyboard::Key>(Config::debug_key)))
			debug_toggle = !debug_toggle;

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

		window.draw(debugText);

		window.draw(background);

		if (debug_toggle)
			window.draw(debugText);

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