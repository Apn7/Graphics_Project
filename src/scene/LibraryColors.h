// =============================================================================
// LibraryColors.h — Named Color Constants for the Library Scene (Phase 5)
// =============================================================================
// Every color in the scene is named here. No raw glm::vec3 literals in Build
// functions — always use these constants.
// =============================================================================

#pragma once

#include <glm/glm.hpp>

namespace LibraryColors {
    // Structural
    const glm::vec3 WALL_CREAM      = {0.92f, 0.90f, 0.85f};
    const glm::vec3 FLOOR_TILE      = {0.78f, 0.78f, 0.80f};
    const glm::vec3 CEILING_WOOD    = {0.22f, 0.14f, 0.06f};

    // Wood (shelves, tables, door, chair frames)
    const glm::vec3 WOOD_DARK       = {0.30f, 0.18f, 0.06f};  // Dark walnut shelves
    const glm::vec3 WOOD_MEDIUM     = {0.45f, 0.28f, 0.10f};  // Chair frames, door
    const glm::vec3 WOOD_TABLE      = {0.25f, 0.14f, 0.05f};  // Dark reading tables

    // Chair cushions
    const glm::vec3 CUSHION_BLUE    = {0.12f, 0.18f, 0.55f};  // Navy blue
    const glm::vec3 CUSHION_DARK    = {0.08f, 0.12f, 0.40f};  // Cushion shadow areas

    // Windows
    const glm::vec3 WINDOW_TEAL     = {0.40f, 0.82f, 0.80f};  // Teal/cyan glass
    const glm::vec3 WINDOW_FRAME    = {0.22f, 0.14f, 0.06f};  // Dark wood frame

    // Books — varied colors for visual interest
    const glm::vec3 BOOK_RED        = {0.75f, 0.15f, 0.10f};
    const glm::vec3 BOOK_BLUE       = {0.15f, 0.30f, 0.70f};
    const glm::vec3 BOOK_GREEN      = {0.10f, 0.55f, 0.20f};
    const glm::vec3 BOOK_YELLOW     = {0.85f, 0.75f, 0.10f};
    const glm::vec3 BOOK_ORANGE     = {0.85f, 0.40f, 0.10f};
    const glm::vec3 BOOK_PURPLE     = {0.45f, 0.10f, 0.65f};
    const glm::vec3 BOOK_WHITE      = {0.90f, 0.88f, 0.85f};
    const glm::vec3 BOOK_BROWN      = {0.50f, 0.28f, 0.10f};

    // Metal/lamp
    const glm::vec3 METAL_DARK      = {0.15f, 0.15f, 0.15f};  // Fan fixtures
    const glm::vec3 LAMP_WARM       = {1.00f, 0.92f, 0.60f};  // Pendant bulb color

    // Librarian Desk
    const glm::vec3 WOOD_MAHOGANY   = {0.36f, 0.17f, 0.06f};  // Deep reddish-brown desk body
    const glm::vec3 DESK_TRIM       = {0.52f, 0.30f, 0.12f};  // Lighter trim/drawers
    const glm::vec3 BRASS           = {0.80f, 0.62f, 0.22f};  // Drawer pulls, nameplate
    const glm::vec3 CUSHION_GREEN   = {0.07f, 0.27f, 0.11f};  // Dark green leather (executive chair)
    const glm::vec3 PAPER_WHITE     = {0.94f, 0.93f, 0.88f};  // Paper / documents
    const glm::vec3 LAMP_METAL      = {0.16f, 0.16f, 0.18f};  // Study lamp arms/shade
}
