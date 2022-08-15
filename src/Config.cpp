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

	color_flags = ColorFlags::Cycle;

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

	boid_view_angle = util::to_radians(boid_view_angle) / 2.0f;

	sep_distance *= sep_distance;
	ali_distance *= ali_distance;
	coh_distance *= coh_distance;

	predator_distance *= predator_distance;
}

void Config::load()
{
	std::ifstream project_file(FILE_NAME + ".json", std::ifstream::binary);
	if (project_file.good())
	{
		try
		{
			load_status = false;
			Config temp = *this;

			nlohmann::json json = nlohmann::json::parse(project_file);
			temp.load_var(json);

			*this = temp;
			load_status = true;

			boid_view_angle = util::to_radians(boid_view_angle) / 2.0f;

			sep_distance *= sep_distance;
			ali_distance *= ali_distance;
			coh_distance *= coh_distance;

			predator_distance *= predator_distance;
		}
		catch (nlohmann::json::parse_error) { }
		catch (nlohmann::detail::type_error e) { }
	}
}

std::vector<Reconstruct> Config::refresh(Config& prev)
{
	std::vector<Reconstruct> result;
	result.reserve((int)Reconstruct::RCount);

	load();

	if (prev.sep_distance != sep_distance || prev.ali_distance != ali_distance || prev.coh_distance != coh_distance || prev.turn_at_border != turn_at_border)
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
	auto config = json[FILE_NAME];

	auto background = config[BACKGROUND];
	auto boid = config[BOID];
	auto rules = config[RULES];
	auto color = config[COLOR];
	auto misc = config[MISC];

	background_color			= str_to_color(background["background_color"]);
	background_texture			= background["background_texture"];
	background_position_x		= background["background_position_x"];
	background_position_y		= background["background_position_y"];
	background_fit_screen		= background["background_fit_screen"];
	background_override_size	= background["background_override_size"];
	background_width			= background["background_width"];
	background_height			= background["background_height"];

	boid_count					= boid["boid_count"];
	boid_size_width				= boid["boid_size_width"];
	boid_size_height			= boid["boid_size_height"];
	boid_max_speed				= boid["boid_max_speed"];
	boid_min_speed				= boid["boid_min_speed"];
	boid_max_steer				= boid["boid_max_steer"];
	boid_view_angle				= boid["boid_view_angle"];

	sep_distance				= rules["sep_distance"];
	ali_distance				= rules["ali_distance"];
	coh_distance				= rules["coh_distance"];

	sep_weight					= rules["sep_weight"];
	ali_weight					= rules["ali_weight"];
	coh_weight					= rules["coh_weight"];

	std::vector<int> temp_color_options = color["color_options"];
	color_flags = (temp_color_options.size()) ? ColorFlags::None : ColorFlags::Cycle;
	for (int i = 0; i < temp_color_options.size(); ++i)
		color_flags |= static_cast<ColorFlags>((int)std::powf(2, temp_color_options[i] - 1));

	color_positional_weight		= color["color_positional_weight"];
	color_cycle_weight			= color["color_cycle_weight"];
	color_density_weight		= color["color_density_weight"];
	color_velocity_weight		= color["color_velocity_weight"];
	color_rotation_weight		= color["color_rotation_weight"];
	color_audio_weight			= color["color_audio_weight"];

	if (color_flags & ColorFlags::Positional)
	{
		boid_color_top_left		= str_to_color(color["boid_color_top_left"]);
		boid_color_top_right	= str_to_color(color["boid_color_top_right"]);
		boid_color_bot_left		= str_to_color(color["boid_color_bot_left"]);
		boid_color_bot_right	= str_to_color(color["boid_color_bot_right"]);
	}
	if (color_flags & ColorFlags::Cycle)
	{
		boid_cycle_colors_random	= color["boid_cycle_colors_random"];
		boid_cycle_colors_speed		= color["boid_cycle_colors_speed"];

		convert_to_color(boid_cycle_colors, color["boid_cycle_colors"]);
	}
	if (color_flags & ColorFlags::Density)
	{
		boid_density				= color["boid_density"];
		boid_density_cycle_enabled	= color["boid_density_cycle_enabled"];
		boid_density_cycle_speed	= color["boid_density_cycle_speed"];

		convert_to_color(boid_density_colors, color["boid_density_colors"]);
	}
	if (color_flags & ColorFlags::Velocity)
	{
		convert_to_color(boid_velocity_colors, color["boid_velocity_colors"]);
	}
	if (color_flags & ColorFlags::Rotation)
	{
		convert_to_color(boid_rotation_colors, color["boid_rotation_colors"]);
	}
	if (color_flags & ColorFlags::Audio)
	{
		std::vector<std::string> temp_processes = color["audio_responsive_apps"];
		audio_responsive_apps = std::vector<std::wstring>(temp_processes.size());

		for (int i = 0; i < audio_responsive_apps.size(); ++i)
		{
			std::string process = temp_processes[i];
			audio_responsive_apps[i] = std::wstring(process.begin(), process.end());
		}

		audio_responsive_strength	= color["audio_responsive_strength"];
		audio_responsive_limit		= color["audio_responsive_limit"];
		audio_responsive_density	= color["audio_responsive_density"];

		convert_to_color(audio_responsive_colors, color["audio_responsive_colors"]);
	}

	impulse_enabled				= color["impulse_enabled"];
	impulse_size				= color["impulse_size"];
	impulse_speed				= color["impulse_speed"];
	impulse_fade_distance		= color["impulse_fade_distance"];

	convert_to_color(impulse_colors, color["impulse_colors"]);

	steer_enabled				= misc["steer_enabled"];
	steer_towards_factor		= misc["steer_towards_factor"];
	steer_away_factor			= misc["steer_away_factor"];

	predator_enabled			= misc["predator_enabled"];
	predator_distance			= misc["predator_distance"];
	predator_factor				= misc["predator_factor"];

	turn_at_border				= misc["turn_at_border"];
	turn_margin_factor			= misc["turn_margin_factor"];
	turn_factor					= misc["turn_factor"];

	grid_extra_cells			= misc["grid_extra_cells"];
	camera_enabled				= misc["camera_enabled"];
	camera_zoom					= misc["camera_zoom"];
	vertical_sync				= misc["vertical_sync"];
	max_framerate				= misc["max_framerate"];
	physics_update_freq			= misc["physics_update_freq"];

	debug_enabled				= misc["debug_enabled"];
	debug_update_freq			= misc["debug_update_freq"];
	debug_toggle_key			= misc["debug_toggle_key"];
}

sf::Vector3f Config::str_to_color(std::string str) const
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

void Config::convert_to_color(std::vector<sf::Vector3f>& dest, const std::vector<std::string>& src)
{
	dest = std::vector<sf::Vector3f>(src.size());

	for (int i = 0; i < src.size(); ++i)
		dest[i] = std::move(str_to_color(src[i]));
}
