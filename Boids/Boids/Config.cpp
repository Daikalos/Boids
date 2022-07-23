#include "Config.h"

sf::Vector3f Config::background				= sf::Vector3f(0.0f, 0.0f, 0.0f);

int Config::boid_count						= 1750;

float Config::boid_size_width				= 22.0f;
float Config::boid_size_height				= 11.0f;
float Config::boid_max_speed				= 330.0f;
float Config::boid_min_speed				= 150.0f;
float Config::boid_max_steer				= 3.0f;
float Config::boid_view_angle				= 240.0f;

float Config::sep_distance					= 30.0f;
float Config::ali_distance					= 60.0f;
float Config::coh_distance					= 60.0f;

float Config::sep_weight					= 2.6f;
float Config::ali_weight					= 1.2f;
float Config::coh_weight					= 1.7f;

int Config::color_option					= 2;

sf::Vector3f Config::boid_color_top_left	= sf::Vector3f(0.73f, 0.33f, 0.82f);
sf::Vector3f Config::boid_color_top_right	= sf::Vector3f(1.0f, 0.0f, 1.0f);
sf::Vector3f Config::boid_color_bot_left	= sf::Vector3f(0.85f, 0.75f, 0.85f);
sf::Vector3f Config::boid_color_bot_right	= sf::Vector3f(0.35f, 0.0f, 0.35f);

bool Config::boid_cycle_colors_random		= true;
float Config::boid_cycle_colors_speed		= 1.0f;
std::vector<sf::Vector3f> Config::boid_cycle_colors = 
{ 
	sf::Vector3f(0.35f, 0.0f, 0.35f), 
	sf::Vector3f(1.0f, 0.1f, 1.0f), 
	sf::Vector3f(0.6f, 0.0f, 1.0f), 
	sf::Vector3f(0.35f, 0.0f, 0.35f)
};

int Config::boid_density					= 25;
bool Config::boid_density_cycle_enabled		= true;
float Config::boid_density_cycle_speed		= 1.0f;
std::vector<sf::Vector3f> Config::boid_density_colors =
{
	sf::Vector3f(1.0f, 0.1f, 1.0f),
	sf::Vector3f(0.6f, 0.0f, 1.0f),
	sf::Vector3f(0.35f, 0.0f, 0.35f),
	sf::Vector3f(0.35f, 0.0f, 0.35f),
	sf::Vector3f(0.35f, 0.0f, 0.35f),
	sf::Vector3f(0.35f, 0.0f, 0.35f),
	sf::Vector3f(0.35f, 0.0f, 0.35f)
};

bool Config::impulse_enabled				= true;
float Config::impulse_size					= 50.0f;
float Config::impulse_speed					= 600.0f;
float Config::impulse_fade_distance			= 750.0f;
std::vector<sf::Vector3f> Config::impulse_colors =
{
	sf::Vector3f(1.0f, 0.0f, 1.0),
	sf::Vector3f(0.6f, 0.0f, 1.0f),
	sf::Vector3f(0.35f, 0.0f, 0.35f),
};

bool Config::gravity_enabled				= true;
float Config::gravity_towards_factor		= 0.8f;
float Config::gravity_away_factor			= 0.8f;

bool Config::predator_enabled				= true;
float Config::predator_distance				= 250.0f;
float Config::predator_factor				= 0.4f;

bool Config::turn_at_border					= false;
float Config::turn_margin_factor			= 0.85f;
float Config::turn_factor					= 50.0f;

int Config::grid_extra_cells				= 8;
bool Config::camera_enabled					= false;
float Config::camera_zoom					= 1.0f;
bool Config::vertical_sync					= true;
int Config::max_framerate					= 144;

std::vector<Impulse> Config::impulses = {};