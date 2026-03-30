#pragma once

/**
 * @brief Renderer-agnostic RGB color.
 *
 * Components are normalized to [0, 1]. Backends map this to their own
 * native color representation (OCCT, VTK, etc.).
 */
struct RgbColor
{
	double r = 1.0;
	double g = 1.0;
	double b = 1.0;
};

/**
 * @brief Renderer-agnostic style tokens for control-point visuals.
 *
 * This struct carries visual intent only (colors/sizes), without any backend
 * API types. Rendering adapters are responsible for mapping these values into
 * backend-specific objects.
 */
struct ControlPointVisualStyle
{
	RgbColor defaultColor{ 1.0, 1.0, 1.0 };
	RgbColor selectedColor{ 1.0, 0.55, 0.0 };
	double defaultMarkerScale = 30.0;
	double selectedMarkerScale = 40.0;
};
