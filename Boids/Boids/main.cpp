#include <iostream>
#include <execution>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <winerror.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include "Impulse.h"
#include "Boid.h"
#include "Grid.h"
#include "VecUtil.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Config.h"
#include "State.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = NULL; } }

IMMDeviceEnumerator* pEnumerator = NULL;
IMMDevice* pDevice = NULL;
IAudioMeterInformation* pMeterInfo = NULL;
IAudioSessionManager2* pSessionManager = NULL;

std::map<std::wstring, std::pair<IAudioSessionControl2*, IAudioMeterInformation*>> processes_session_control;

std::mutex mutex;

void refresh(std::wstring* comp)
{
	IAudioSessionEnumerator* pSessionEnumerator = NULL;

	if (FAILED(pSessionManager->GetSessionEnumerator(&pSessionEnumerator)))
		return;

	int sessionCount = 0;
	if (FAILED(pSessionEnumerator->GetCount(&sessionCount)))
		return;

	for (int i = 0; i < sessionCount; ++i)
	{
		IAudioSessionControl* sessionControl = NULL;
		IAudioSessionControl2* sessionControl2 = NULL;
		IAudioMeterInformation* meterInformation = NULL;

		std::wstring process_name;

		if (SUCCEEDED(pSessionEnumerator->GetSession(i, &sessionControl)))
		{
			AudioSessionState state;
			if (SUCCEEDED(sessionControl->GetState(&state)) && state != AudioSessionStateExpired)
			{
				if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2)))
				{
					LPWSTR sessionID;
					if (SUCCEEDED(sessionControl2->GetSessionInstanceIdentifier(&sessionID)))
					{
						_wcslwr_s(sessionID, wcslen(sessionID) + 1);

						for (int j = 0; j < Config::audio_responsive_processes.size(); ++j)
						{
							process_name = Config::audio_responsive_processes[j];

							if (process_name.size() == 0)
								continue;

							if (comp != nullptr && *comp != process_name)
								continue;

							if (wcsstr(sessionID, process_name.c_str()) != 0)
							{
								if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&meterInformation)))
								{
									break;
								}
							}
						}
					}
				}
			}
		}

		if (sessionControl2 && meterInformation)
			processes_session_control[process_name] = std::make_pair(sessionControl2, meterInformation);

		SAFE_RELEASE(sessionControl);
	}

	SAFE_RELEASE(pSessionEnumerator);
}

bool initialize_audio()
{
	HRESULT hr;

	if (FAILED(CoInitialize(NULL)))
		return false;

	// Get enumerator for audio endpoint devices.
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator)))
		return false;

	// Get peak meter for default audio-rendering device.
	if (FAILED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice)))
		return false;

	if (FAILED(pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pSessionManager)))
		return false;

	if (Config::audio_responsive_processes.size() > 0)
	{
		refresh(nullptr);
	}
	else
	{
		if (FAILED(pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo)))
			return false;
	}

	return true;
}

int main()
{
	srand(time(NULL));
	Config::load();

	sf::Window window(sf::VideoMode::getDesktopMode(), "Boids");// sf::Style::Fullscreen);

	if (!Config::vertical_sync)
		window.setFramerateLimit(Config::max_framerate);
	else
		window.setVerticalSyncEnabled(Config::vertical_sync);
	
	if (!window.setActive(true))
		return -1;

	Rect_i border(0, 0, window.getSize().x, window.getSize().y);

	Camera camera(&window);
	InputHandler inputHandler;

	float refresh_freq_max = 1.0f;
	float refresh_freq = refresh_freq_max;

	if (Config::color_option == 3)
	{
		if (!initialize_audio())
		{
			SAFE_RELEASE(pEnumerator);
			SAFE_RELEASE(pDevice);
			SAFE_RELEASE(pMeterInfo);
			CoUninitialize();
		}
	}

	sf::Clock clock;
	float deltaTime = 1.0f / 90.0f;
	float rDeltaTime = FLT_EPSILON;
	float accumulator = FLT_EPSILON;

	sf::Vector2f mouse_pos;
	GLsizei vertex_count = Config::boid_count * 3;

	Boid* boids = (Boid*)::operator new(Config::boid_count * sizeof(Boid));

	Grid grid(
		border.left	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.top	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.right + Config::min_distance * (Config::grid_extra_cells + 1),
		border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
		Config::min_distance * 2.0f, Config::min_distance * 2.0f);

	State state(vertex_count);

	for (int i = 0; i < Config::boid_count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0, border.width()) - border.left,
			util::random(0, border.height()) - border.top);

		new(boids + i) Boid(&grid, boids, pos);
	}

	state.update(boids, 1.0f);

	glClearColor(
		Config::background.x, 
		Config::background.y, 
		Config::background.z, 1.0f);

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

		{
			Config::volume = 0.0f;

			if (pMeterInfo)
			{
				if (SUCCEEDED(pMeterInfo->GetPeakValue(&Config::volume)))
					Config::volume = -20 * std::log10f(1.0f - Config::volume);
			}

			for (int i = 0; i < Config::audio_responsive_processes.size(); ++i)
			{
				std::wstring process_name = Config::audio_responsive_processes[i];

				if (processes_session_control.contains(process_name))
				{
					IAudioSessionControl2* sessionControl2 = processes_session_control[process_name].first;
					IAudioMeterInformation* meterInformation = processes_session_control[process_name].second;

					AudioSessionState state;
					if (sessionControl2 == NULL || meterInformation == NULL || FAILED(sessionControl2->GetState(&state)) || state == AudioSessionStateExpired)
					{
						SAFE_RELEASE(processes_session_control[process_name].first);
						SAFE_RELEASE(processes_session_control[process_name].second);
						processes_session_control.erase(process_name);
						continue;
					}

					float temp = 0.0f;
					if (SUCCEEDED(meterInformation->GetPeakValue(&temp)) && temp > Config::volume)
						Config::volume = -20 * std::log10f(1.0f - temp);
				}
				else
				{
					refresh_freq -= rDeltaTime;

					if (refresh_freq <= 0.0f)
					{
						refresh(&process_name);
						refresh_freq = refresh_freq_max;
					}
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
					if (Config::gravity_enabled)
					{
						if (inputHandler.get_left_held())
							boid.steer_towards(mouse_pos, Config::gravity_towards_factor);
						if (inputHandler.get_right_held())
							boid.steer_towards(mouse_pos, -Config::gravity_away_factor);
					}

					if (Config::predator_enabled && !(Config::gravity_enabled && (inputHandler.get_left_held() || inputHandler.get_right_held())))
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

		glPushMatrix();

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.get_world_matrix());

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		glPopMatrix();

		window.display();
	}

	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pMeterInfo);
	SAFE_RELEASE(pSessionManager);

	for (int i = 0; i < Config::audio_responsive_processes.size(); ++i)
	{
		std::wstring process_name = Config::audio_responsive_processes[i];

		SAFE_RELEASE(processes_session_control[process_name].first);
		SAFE_RELEASE(processes_session_control[process_name].second);
	}

	CoUninitialize();

	return 0;
}