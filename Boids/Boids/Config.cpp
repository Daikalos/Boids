#include "Config.h"

sf::Vector3f Config::background;

size_t Config::boid_count;

float Config::boid_size_width;
float Config::boid_size_height;
float Config::boid_max_speed;
float Config::boid_max_steer;
float Config::boid_view_angle;
float Config::boid_min_distance;

sf::Vector3f Config::boid_color_top_left;
sf::Vector3f Config::boid_color_top_right;
sf::Vector3f Config::boid_color_bot_left;
sf::Vector3f Config::boid_color_bot_right;

float Config::weight_sep;
float Config::weight_ali;
float Config::weight_coh;

bool Config::cursor_enabled;
float Config::cursor_towards;
float Config::cursor_away;

bool Config::turn_at_border;
float Config::turn_margin_factor;
float Config::turn_factor;

int Config::grid_cell_max_boids;
int Config::grid_extra_cells;

bool Config::vertical_sync;
int Config::max_framerate;