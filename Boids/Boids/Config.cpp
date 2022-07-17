#include "Config.h"

sf::Vector3f Config::background				= sf::Vector3f(0.0f, 0.0f, 0.0f);

int Config::boid_count						= 2600;

float Config::boid_size_width				= 17.0f;
float Config::boid_size_height				= 8.5f;
float Config::boid_max_speed				= 340.0f;
float Config::boid_min_speed				= 100.0f;
float Config::boid_max_steer				= 3.0f;
float Config::boid_view_angle				= 280.0f;
float Config::boid_min_distance				= 54;

sf::Vector3f Config::boid_color_top_left	= sf::Vector3f(0.73f, 0.33f, 0.82f);
sf::Vector3f Config::boid_color_top_right	= sf::Vector3f(1.0f, 0.0f, 1.0f);
sf::Vector3f Config::boid_color_bot_left	= sf::Vector3f(0.85f, 0.75f, 0.85f);
sf::Vector3f Config::boid_color_bot_right	= sf::Vector3f(0.35f, 0.0f, 0.35f);

float Config::weight_sep					= 2.5f;
float Config::weight_ali					= 1.2f;
float Config::weight_coh					= 1.65f;

bool Config::gravity_enabled				= true;
float Config::gravity_towards_factor		= 0.8f;
float Config::gravity_away_factor			= 0.8f;

bool Config::predator_enabled				= true;
float Config::predator_distance				= 200.0f;
float Config::predator_factor				= 0.3f;

bool Config::turn_at_border					= false;
float Config::turn_margin_factor			= 0.85f;
float Config::turn_factor					= 160.0f;

int Config::grid_extra_cells				= 12;

bool Config::camera_enabled					= false;
bool Config::vertical_sync					= true;
int Config::max_framerate					= 144;