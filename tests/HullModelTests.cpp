#include "core/HullModel.h"

#include <gp_Pnt.hxx>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
void
require(bool condition, const std::string& message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

void
requireValid(const HullModel& model)
{
	const std::vector<std::string> errors = model.validateTopology();
	if (!errors.empty())
	{
		throw std::runtime_error("Topology validation failed: " + errors.front());
	}
}

void
testCreateFaceAndAdjacency()
{
	HullModel model;
	const int p0 = model.addPoint(gp_Pnt(0, 0, 0));
	const int p1 = model.addPoint(gp_Pnt(10, 0, 0));
	const int p2 = model.addPoint(gp_Pnt(10, 10, 0));
	const int p3 = model.addPoint(gp_Pnt(0, 10, 0));

	const int faceId = model.addFace({p0, p1, p2, p3});
	require(model.pointCount() == 4, "Expected four control points");
	require(model.edgeCount() == 4, "A quad should create four edges");
	require(model.faceCount() == 1, "Expected one control face");
	require(model.face(faceId).edgeIds.size() == 4, "Face must retain its edge loop");
	requireValid(model);
}

void
testSplitEdgeRewritesFace()
{
	HullModel model;
	const int p0 = model.addPoint(gp_Pnt(0, 0, 0));
	const int p1 = model.addPoint(gp_Pnt(10, 0, 0));
	const int p2 = model.addPoint(gp_Pnt(10, 10, 0));
	const int p3 = model.addPoint(gp_Pnt(0, 10, 0));
	const int faceId = model.addFace({p0, p1, p2, p3});
	const int edgeId = model.face(faceId).edgeIds.front();

	const EdgeSplitResult result = model.splitEdge(edgeId);
	require(model.pointCount() == 5, "Splitting an edge should add one point");
	require(model.edgeCount() == 5, "Splitting a quad edge should add one net edge");
	require(model.faceCount() == 1, "Splitting an edge should preserve adjacent face count");
	require(result.replacementFaceIds.size() == 1, "One adjacent face should be rewritten");
	require(
		model.face(result.replacementFaceIds.front()).pointIds.size() == 5,
		"Rewritten face should contain the midpoint"
	);
	require(
		model.point(result.pointId).getPosition().Distance(gp_Pnt(5, 0, 0)) < 1.0e-9,
		"Split point should be located at the edge midpoint"
	);
	requireValid(model);
}

void
testInsertEdgeSplitsFace()
{
	HullModel model;
	const int p0 = model.addPoint(gp_Pnt(0, 0, 0));
	const int p1 = model.addPoint(gp_Pnt(10, 0, 0));
	const int p2 = model.addPoint(gp_Pnt(10, 10, 0));
	const int p3 = model.addPoint(gp_Pnt(0, 10, 0));
	const int faceId = model.addFace({p0, p1, p2, p3});
	const EdgeSplitResult split = model.splitEdge(model.face(faceId).edgeIds.front());

	const auto replacementFaces =
		model.insertEdge(split.replacementFaceIds.front(), split.pointId, p3);
	require(model.edgeCount() == 6, "Inserting a diagonal should add one edge");
	require(model.faceCount() == 2, "Inserting a diagonal should split one face into two");
	require(model.face(replacementFaces.first).pointIds.size() >= 3, "First face is valid");
	require(model.face(replacementFaces.second).pointIds.size() >= 3, "Second face is valid");
	requireValid(model);
}

void
testRejectNonManifoldEdge()
{
	HullModel model;
	const int a = model.addPoint(gp_Pnt(0, 0, 0));
	const int b = model.addPoint(gp_Pnt(10, 0, 0));
	const int c = model.addPoint(gp_Pnt(0, 10, 0));
	const int d = model.addPoint(gp_Pnt(0, -10, 0));
	const int e = model.addPoint(gp_Pnt(0, 0, 10));
	model.addFace({a, b, c});
	model.addFace({b, a, d});

	bool rejected = false;
	try
	{
		model.addFace({a, b, e});
	}
	catch (const std::invalid_argument&)
	{
		rejected = true;
	}
	require(rejected, "A third face on one edge must be rejected");
	require(model.faceCount() == 2, "Rejected operation must not modify face topology");
	requireValid(model);
}
} // namespace

int
main()
{
	try
	{
		testCreateFaceAndAdjacency();
		testSplitEdgeRewritesFace();
		testInsertEdgeSplitsFace();
		testRejectNonManifoldEdge();
	}
	catch (const std::exception& error)
	{
		std::cerr << "HullModelTests failed: " << error.what() << '\n';
		return EXIT_FAILURE;
	}

	std::cout << "HullModelTests passed\n";
	return EXIT_SUCCESS;
}
