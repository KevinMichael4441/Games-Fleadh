#ifndef ORGANISM_MEMBRANE_H
#define ORGANISM_MEMBRANE_H

#include <raylib.h>

#include "simulation/organism.h"

//=================================================================
// Organism Membrane
//
// Liquid metaball-based membrane that wraps around ALL Organism's
// internal structures (pseudopods, vacuoles, nucleus).
//
// Uses optimized raymarching metaball algorithm for R36S performance.
// The membrane creates a smooth, organic outline around ALL 7 organisms.
//
// Integration with Organism:
//   - All organism's parts are treated as internal "molecules"
//   - Membrane wraps around all internal structures
//   - Bezier curve creates smooth liquid edge
//
// Performance optimized for R36S ARM:
//   - Reduced ray count (180 rays)
//   - Cached math (radius_squared, cos/sin)
//   - Single-pass smoothing
//   - Minimal allocations
//=================================================================

#ifdef __cplusplus
extern "C"
{
#endif

// Performance tuning for R36S
#define ORGANISM_MEMBRANE_N_RAYS 180	  // Raymarch resolution (lower = faster)
#define ORGANISM_MEMBRANE_N_ANCHORS 10	  // Bezier control points
#define ORGANISM_MEMBRANE_BEZIER_STEPS 30 // Curve smoothness
#define ORGANISM_MEMBRANE_MAX_PARTS 64	  // Max internal structures to wrap (7 organisms × 8 parts each)

	typedef enum
	{
		ORGANISM_SHAPE_IDLE,
		ORGANISM_SHAPE_STRETCH,
		ORGANISM_SHAPE_SQUASH,
		ORGANISM_SHAPE_COUNT
	} OrganismShapeType;

	// Internal structure (pseudopod, vacuole, nucleus, etc)
	typedef struct OrganismPart
	{
		Vector2 offset;		  // Offset from membrane center
		float radius;		  // Part radius
		float radius_squared; // Cached r squared for fast metaball
		bool active;		  // Is this part used?
	} OrganismPart;

	// Tweakable shape parameters
	typedef struct OrganismShapeParams
	{
		float stretch_length;	  // How long stretch is (2.5)
		float stretch_width;	  // How thin stretch is (0.2)
		float squash_width;		  // How wide squash is (1.8)
		float squash_height;	  // How flat squash is (0.15)
		float idle_breath_amount; // Breathing magnitude (0.15)
		float idle_breath_speed;  // Breathing speed (1.5)
	} OrganismShapeParams;

	typedef struct OrganismMembrane
	{
		Vector2 center;	  // Membrane center position (centroid of all organisms)
		Vector2 velocity; // Movement velocity

		// Internal parts (from ALL Organism structures)
		OrganismPart parts[ORGANISM_MEMBRANE_MAX_PARTS];
		int part_count;

		// Shape state
		OrganismShapeType currentShape;
		float global_pulse; // Breathing phase
		float tension;		// Membrane tightness (0-1)
		float viscosity;	// Thickness (0-1)

		// Metaball calculation
		float metaball_threshold;								// Outline threshold
		float radii[ORGANISM_MEMBRANE_N_RAYS];					// Computed outline radii
		float radii_smoothing_buffer[ORGANISM_MEMBRANE_N_RAYS]; // Smoothing buffer
		Vector2 anchors[ORGANISM_MEMBRANE_N_ANCHORS];			// Bezier control points

		// Shape parameters
		OrganismShapeParams shapeParams;
	} OrganismMembrane;

	//=================================================================
	// Organism Membrane
	//=================================================================

	// Initialise membrane
	void OrganismMembraneInit(OrganismMembrane *membrane, Vector2 center);

	// Add an internal part (pseudopod, organelle, etc)
	void OrganismMembraneAddPart(OrganismMembrane *membrane, Vector2 offset, float radius);

	// Clear all parts (call before rebuilding from Organism)
	void OrganismMembraneClearParts(OrganismMembrane *membrane);

	// Update membrane (call every frame)
	void OrganismMembraneUpdate(OrganismMembrane *membrane, float dt, float elapsed);

	// Set shape type
	void OrganismMembraneSetShape(OrganismMembrane *membrane, OrganismShapeType shape, float direction);

	// Compute the outline (TODO: expensive call once per frame)
	void OrganismMembraneComputeOutline(OrganismMembrane *membrane, float time);

	// Draw the membrane outline
	void OrganismMembraneDraw(const OrganismMembrane *membrane, float thickness, Color color);

	// Draw with glow effect
	void OrganismMembraneDrawWithGlow(const OrganismMembrane *membrane);

	// Cleanup (no-op, no heap allocations)
	void OrganismMembraneExit(OrganismMembrane *membrane);

	// Helper: Rebuild membrane parts from ALL organisms
	// Call this each frame to sync membrane with organism's internal parts
	// This adds ALL 7 organisms as internal parts for the membrane
	void OrganismMembraneRebuildFromOrganism(
		OrganismMembrane *membrane,
		const Organism *organism);

#ifdef __cplusplus
}
#endif

#endif // ORGANISM_MEMBRANE_H