#include "Config.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <ranges>

#include "CommonUtilities.hpp"

sf::Vector3f ConvertToColor(const nlohmann::basic_json<>::value_type& src)
{
	return sf::Vector3f
	{
		src["r"] / 255.999f,
		src["g"] / 255.999f,
		src["b"] / 255.999f
	};
}

std::vector<sf::Vector3f> ConvertToColorList(const std::vector<nlohmann::basic_json<>::value_type>& src)
{
	std::vector<sf::Vector3f> result(src.size());

	for (std::size_t i = 0; i < src.size(); ++i)
		result[i] = ConvertToColor(src[i]);

	return result;
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

	oc.Background.Color				= ConvertToColor(background["Color"]);
	oc.Background.Texture			= background["Texture"];
	oc.Background.Position			= sf::Vector2i((int)background["Position"]["x"], (int)background["Position"]["y"]);
	oc.Background.FitScreen			= background["FitScreen"];
	oc.Background.OverrideSize		= background["OverrideSize"];
	oc.Background.UseWallpaper		= background["UseWallpaper"];
	oc.Background.Width				= background["Width"];
	oc.Background.Height			= background["Height"];

	oc.Boids.Texture				= boids["Texture"];
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
	oc.Color.Flags = CF_None;
	for (std::size_t i = 0; i < temp_color_options.size(); ++i)
		oc.Color.Flags |= util::Pow(2, temp_color_options[i] - 1);

	oc.Color.PositionalWeight		= color["ColorPositionalWeight"];
	oc.Color.CycleWeight			= color["ColorCycleWeight"];
	oc.Color.DensityWeight			= color["ColorDensityWeight"];
	oc.Color.VelocityWeight			= color["ColorVelocityWeight"];
	oc.Color.RotationWeight			= color["ColorRotationWeight"];
	oc.Color.AudioWeight			= color["ColorAudioWeight"];
	oc.Color.FluidWeight			= color["ColorFluidWeight"];

	if ((oc.Color.Flags & CF_Positional) == CF_Positional)
	{
		oc.Positional.TopLeft		= ConvertToColor(color["TopLeft"]);
		oc.Positional.TopRight		= ConvertToColor(color["TopRight"]);
		oc.Positional.BotLeft		= ConvertToColor(color["BotLeft"]);
		oc.Positional.BotRight		= ConvertToColor(color["BotRight"]);
	}
	if ((oc.Color.Flags & CF_Cycle) == CF_Cycle)
	{
		oc.Cycle.Random				= color["CycleRandom"];
		oc.Cycle.Speed				= color["CycleSpeed"];

		oc.Cycle.Colors = ConvertToColorList(color["CycleColors"]);
	}
	if ((oc.Color.Flags & CF_Density) == CF_Density)
	{
		oc.Density.Density				= color["Density"];
		oc.Density.DensityCycleEnabled	= color["DensityCycleEnabled"];
		oc.Density.DensityCycleSpeed	= color["DensityCycleSpeed"];

		oc.Density.Colors = ConvertToColorList(color["DensityColors"]);
	}
	if ((oc.Color.Flags & CF_Velocity) == CF_Velocity)
	{
		oc.Velocity.Colors = ConvertToColorList(color["VelocityColors"]);
	}
	if ((oc.Color.Flags & CF_Rotation) == CF_Rotation)
	{
		oc.Rotation.Colors = ConvertToColorList(color["RotationColors"]);
	}
	if ((oc.Color.Flags & CF_Audio) == CF_Audio)
	{
		std::vector<std::string> temp_processes = color["AudioApps"];
		oc.Audio.Apps = std::vector<std::wstring>(temp_processes.size());

		for (std::size_t i = 0; i < oc.Audio.Apps.size(); ++i)
		{
			std::string process = temp_processes[i];
			oc.Audio.Apps[i] = std::wstring(process.begin(), process.end());
		}

		oc.Audio.Strength			= color["AudioStrength"];
		oc.Audio.Limit				= color["AudioLimit"];
		oc.Audio.Speed				= color["AudioSpeed"];
		oc.Audio.Density			= color["AudioDensity"];

		oc.Audio.Colors = ConvertToColorList(color["AudioColors"]);
	}
	if ((oc.Color.Flags & CF_Fluid) == CF_Fluid)
	{
		oc.Fluid.Scale				= color["FluidScale"];
		oc.Fluid.MouseStrength		= color["FluidMouseStrength"];
		oc.Fluid.ColorVel			= color["FluidColorVel"];
		oc.Fluid.Diffusion			= color["FluidDiffusion"];
		oc.Fluid.Viscosity			= color["FluidViscosity"];

		oc.Fluid.Colors = ConvertToColorList(color["FluidColors"]);
	}

	oc.Impulse.Enabled				= color["ImpulseEnabled"];
	oc.Impulse.Size					= color["ImpulseSize"];
	oc.Impulse.Speed				= color["ImpulseSpeed"];
	oc.Impulse.FadeDistance			= color["ImpulseFadeDistance"];
	oc.Impulse.Force				= color["ImpulseForce"];

	oc.Impulse.Colors = ConvertToColorList(color["ImpulseColors"]);

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
	std::ifstream projectFile(FILE_NAME);
	if (projectFile.good())
	{
		try
		{
			LoadStatus = false;
			Config temp = *this;

			nlohmann::json json = nlohmann::json::parse(projectFile, nullptr, true, true);
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
	result.reserve((int)Rebuild::Count);

	Load();

	if ( prev.Rules.SepDistance != Rules.SepDistance || 
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

	if (prev.Boids.Texture != Boids.Texture)
		result.emplace_back(Rebuild::BoidsTex);

	if (prev.Color.Flags != Color.Flags ||
		prev.Cycle.Random != Cycle.Random)
		result.emplace_back(Rebuild::BoidsCycle);

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
		prev.Background.Position != Background.Position ||
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
	Boids.ViewAngle = util::ToRadians(Boids.ViewAngle) / 2.0f;

	Rules.SepDistance *= Rules.SepDistance;
	Rules.AliDistance *= Rules.AliDistance;
	Rules.CohDistance *= Rules.CohDistance;

	Interaction.PredatorDistance *= Interaction.PredatorDistance;
	Interaction.BoidAddMouseDiff *= Interaction.BoidAddMouseDiff;

	BoidHalfSize = sf::Vector2f(Boids.Width, Boids.Height) / 2.0f;

	BoidSpeedInv = (Boids.SpeedMax == Boids.SpeedMin) ? 1.0f
		: (1.0f / (Boids.SpeedMax - Boids.SpeedMin));

	BoidSpeedMinSq = Boids.SpeedMin * Boids.SpeedMin;
	BoidSpeedMaxSq = Boids.SpeedMax * Boids.SpeedMax;
}
