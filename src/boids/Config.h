#pragma once

#include <vector>
#include <string>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

inline constexpr const char* FILE_NAME	= "Config.json";

inline constexpr const char* CONFIG			= "Config";
inline constexpr const char* BACKGROUND		= "Background";
inline constexpr const char* BOIDS			= "Boids";
inline constexpr const char* RULES			= "Rules";
inline constexpr const char* INTERACTION	= "Interaction";
inline constexpr const char* COLOR			= "Color";
inline constexpr const char* MISC			= "Misc";

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

enum class Rebuild
{
	None = -1,
	Grid,
	Boids,
	BoidsCycle,
	BackgroundTex,
	BackgroundProp,
	Audio,
	Window,
	Camera,
	Fluid,
	Count
};

using ColorCont = std::vector<sf::Vector3f>;
using WStringCont = std::vector<std::wstring>;

struct BackgroundConfig
{
	std::string		Texture			{""};
	sf::Vector3f	Color			{0.0f, 0.0f, 0.0f};
	int				PositionX		{0};
	int				PositionY		{0};
	int				Width			{0};
	int				Height			{0};
	bool			FitScreen		{true};
	bool			OverrideSize	{false};
	bool			UseWallpaper	{false};
};

struct BoidsConfig
{
	std::size_t		Count						{2000};
	float			Width						{22.0f};
	float			Height						{11.0f};
	float			SpeedMax					{360.0f};
	float			SpeedMin					{160.0f};
	float			SteerMax					{4.0f};
	float			ViewAngle					{240.0f};
};

struct RulesConfig
{
	float			SepDistance					{30.0f};
	float			AliDistance					{60.0f};
	float			CohDistance					{60.0f};
	float			SepWeight					{2.8f};
	float			AliWeight					{1.5f};
	float			CohWeight					{1.8f};
};

struct InteractionConfig
{
	int				BoidAddAmount				{5};
	float			BoidAddMouseDiff			{1.0f};
	int				BoidRemoveAmount			{50};

	float			SteerTowardsFactor			{0.9f};
	float			SteerAwayFactor				{0.9f};

	float			PredatorDistance			{250.0f};
	float			PredatorFactor				{0.6f};

	float			TurnMarginFactor			{0.85f};
	float			TurnFactor					{275.0f};

	bool			SteerEnabled				{true};
	bool			PredatorEnabled				{true};
	bool			TurnAtBorder				{false};
};

struct ColorConfig
{
	uint32_t		Flags				{CF_Cycle | CF_Density | CF_Velocity | CF_Rotation};
	float			PositionalWeight	{1.0f};
	float			CycleWeight			{0.5f};
	float			DensityWeight		{0.9f};
	float			VelocityWeight		{0.5f};
	float			RotationWeight		{-1.7f};
	float			AudioWeight			{1.0f};
	float			FluidWeight			{1.0f};
};

struct ColorPosConfig
{
	sf::Vector3f	TopLeft				{0.73f, 0.33f, 0.82f};
	sf::Vector3f	TopRight			{1.0f, 0.0f, 1.0f};
	sf::Vector3f	BotLeft				{0.85f, 0.75f, 0.85f};
	sf::Vector3f	BotRight			{0.35f, 0.0f, 0.35f};
};

struct ColorCycleConfig
{
	ColorCont		Colors
	{
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(1.0f, 0.1f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};
	float			Speed				{1.0f};
	bool			Random				{true};
};

struct ColorDensityConfig
{
	ColorCont		Colors
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

	int				Density				{70};
	float			DensityCycleSpeed	{0.75};
	bool			DensityCycleEnabled	{true};
};

struct ColorVelocityConfig
{
	ColorCont		Colors
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
};

struct ColorRotationConfig
{
	ColorCont		Colors
	{
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.18f, 0.0f, 0.18f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.35f, 0.0f, 0.35f),
		sf::Vector3f(0.29f, 0.0f, 0.29f),
		sf::Vector3f(0.29f, 0.0f, 0.29f)
	};
};

struct ColorAudioConfig
{
	WStringCont		Apps			{};
	ColorCont		Colors
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

	float			Strength			{4.75f};
	float			Limit				{1.2f};
	float			Speed				{50.0f};
	int				Density				{25};
};

struct ColorImpulseConfig
{
	ColorCont		Colors
	{
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(1.0f, 0.15f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.6f, 0.0f, 1.0f),
		sf::Vector3f(0.35f, 0.0f, 0.35f)
	};

	float			Size				{50.0f};
	float			Speed				{500.0f};
	float			FadeDistance		{400.0f};
	float			Force				{256.0f};
	bool			Enabled				{true};
};

struct ColorFluidConfig
{
	ColorCont		Colors
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

	int				Scale				{10};
	float			MouseStrength		{0.2f};
	float			ColorVel			{0.1f};
	float			Diffusion			{0.0f};
	float			Viscosity			{0.00001f};
};

struct MiscConfig
{
	int				GridExtraCells				{16};
	float			CameraZoom					{1.0f};
	int				MaxFramerate				{200};
	float			PhysicsUpdateFreq			{60.0f};
	std::size_t		PolicyThreshold				{1500};

	float			DebugUpdateFreq				{0.5f};
	int				DebugToggleKey				{85};

	bool			CameraEnabled				{false};
	bool			VerticalSync				{true};
	bool			DebugEnabled				{false};
};

class Config
{
public:
	BackgroundConfig	Background;
	BoidsConfig			Boids;
	RulesConfig			Rules;
	InteractionConfig	Interaction;
	ColorConfig			Color;
	ColorPosConfig		Positional;
	ColorCycleConfig	Cycle;
	ColorDensityConfig	Density;
	ColorVelocityConfig Velocity;
	ColorRotationConfig Rotation;
	ColorAudioConfig	Audio;
	ColorImpulseConfig	Impulse;
	ColorFluidConfig	Fluid;
	MiscConfig			Misc;

	// Misc

	sf::Vector2f	BoidHalfSize;
	float			BoidSpeedInv				{0.0f};
	float			BoidSpeedMinSq				{0.0f};
	float			BoidSpeedMaxSq				{0.0f};
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

	void Load();

	std::vector<Rebuild> Refresh(Config& prev);

private:
	void UpdateMisc();
};


