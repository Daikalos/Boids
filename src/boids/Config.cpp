#include "Config.h"

Config::Config()
{
	BoidViewAngle = util::to_radians(BoidViewAngle) / 2.0f;

	SepDistance *= SepDistance;
	AliDistance *= AliDistance;
	CohDistance *= CohDistance;

	PredatorDistance *= PredatorDistance;

	BoidHalfSize = sf::Vector2f(BoidWidth, BoidHeight) / 2.0f;

	BoidSpeedInv = (BoidSpeedMax == BoidSpeedMin) ? 1.0f
		: (1.0f / (BoidSpeedMax - BoidSpeedMin));

	BoidSpeedMinSq = BoidSpeedMin * BoidSpeedMin;
	BoidSpeedMaxSq = BoidSpeedMax * BoidSpeedMax;

	load();
}

void Config::load()
{
	std::ifstream projectFile(FILE_NAME, std::ifstream::binary);
	if (projectFile.good())
	{
		try
		{
			LoadStatus = false;
			Config temp = *this;

			nlohmann::json json = nlohmann::json::parse(projectFile);
			temp.load_var(json);

			*this = temp;
			LoadStatus = true;

			BoidViewAngle = util::to_radians(BoidViewAngle) / 2.0f;

			SepDistance *= SepDistance;
			AliDistance *= AliDistance;
			CohDistance *= CohDistance;

			PredatorDistance *= PredatorDistance;

			BoidHalfSize = sf::Vector2f(BoidWidth, BoidHeight) / 2.0f;

			BoidSpeedInv = (BoidSpeedMax == BoidSpeedMin) ? 1.0f
				: (1.0f / (BoidSpeedMax - BoidSpeedMin));

			BoidSpeedMinSq = BoidSpeedMin * BoidSpeedMin;
			BoidSpeedMaxSq = BoidSpeedMax * BoidSpeedMax;
		}
		catch (nlohmann::json::parse_error) { }
		catch (nlohmann::detail::type_error e) { }
	}
}

std::vector<Rebuild> Config::refresh(Config& prev)
{
	std::vector<Rebuild> result;
	result.reserve((int)RB_Count);

	load();

	if (prev.SepDistance != SepDistance
		|| prev.AliDistance != AliDistance
		|| prev.CohDistance != CohDistance
		|| prev.BoidWidth != BoidWidth
		|| prev.BoidHeight != BoidHeight
		|| prev.TurnAtBorder != TurnAtBorder)
	{
		result.push_back(RB_Grid);
	}

	if (prev.BoidCount != BoidCount)
		result.push_back(RB_Boids);
	if (prev.BoidCycleColorsRandom != BoidCycleColorsRandom)
		result.push_back(RB_BoidsCycle);
	if (prev.BackgroundTexture != BackgroundTexture)
		result.push_back(RB_BackgroundTex);
	if (prev.VerticalSync != VerticalSync || prev.MaxFramerate != MaxFramerate)
		result.push_back(RB_Window);
	if (!std::ranges::equal(prev.AudioResponsiveApps, AudioResponsiveApps))
		result.push_back(RB_Audio);
	if (prev.CameraZoom != CameraZoom)
		result.push_back(RB_Camera);
	if (prev.FluidScale != FluidScale)
		result.push_back(RB_Fluid);

	if (prev.BackgroundColor != BackgroundColor ||
		prev.BackgroundPositionX != BackgroundPositionX ||
		prev.BackgroundPositionY != BackgroundPositionY ||
		prev.BackgroundFitScreen != BackgroundFitScreen ||
		prev.BackgroundOverrideSize != BackgroundOverrideSize ||
		prev.BackgroundWidth != BackgroundWidth ||
		prev.BackgroundHeight != BackgroundHeight)
	{
		result.push_back(RB_BackgroundProp);
	}

	return result;
}

