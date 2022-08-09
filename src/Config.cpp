#include "Config.h"

Config::Config()
{
	background_color = sf::Vector3f(0.0f, 0.0f, 0.0f);
	background_texture = "";
	background_position_x = 0;
	background_position_y = 0;
	background_fit_screen = true;
	background_override_size = false;
	background_width = 0;
	background_height = 0;

	boid_count = 2000;

	boid_size_width = 22.0f;
	boid_size_height = 11.0f;
	boid_max_speed = 360.0f;
	boid_min_speed = 160.0f;
	boid_max_steer = 4.0f;
	boid_view_angle = 240.0f;

	sep_distance = 30.0f;
	ali_distance = 60.0f;
	coh_distance = 60.0f;

	sep_weight = 3.0f;
	ali_weight = 1.6f;
	coh_weight = 1.9f;

	color_option = 2;

	boid_color_top_left = sf::Vector3f(0.73f, 0.33f, 0.82f);
	boid_color_top_right = sf::Vector3f(1.0f, 0.0f, 1.0f);
	boid_color_bot_left = sf::Vector3f(0.85f, 0.75f, 0.85f);
	boid_color_bot_right = sf::Vector3f(0.35f, 0.0f, 0.35f);

	boid_cycle_colors_random = true;
	boid_cycle_colors_speed = 1.0f;
	boid_cycle_colors =
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	boid_density = 25;
	boid_density_cycle_enabled = true;
	boid_density_cycle_speed = 1.0f;
	boid_density_colors =
	{
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	std::vector<std::wstring> audio_responsive_apps = {};
	audio_responsive_strength = 0.75f;
	audio_responsive_limit = 1.2f;
	audio_responsive_density = 25;
	audio_responsive_colors =
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.78f, 0.05f, 1.0f),
		sf::Vector3f(0.78f, 0.05f, 1.0f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
	};

	impulse_enabled = true;
	impulse_size = 50.0f;
	impulse_speed = 600.0f;
	impulse_fade_distance = 800.0f;
	impulse_colors =
	{
		sf::Vector3f(1.0f, 0.0f, 1.0f),
		sf::Vector3f(1.0f, 0.0f, 1.0f),
		sf::Vector3f(1.0f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
	};

	steer_enabled = true;
	steer_towards_factor = 0.9f;
	steer_away_factor = 0.9f;

	predator_enabled = true;
	predator_distance = 250.0f;
	predator_factor = 0.6f;

	turn_at_border = false;
	turn_margin_factor = 0.85f;
	turn_factor = 275.0f;

	grid_extra_cells = 16;
	camera_enabled = false;
	camera_zoom = 1.0f;
	vertical_sync = true;
	max_framerate = 160;
	physics_update_freq = 90.0f;

	debug_enabled = false;
	debug_update_freq = 1.0f;
	debug_toggle_key = 89;

	sep_distance *= sep_distance;
	ali_distance *= ali_distance;
	coh_distance *= coh_distance;

	predator_distance *= predator_distance;
}

Config::~Config()
{

}

void Config::load()
{
	std::ifstream project_file(FILE_NAME + ".json", std::ifstream::binary);
	if (project_file.good())
	{
		try
		{
			nlohmann::json json = nlohmann::json::parse(project_file);
			load_var(json);

			sep_distance *= sep_distance;
			ali_distance *= ali_distance;
			coh_distance *= coh_distance;

			predator_distance *= predator_distance;
		}
		catch (nlohmann::json::parse_error) {}
		catch (nlohmann::detail::type_error e) {}
	}
}

std::vector<Reconstruct> Config::refresh(Config& prev)
{
	std::vector<Reconstruct> result;
	result.reserve(8);

	load();

	if (prev.sep_distance != sep_distance || prev.ali_distance != ali_distance || prev.coh_distance != coh_distance)
		result.push_back(Reconstruct::RGrid);
	if (prev.boid_count != boid_count)
		result.push_back(Reconstruct::RBoids);
	if (prev.boid_cycle_colors_random != boid_cycle_colors_random)
		result.push_back(Reconstruct::RBoidsCycle);
	if (prev.background_texture != background_texture)
		result.push_back(Reconstruct::RBackgroundTex);
	if (prev.vertical_sync != vertical_sync || prev.max_framerate != max_framerate)
		result.push_back(Reconstruct::RWindow);
	if (!std::ranges::equal(prev.audio_responsive_apps, audio_responsive_apps))
		result.push_back(Reconstruct::RAudio);
	if (prev.camera_zoom != camera_zoom)
		result.push_back(Reconstruct::RCamera);
	if (prev.physics_update_freq != physics_update_freq)
		result.push_back(Reconstruct::RPhysics);
	if (prev.debug_update_freq != debug_update_freq)
		result.push_back(Reconstruct::RDebug);

	if (prev.background_color != background_color ||
		prev.background_position_x != background_position_x ||
		prev.background_position_y != background_position_y ||
		prev.background_fit_screen != background_fit_screen ||
		prev.background_override_size != background_override_size ||
		prev.background_width != background_width ||
		prev.background_height != background_height)
	{
		result.push_back(Reconstruct::RBackgroundProp);
	}

	return result;
}

void Config::load_var(nlohmann::json& json)
{
	nlohmann::basic_json<>::value_type config = json[FILE_NAME];

	background_color = convert(config["background_color"]);
	background_texture = config["background_texture"];
	background_position_x = config["background_position_x"];
	background_position_y = config["background_position_y"];
	background_fit_screen = config["background_fit_screen"];
	background_override_size = config["background_override_size"];
	background_width = config["background_width"];
	background_height = config["background_height"];

	boid_count = config["boid_count"];
	boid_size_width = config["boid_size_width"];
	boid_size_height = config["boid_size_height"];
	boid_max_speed = config["boid_max_speed"];
	boid_min_speed = config["boid_min_speed"];
	boid_max_steer = config["boid_max_steer"];
	boid_view_angle = config["boid_view_angle"];

	sep_distance = config["sep_distance"];
	ali_distance = config["ali_distance"];
	coh_distance = config["coh_distance"];

	sep_weight = config["sep_weight"];
	ali_weight = config["ali_weight"];
	coh_weight = config["coh_weight"];

	color_option = config["color_option"];

	switch (color_option)
	{
	case 0:
	{
		boid_color_top_left = convert(config["boid_color_top_left"]);
		boid_color_top_right = convert(config["boid_color_top_right"]);
		boid_color_bot_left = convert(config["boid_color_bot_left"]);
		boid_color_bot_right = convert(config["boid_color_bot_right"]);
	}
	break;
	case 2:
	{
		boid_density = config["boid_density"];
		boid_density_cycle_enabled = config["boid_density_cycle_enabled"];
		boid_density_cycle_speed = config["boid_density_cycle_speed"];

		std::vector<std::string> temp_colors = config["boid_density_colors"];
		boid_density_colors = std::vector<sf::Vector3f>(temp_colors.size());

		for (int i = 0; i < temp_colors.size(); ++i)
			boid_density_colors[i] = convert(temp_colors[i]);
	}
	break;
	case 3:
	{
		std::vector<std::string> temp_processes = config["audio_responsive_apps"];
		audio_responsive_apps = std::vector<std::wstring>(temp_processes.size());

		for (int i = 0; i < audio_responsive_apps.size(); ++i)
		{
			std::string process = temp_processes[i];
			audio_responsive_apps[i] = std::wstring(process.begin(), process.end());
		}

		audio_responsive_strength = config["audio_responsive_strength"];
		audio_responsive_limit = config["audio_responsive_limit"];
		audio_responsive_density = config["audio_responsive_density"];

		std::vector<std::string> temp_colors = config["audio_responsive_colors"];
		audio_responsive_colors = std::vector<sf::Vector3f>(temp_colors.size());

		for (int i = 0; i < temp_colors.size(); ++i)
			audio_responsive_colors[i] = convert(temp_colors[i]);
	}
	break;
	default:
	{
		boid_cycle_colors_random = config["boid_cycle_colors_random"];
		boid_cycle_colors_speed = config["boid_cycle_colors_speed"];

		std::vector<std::string> temp_colors = config["boid_cycle_colors"];
		boid_cycle_colors = std::vector<sf::Vector3f>(temp_colors.size());

		for (int i = 0; i < temp_colors.size(); ++i)
			boid_cycle_colors[i] = convert(temp_colors[i]);
	}
	break;
	}

	impulse_enabled = config["impulse_enabled"];
	impulse_size = config["impulse_size"];
	impulse_speed = config["impulse_speed"];
	impulse_fade_distance = config["impulse_fade_distance"];

	{
		std::vector<std::string> temp_colors = config["impulse_colors"];
		impulse_colors = std::vector<sf::Vector3f>(temp_colors.size());

		for (int i = 0; i < temp_colors.size(); ++i)
			impulse_colors[i] = convert(temp_colors[i]);
	}

	steer_enabled = config["steer_enabled"];
	steer_towards_factor = config["steer_towards_factor"];
	steer_away_factor = config["steer_away_factor"];

	predator_enabled = config["predator_enabled"];
	predator_distance = config["predator_distance"];
	predator_factor = config["predator_factor"];

	turn_at_border = config["turn_at_border"];
	turn_margin_factor = config["turn_margin_factor"];
	turn_factor = config["turn_factor"];

	grid_extra_cells = config["grid_extra_cells"];
	camera_enabled = config["camera_enabled"];
	camera_zoom = config["camera_zoom"];
	vertical_sync = config["vertical_sync"];
	max_framerate = config["max_framerate"];
	physics_update_freq = config["physics_update_freq"];

	debug_enabled = config["debug_enabled"];
	debug_update_freq = config["debug_update_freq"];
	debug_toggle_key = config["debug_toggle_key"];
}

sf::Vector3f Config::convert(std::string str) const
{
	std::stringstream stream(str);
	std::string segment;
	std::vector<std::string> values;

	while (std::getline(stream, segment, ' '))
		values.push_back(segment);

	sf::Vector3f result(1.0f, 1.0f, 1.0f);

	try
	{
		result.x = std::stof(values[0]) / 255.0f;
		result.y = std::stof(values[1]) / 255.0f;
		result.z = std::stof(values[2]) / 255.0f;
	}
	catch (std::exception e) {}

	return result;
}
