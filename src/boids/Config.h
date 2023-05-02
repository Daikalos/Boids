#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

#include "../utilities/Utilities.h"

static const std::string FILE_NAME	= "Config";

static const std::string BACKGROUND = "Background";
static const std::string BOID		= "Boid";
static const std::string RULES		= "Rules";
static const std::string COLOR		= "Color";
static const std::string MISC		= "Misc";

enum ColorFlags : std::uint32_t
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

using ColorCont = std::vector<sf::Vector3f>;
using WStringCont = std::vector<std::wstring>;

class Config
{
public:
	sf::Vector3f	BackgroundColor				{0.0f, 0.0f, 0.0f};
	std::string		BackgroundTexture			{""};
	int				BackgroundPositionX			{0};
	int				BackgroundPositionY			{0};
	bool			BackgroundFitScreen			{true};
	bool			BackgroundOverrideSize		{false};
	int				BackgroundWidth				{0};
	int				BackgroundHeight			{0};

	int				BoidCount					{2000};
	float			BoidWidth					{22.0f};
	float			BoidHeight					{11.0f};
	float			BoidSpeedMax				{360.0f};
	float			BoidSpeedMin				{160.0f};
	float			BoidSteerMax				{4.0f};
	float			BoidViewAngle				{240.0f};

	float			SepDistance					{30.0f};
	float			AliDistance					{60.0f};
	float			CohDistance					{60.0f};

	float			SepWeight					{2.8f};
	float			AliWeight					{1.5f};
	float			CohWeight					{1.8f};

	uint32_t		ColorFlag					{CF_Cycle | CF_Density | CF_Velocity | CF_Rotation};

	float			ColorPositionalWeight		{1.0f};
	float			ColorCycleWeight			{0.5f};
	float			ColorDensityWeight			{0.9f};
	float			ColorVelocityWeight			{0.5f};
	float			ColorRotationWeight			{-1.7f};
	float			ColorAudioWeight			{1.0f};
	float			ColorFluidWeight			{1.0f};

	sf::Vector3f	BoidColorTopLeft			{0.73f, 0.33f, 0.82f};
	sf::Vector3f	BoidColorTopRight			{1.0f, 0.0f, 1.0f};
	sf::Vector3f	BoidColorBotLeft			{0.85f, 0.75f, 0.85f};
	sf::Vector3f	BoidColorBotRight			{0.35f, 0.0f, 0.35f};

	bool			BoidCycleColorsRandom		{true};
	float			BoidCycleColorsSpeed		{1.0f};
	ColorCont		BoidCycleColors
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	int				BoidDensity					{70};
	bool			BoidDensityCycleEnabled		{true};
	float			BoidDensityCycleSpeed		{0.75};
	ColorCont		BoidDensityColors
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

	ColorCont		BoidVelocityColors
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

	ColorCont		BoidRotationColors
	{
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.29f, 0.0f, 0.29f),
		sf::Vector3f(0.29f, 0.0f, 0.29f)
	};

	WStringCont		AudioResponsiveApps			{};
	float			AudioResponsiveStrength		{4.75f};
	float			AudioResponsiveLimit		{1.2f};
	int				AudioResponsiveDensity		{25};
	ColorCont		AudioResponsiveColors
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

	bool			ImpulseEnabled				{true};
	float			ImpulseSize					{50.0f};
	float			ImpulseSpeed				{500.0f};
	float			ImpulseFadeDistance			{400.0f};
	ColorCont		ImpulseColors
	{
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	int				FluidScale					{10};
	float			FluidMouseStrength			{0.2f};
	float			FluidColorVel				{0.1f};
	float			FluidDiffusion				{0.0f};
	float			FluidViscosity				{0.00001f};
	ColorCont		FluidColors
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

	int				BoidAddAmount				{5};
	float			BoidAddMouseDiff			{1.0f};
	int				BoidRemoveAmount			{50};

	bool			SteerEnabled				{true};
	float			SteerTowardsFactor			{0.9f};
	float			SteerAwayFactor				{0.9f};

	bool			PredatorEnabled				{true};
	float			PredatorDistance			{250.0f};
	float			PredatorFactor				{0.6f};

	bool			TurnAtBorder				{false};
	float			TurnMarginFactor			{0.85f};
	float			TurnFactor					{275.0f};

	int				GridExtraCells				{16};
	bool			CameraEnabled				{false};
	float			CameraZoom					{1.0f};
	bool			VerticalSync				{true};
	int				MaxFramerate				{200};
	float			PhysicsUpdateFreq			{60.0f};
	int				PolicyThreshold				{1500};

	bool			DebugEnabled				{false};
	float			DebugUpdateFreq				{0.5f};
	int				DebugToggleKey				{85};

	// Misc

	bool			LoadStatus					{false};

public:
	static Config& Inst() 
	{
		// switched to using singleton because of application scale and in this context, 
		// will never go beyond one allocation

		static Config instance;
		return instance;
	}

public:
	Config();

	void load();
	std::vector<Rebuild> refresh(Config& prev);

private:
	void load_var(nlohmann::json& json);

	sf::Vector3f str_to_color(std::string str) const;
	void convert_to_color(std::vector<sf::Vector3f>& dest, const std::vector<std::string>& src);
};