void Config::load_var(nlohmann::json& json)
{
	auto& config = json[CONFIG];

	auto& background	= config[BACKGROUND];
	auto& boid			= config[BOID];
	auto& rules			= config[RULES];
	auto& color			= config[COLOR];
	auto& misc			= config[MISC];

	BackgroundColor				= str_to_color(background["BackgroundColor"]);
	BackgroundTexture			= background["BackgroundTexture"];
	BackgroundPositionX			= background["BackgroundPositionX"];
	BackgroundPositionY			= background["BackgroundPositionY"];
	BackgroundFitScreen			= background["BackgroundFitScreen"];
	BackgroundOverrideSize		= background["BackgroundOverrideSize"];
	BackgroundWidth				= background["BackgroundWidth"];
	BackgroundHeight			= background["BackgroundHeight"];

	BoidCount					= boid["BoidCount"];
	BoidWidth					= boid["BoidWidth"];
	BoidHeight					= boid["BoidHeight"];
	BoidSpeedMax				= boid["BoidSpeedMax"];
	BoidSpeedMin				= boid["BoidSpeedMin"];
	BoidSteerMax				= boid["BoidSteerMax"];
	BoidViewAngle				= boid["BoidViewAngle"];

	SepDistance					= rules["SepDistance"];
	AliDistance					= rules["AliDistance"];
	CohDistance					= rules["CohDistance"];

	SepWeight					= rules["SepWeight"];
	AliWeight					= rules["AliWeight"];
	CohWeight					= rules["CohWeight"];

	std::vector<int> temp_color_options = color["ColorOptions"];
	ColorFlag = !temp_color_options.empty() ? CF_None : CF_Cycle;
	for (int i = 0; i < temp_color_options.size(); ++i)
		ColorFlag |= util::pow(2, temp_color_options[i] - 1);

	ColorPositionalWeight		= color["ColorPositionalWeight"];
	ColorCycleWeight			= color["ColorCycleWeight"];
	ColorDensityWeight			= color["ColorDensityWeight"];
	ColorVelocityWeight			= color["ColorVelocityWeight"];
	ColorRotationWeight			= color["ColorRotationWeight"];
	ColorAudioWeight			= color["ColorAudioWeight"];
	ColorFluidWeight			= color["ColorFluidWeight"];

	if ((ColorFlag & CF_Positional) == CF_Positional)
	{
		BoidColorTopLeft		= str_to_color(color["BoidColorTopLeft"]);
		BoidColorTopRight		= str_to_color(color["BoidColorTopRight"]);
		BoidColorBotLeft		= str_to_color(color["BoidColorBotLeft"]);
		BoidColorBotRight		= str_to_color(color["BoidColorBotRight"]);
	}
	if ((ColorFlag & CF_Cycle) == CF_Cycle)
	{
		BoidCycleColorsRandom	= color["BoidCycleColorsRandom"];
		BoidCycleColorsSpeed	= color["BoidCycleColorsSpeed"];

		convert_to_color(BoidCycleColors, color["BoidCycleColors"]);
	}
	if ((ColorFlag & CF_Density) == CF_Density)
	{
		BoidDensity				= color["BoidDensity"];
		BoidDensityCycleEnabled	= color["BoidDensityCycleEnabled"];
		BoidDensityCycleSpeed	= color["BoidDensityCycleSpeed"];

		convert_to_color(BoidDensityColors, color["BoidDensityColors"]);
	}
	if ((ColorFlag & CF_Velocity) == CF_Velocity)
	{
		convert_to_color(BoidVelocityColors, color["BoidVelocityColors"]);
	}
	if ((ColorFlag & CF_Rotation) == CF_Rotation)
	{
		convert_to_color(BoidRotationColors, color["BoidRotationColors"]);
	}
	if ((ColorFlag & CF_Audio) == CF_Audio)
	{
		std::vector<std::string> temp_processes = color["AudioResponsiveApps"];
		AudioResponsiveApps = std::vector<std::wstring>(temp_processes.size());

		for (int i = 0; i < AudioResponsiveApps.size(); ++i)
		{
			std::string process = temp_processes[i];
			AudioResponsiveApps[i] = std::wstring(process.begin(), process.end());
		}

		AudioResponsiveStrength		= color["AudioResponsiveStrength"];
		AudioResponsiveLimit		= color["AudioResponsiveLimit"];
		AudioResponsiveSpeed		= color["AudioResponsiveSpeed"];
		AudioResponsiveDensity		= color["AudioResponsiveDensity"];

		convert_to_color(AudioResponsiveColors, color["AudioResponsiveColors"]);
	}
	if ((ColorFlag & CF_Fluid) == CF_Fluid)
	{
		FluidScale				= color["FluidScale"];
		FluidMouseStrength		= color["FluidMouseStrength"];
		FluidColorVel			= color["FluidColorVel"];
		FluidDiffusion			= color["FluidDiffusion"];
		FluidViscosity			= color["FluidViscosity"];

		convert_to_color(FluidColors, color["FluidColors"]);
	}

	ImpulseEnabled				= color["ImpulseEnabled"];
	ImpulseSize					= color["ImpulseSize"];
	ImpulseSpeed				= color["ImpulseSpeed"];
	ImpulseFadeDistance			= color["ImpulseFadeDistance"];
	ImpulseForce				= color["ImpulseForce"];

	convert_to_color(ImpulseColors, color["ImpulseColors"]);

	BoidAddAmount				= misc["BoidAddAmount"];
	BoidAddMouseDiff			= misc["BoidAddMouseDiff"];
	BoidRemoveAmount			= misc["BoidRemoveAmount"];

	SteerEnabled				= misc["SteerEnabled"];
	SteerTowardsFactor			= misc["SteerTowardsFactor"];
	SteerAwayFactor				= misc["SteerAwayFactor"];

	PredatorEnabled				= misc["PredatorEnabled"];
	PredatorDistance			= misc["PredatorDistance"];
	PredatorFactor				= misc["PredatorFactor"];

	TurnAtBorder				= misc["TurnAtBorder"];
	TurnMarginFactor			= misc["TurnMarginFactor"];
	TurnFactor					= misc["TurnFactor"];

	GridExtraCells				= misc["GridExtraCells"];
	CameraEnabled				= misc["CameraEnabled"];
	CameraZoom					= misc["CameraZoom"];
	VerticalSync				= misc["VerticalSync"];
	MaxFramerate				= misc["MaxFramerate"];
	PhysicsUpdateFreq			= misc["PhysicsUpdateFreq"];
	PolicyThreshold				= misc["PolicyThreshold"];

	DebugEnabled				= misc["DebugEnabled"];
	DebugUpdateFreq				= misc["DebugUpdateFreq"];
	DebugToggleKey				= misc["DebugToggleKey"];
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
