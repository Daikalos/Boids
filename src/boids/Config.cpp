#include "Config.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <ranges>

#include "../utilities/Utilities.h"

sf::Vector3f StrToColor(std::string str)
{
	std::stringstream stream(str);
	std::string segment;
	std::vector<std::string> values;

	while (std::getline(stream, segment, ' '))
	{
		values.emplace_back(segment);
	}

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

void ConvertToColor(std::vector<sf::Vector3f>& dest, const std::vector<std::string>& src)
{
	dest = std::vector<sf::Vector3f>(src.size());

	for (int i = 0; i < src.size(); ++i)
		dest[i] = StrToColor(src[i]);
}

void ReadJSON(Config& oc, nlohmann::json& json)
{
	auto& ic			= json[CONFIG];

	auto& background	= ic[BACKGROUND];
	auto& boids			= ic[BOIDS];
	auto& rules			= ic[RULES];
	auto& interaction	= ic[INTERACTION];
	auto& color			= ic[COLOR];
	auto& misc			= ic[MISC];

	oc.Background.Color				= StrToColor(background["Color"]);
	oc.Background.Texture			= background["Texture"];
	oc.Background.PositionX			= background["PositionX"];
	oc.Background.PositionY			= background["PositionY"];
	oc.Background.FitScreen			= background["FitScreen"];
	oc.Background.OverrideSize		= background["OverrideSize"];
	oc.Background.UseWallpaper		= background["UseWallpaper"];
	oc.Background.Width				= background["Width"];
	oc.Background.Height			= background["Height"];

	oc.Boids.Count					= boids["Count"];
	oc.Boids.Width					= boids["Width"];
	oc.Boids.Height					= boids["Height"];
	oc.Boids.SpeedMax				= boids["SpeedMax"];
	oc.Boids.SpeedMin				= boids["SpeedMin"];
	oc.Boids.SteerMax				= boids["SteerMax"];
	oc.Boids.ViewAngle				= boids["ViewAngle"];

	oc.Rules.SepDistance			= rules["SepDistance"];
	oc.Rules.AliDistance			= rules["AliDistance"];
	oc.Rules.CohDistance			= rules["CohDistance"];

	oc.Rules.SepWeight				= rules["SepWeight"];
	oc.Rules.AliWeight				= rules["AliWeight"];
	oc.Rules.CohWeight				= rules["CohWeight"];

	oc.Interaction.BoidAddAmount		= interaction["BoidAddAmount"];
	oc.Interaction.BoidAddMouseDiff		= interaction["BoidAddMouseDiff"];
	oc.Interaction.BoidRemoveAmount		= interaction["BoidRemoveAmount"];

	oc.Interaction.SteerEnabled			= interaction["SteerEnabled"];
	oc.Interaction.SteerTowardsFactor	= interaction["SteerTowardsFactor"];
	oc.Interaction.SteerAwayFactor		= interaction["SteerAwayFactor"];

	oc.Interaction.PredatorEnabled		= interaction["PredatorEnabled"];
	oc.Interaction.PredatorDistance		= interaction["PredatorDistance"];
	oc.Interaction.PredatorFactor		= interaction["PredatorFactor"];

	oc.Interaction.TurnAtBorder			= interaction["TurnAtBorder"];
	oc.Interaction.TurnMarginFactor		= interaction["TurnMarginFactor"];
	oc.Interaction.TurnFactor			= interaction["TurnFactor"];

	std::vector<int> temp_color_options = color["ColorOptions"];
	oc.Color.Flags = !temp_color_options.empty() ? CF_None : CF_Cycle;
	for (int i = 0; i < temp_color_options.size(); ++i)
		oc.Color.Flags |= util::pow(2, temp_color_options[i] - 1);

	oc.Color.PositionalWeight		= color["ColorPositionalWeight"];
	oc.Color.CycleWeight			= color["ColorCycleWeight"];
	oc.Color.DensityWeight			= color["ColorDensityWeight"];
	oc.Color.VelocityWeight			= color["ColorVelocityWeight"];
	oc.Color.RotationWeight			= color["ColorRotationWeight"];
	oc.Color.AudioWeight			= color["ColorAudioWeight"];
	oc.Color.FluidWeight			= color["ColorFluidWeight"];

	if ((oc.Color.Flags & CF_Positional) == CF_Positional)
	{
		oc.Positional.TopLeft		= StrToColor(color["TopLeft"]);
		oc.Positional.TopRight		= StrToColor(color["TopRight"]);
		oc.Positional.BotLeft		= StrToColor(color["BotLeft"]);
		oc.Positional.BotRight		= StrToColor(color["BotRight"]);
	}
	if ((oc.Color.Flags & CF_Cycle) == CF_Cycle)
	{
		oc.Cycle.Random				= color["CycleRandom"];
		oc.Cycle.Speed				= color["CycleSpeed"];

		ConvertToColor(oc.Cycle.Colors, color["CycleColors"]);
	}
	if ((oc.Color.Flags & CF_Density) == CF_Density)
	{
		oc.Density.Density				= color["Density"];
		oc.Density.DensityCycleEnabled	= color["DensityCycleEnabled"];
		oc.Density.DensityCycleSpeed	= color["DensityCycleSpeed"];

		ConvertToColor(oc.Density.Colors, color["DensityColors"]);
	}
	if ((oc.Color.Flags & CF_Velocity) == CF_Velocity)
	{
		ConvertToColor(oc.Velocity.Colors, color["VelocityColors"]);
	}
	if ((oc.Color.Flags & CF_Rotation) == CF_Rotation)
	{
		ConvertToColor(oc.Rotation.Colors, color["RotationColors"]);
	}
	if ((oc.Color.Flags & CF_Audio) == CF_Audio)
	{
		std::vector<std::string> temp_processes = color["AudioApps"];
		oc.Audio.Apps = std::vector<std::wstring>(temp_processes.size());

		for (int i = 0; i < oc.Audio.Apps.size(); ++i)
		{
			std::string process = temp_processes[i];
			oc.Audio.Apps[i] = std::wstring(process.begin(), process.end());
		}

		oc.Audio.Strength			= color["AudioStrength"];
		oc.Audio.Limit				= color["AudioLimit"];
		oc.Audio.Speed				= color["AudioSpeed"];
		oc.Audio.Density			= color["AudioDensity"];

		ConvertToColor(oc.Audio.Colors, color["AudioColors"]);
	}
	if ((oc.Color.Flags & CF_Fluid) == CF_Fluid)
	{
		oc.Fluid.Scale				= color["FluidScale"];
		oc.Fluid.MouseStrength		= color["FluidMouseStrength"];
		oc.Fluid.ColorVel			= color["FluidColorVel"];
		oc.Fluid.Diffusion			= color["FluidDiffusion"];
		oc.Fluid.Viscosity			= color["FluidViscosity"];

		ConvertToColor(oc.Fluid.Colors, color["FluidColors"]);
	}

	oc.Impulse.Enabled				= color["ImpulseEnabled"];
	oc.Impulse.Size					= color["ImpulseSize"];
	oc.Impulse.Speed				= color["ImpulseSpeed"];
	oc.Impulse.FadeDistance			= color["ImpulseFadeDistance"];
	oc.Impulse.Force				= color["ImpulseForce"];

	ConvertToColor(oc.Impulse.Colors, color["ImpulseColors"]);

	oc.Misc.GridExtraCells			= misc["GridExtraCells"];
	oc.Misc.CameraEnabled			= misc["CameraEnabled"];
	oc.Misc.CameraZoom				= misc["CameraZoom"];
	oc.Misc.VerticalSync			= misc["VerticalSync"];
	oc.Misc.MaxFramerate			= misc["MaxFramerate"];
	oc.Misc.PhysicsUpdateFreq		= misc["PhysicsUpdateFreq"];
	oc.Misc.PolicyThreshold			= misc["PolicyThreshold"];

	oc.Misc.DebugEnabled			= misc["DebugEnabled"];
	oc.Misc.DebugUpdateFreq			= misc["DebugUpdateFreq"];
	oc.Misc.DebugToggleKey			= misc["DebugToggleKey"];
}

Config::Config()
{
	UpdateMisc();
	Load();
}

void Config::Load()
{
	std::ifstream projectFile(FILE_NAME, std::ifstream::binary);
	if (projectFile.good())
	{
		try
		{
			LoadStatus = false;
			Config temp = *this;

			nlohmann::json json = nlohmann::json::parse(projectFile);
			ReadJSON(temp, json);

			*this = temp;
			LoadStatus = true;

			UpdateMisc();
		}
		catch (nlohmann::json::parse_error) { }
		catch (nlohmann::detail::type_error e) { }
	}
}

std::vector<Rebuild> Config::Refresh(Config& prev)
{
	std::vector<Rebuild> result;
	result.reserve(static_cast<int>(Rebuild::Count));

	Load();

	if (prev.Rules.SepDistance != Rules.SepDistance || 
		prev.Rules.AliDistance != Rules.AliDistance || 
		prev.Rules.CohDistance != Rules.CohDistance || 
		prev.Boids.Width != Boids.Width || 
		prev.Boids.Height != Boids.Height || 
		prev.Interaction.TurnAtBorder != Interaction.TurnAtBorder)
	{
		result.emplace_back(Rebuild::Grid);
	}

	if (prev.Boids.Count != Boids.Count)
		result.emplace_back(Rebuild::Boids);

	if (prev.Cycle.Random != Cycle.Random)
		result.emplace_back(Rebuild::BoidsCycle);

	if (Interaction.TurnAtBorder)
		result.emplace_back(Rebuild::Interp);

	if (prev.Background.Texture != Background.Texture ||
		prev.Background.UseWallpaper != Background.UseWallpaper)
		result.emplace_back(Rebuild::BackgroundTex);

	if (prev.Misc.VerticalSync != Misc.VerticalSync ||
		prev.Misc.MaxFramerate != Misc.MaxFramerate)
		result.emplace_back(Rebuild::Window);

	if (!std::ranges::equal(prev.Audio.Apps, Audio.Apps))
		result.emplace_back(Rebuild::Audio);

	if (prev.Misc.CameraZoom != Misc.CameraZoom)
		result.emplace_back(Rebuild::Camera);

	if (prev.Fluid.Scale != Fluid.Scale)
		result.emplace_back(Rebuild::Fluid);

	if (prev.Background.Color != Background.Color ||
		prev.Background.PositionX != Background.PositionX ||
		prev.Background.PositionY != Background.PositionY ||
		prev.Background.FitScreen != Background.FitScreen ||
		prev.Background.OverrideSize != Background.OverrideSize ||
		prev.Background.Width != Background.Width ||
		prev.Background.Height != Background.Height)
	{
		result.emplace_back(Rebuild::BackgroundProp);
	}

	return result;
}

void Config::UpdateMisc()
{
	Boids.ViewAngle = util::to_radians(Boids.ViewAngle) / 2.0f;

	Rules.SepDistance *= Rules.SepDistance;
	Rules.AliDistance *= Rules.AliDistance;
	Rules.CohDistance *= Rules.CohDistance;

	Interaction.PredatorDistance *= Interaction.PredatorDistance;

	BoidHalfSize = sf::Vector2f(Boids.Width, Boids.Height) / 2.0f;

	BoidSpeedInv = (Boids.SpeedMax == Boids.SpeedMin) ? 1.0f
		: (1.0f / (Boids.SpeedMax - Boids.SpeedMin));

	BoidSpeedMinSq = Boids.SpeedMin * Boids.SpeedMin;
	BoidSpeedMaxSq = Boids.SpeedMax * Boids.SpeedMax;
}
