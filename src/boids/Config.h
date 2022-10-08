#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

#include "../utilities/Utilities.h"

static const std::string FILE_NAME = "config";

static const std::string BACKGROUND = "background";
static const std::string BOID = "boid";
static const std::string RULES = "rules";
static const std::string COLOR = "color";
static const std::string MISC = "misc";

enum ColorFlags
{
	CF_None			= 0,
	CF_Positional	= 1 << 0,
	CF_Cycle		= 1 << 1,
	CF_Density		= 1 << 2,
	CF_Velocity		= 1 << 3,
	CF_Rotation		= 1 << 4,
	CF_Audio		= 1 << 5,
	CF_Fluid		= 1 << 6,
};

inline ColorFlags operator|(ColorFlags a, ColorFlags b)
{
	return static_cast<ColorFlags>(static_cast<int>(a) | static_cast<int>(b));
}
inline ColorFlags operator|=(ColorFlags& a, ColorFlags b)
{
	return a = a | b;
}

enum Rebuild : int
{
	RB_Grid,
	RB_Boids,
	RB_BoidsCycle,
	RB_BackgroundTex,
	RB_BackgroundProp,
	RB_Audio,
	RB_Window,
	RB_Camera,
	RB_Fluid,
	
	RB_Count
};

struct Config
{
	sf::Vector3f background_color						{0.0f, 0.0f, 0.0f};
	std::string background_texture						{""};
	int background_position_x							{0};
	int background_position_y							{0};
	bool background_fit_screen							{true};
	bool background_override_size						{false};
	int background_width								{0};
	int background_height								{0};

	int boid_count										{2000};
	float boid_size_width								{22.0f};
	float boid_size_height								{11.0f};
	float boid_max_speed								{360.0f};
	float boid_min_speed								{160.0f};
	float boid_max_steer								{4.0f};
	float boid_view_angle								{240.0f};

	float sep_distance									{30.0f};
	float ali_distance									{60.0f};
	float coh_distance									{60.0f};

	float sep_weight									{2.8f};
	float ali_weight									{1.5f};
	float coh_weight									{1.8f};

	ColorFlags color_flags								{CF_Cycle | CF_Density | CF_Velocity | CF_Rotation};

	float color_positional_weight						{1.0f};
	float color_cycle_weight							{0.5f};
	float color_density_weight							{0.9f};
	float color_velocity_weight							{0.5f};
	float color_rotation_weight							{-1.7f};
	float color_audio_weight							{1.0f};
	float color_fluid_weight							{1.0f};

	sf::Vector3f boid_color_top_left					{0.73f, 0.33f, 0.82f};
	sf::Vector3f boid_color_top_right					{1.0f, 0.0f, 1.0f};
	sf::Vector3f boid_color_bot_left					{0.85f, 0.75f, 0.85f};
	sf::Vector3f boid_color_bot_right					{0.35f, 0.0f, 0.35f};

	bool boid_cycle_colors_random						{true};
	float boid_cycle_colors_speed						{1.0f};
	std::vector<sf::Vector3f> boid_cycle_colors			
	{ 
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f) 
	};

	int boid_density									{70};
	bool boid_density_cycle_enabled						{true};
	float boid_density_cycle_speed						{0.75};
	std::vector<sf::Vector3f> boid_density_colors		
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	std::vector<sf::Vector3f> boid_velocity_colors
	{
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(1.0f, 0.1f, 1.0f)
	};

	std::vector<sf::Vector3f> boid_rotation_colors
	{
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.29f, 0.0f, 0.29f),
		sf::Vector3f(0.29f, 0.0f, 0.29f)
	};

	std::vector<std::wstring> audio_responsive_apps		{};
	float audio_responsive_strength						{4.75f};
	float audio_responsive_limit						{1.2f};
	int audio_responsive_density						{25};
	std::vector<sf::Vector3f> audio_responsive_colors
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.8f, 0.1f, 1.0f),
		sf::Vector3f(0.8f, 0.1f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.8f, 0.1f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	bool impulse_enabled								{true};
	float impulse_size									{50.0f};
	float impulse_speed									{500.0f};
	float impulse_fade_distance							{400.0f};
	std::vector<sf::Vector3f> impulse_colors
	{
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	int fluid_scale										{10};
	float fluid_mouse_strength							{0.2f};
	float fluid_color_vel								{0.1f};
	float fluid_diffusion								{0.0f};
	float fluid_viscosity								{0.00001f};
	std::vector<sf::Vector3f> fluid_colors
	{
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.0f, 0.0f, 0.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(1.0f, 0.1f, 1.0f)
	};

	int boid_add_amount									{5};
	float boid_add_mouse_diff							{1.0f};
	int boid_remove_amount								{50};

	bool steer_enabled									{true};
	float steer_towards_factor							{0.9f};
	float steer_away_factor								{0.9f};

	bool predator_enabled								{true};
	float predator_distance								{250.0f};
	float predator_factor								{0.6f};

	bool turn_at_border									{false};
	float turn_margin_factor							{0.85f};
	float turn_factor									{275.0f};

	int grid_extra_cells								{16};
	bool camera_enabled									{false};
	float camera_zoom									{1.0f};
	bool vertical_sync									{true};
	int max_framerate									{200};
	float physics_update_freq							{60.0f};
	int policy_threshold								{1500};

	bool debug_enabled									{false};
	float debug_update_freq								{0.5f};
	int debug_toggle_key								{85};

	// misc

	bool load_status{false};

public:
	Config();

	void load();
	std::vector<Rebuild> refresh(Config& prev);

private:
	void load_var(nlohmann::json& json);

	sf::Vector3f str_to_color(std::string str) const;
	void convert_to_color(std::vector<sf::Vector3f>& dest, const std::vector<std::string>& src);
};
