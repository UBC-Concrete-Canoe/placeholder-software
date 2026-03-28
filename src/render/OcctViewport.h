#ifndef OCCTVIEWPORT_H
#define OCCTVIEWPORT_H

#include <AIS_InteractiveContext.hxx>
#include <QWindow>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <vector>

class ControlPoint;
class VisualPoint;

/**
 * @brief The rendering engine layer for OpenCascade viewport.
 *
 * Manages the OCCT viewer, context, and view. Provides a clean interface for:
 * - Displaying and removing geometry (TopoDS_Shape objects)
 * - Viewport transformations (fitting, zoom, pan, rotate)
 * - Display mode control (shaded vs wireframe)
 * - View preset configurations (top, bottom, front, side views)
 */
class OcctViewport
{
public:
	///! Constructor.
	OcctViewport();
	//! Destructor.
	~OcctViewport() = default;

	/**
	 * @brief Initialize the OCCT viewer within a native OS window.
	 *
	 * Sets up the OpenGL driver, viewer, context, view, and lighting.
	 * Must be called before any rendering occurs. Internally handles
	 * platform-specific window embedding (Windows/macOS/Linux).
	 *
	 * @param windowHandle Native window handle from Qt (WId)
	 */
	void initialize(WId windowHandle);

	/**
	 * @brief Display a shape in the viewport.
	 *
	 * Wraps the shape in an AIS_Shape and adds it to the context.
	 * Automatically fits the view to show the entire shape.
	 *
	 * @param shape The geometry to display (TopoDS_Shape)
	 */
	void displayShape(const TopoDS_Shape& shape);

	/**
	 * @brief Display a selectable visual marker for a model control point.
	 * @param point Non-owning pointer to model control point data.
	 */
	void displayControlPoint(const ControlPoint* point);

	/**
	 * @brief Synchronize all displayed visual points with their model state.
	 *
	 * Calls each point's synchronize() and updates presentation/selection style
	 * if needed without recreating AIS objects.
	 */
	void synchronizeVisualPoints();

	/**
	 * @brief Clear all displayed objects from the viewport.
	 */
	void removeAll();

	/**
	 * @brief Fit the view to show all objects.
	 */
	void fitAll();

	/**
	 * @brief Fit the view to show only selected objects.
	 */
	void fitSelected();

	/**
	 * @brief Set the viewing angle (e.g., top view, front view, isometric).
	 * @param orientation V3d view orientation preset
	 */
	void setViewPreset(V3d_TypeOfOrientation orientation);

	/**
	 * @brief Toggle between shaded and wireframe display modes.
	 * @param wireframe True for wireframe, false for shaded
	 */
	void setShadingMode(bool wireframe);

	/**
	 * @brief Trigger a view redraw on the next render cycle.
	 */
	void redraw();

	/**
	 * @brief Get the interactive context for direct OCCT access.
	 * @return Reference to the AIS_InteractiveContext
	 */
	Handle(AIS_InteractiveContext) getContext() { return myContext; }

	/**
	 * @brief Get the 3D view handle for direct OCCT access.
	 * @return Reference to the V3d_View
	 */
	Handle(V3d_View) getView() { return myView; }

private:
	/**
	 * @brief Apply selected/unselected style to tracked visual points.
	 * @return True if any point style changed.
	 */
	bool updateVisualPointSelectionStyles();
	Handle(V3d_Viewer) myViewer;
	Handle(V3d_View) myView;
	Handle(AIS_InteractiveContext) myContext;
	std::vector<Handle(VisualPoint)> myVisualPoints;
};

#endif // OCCTVIEWPORT_H
