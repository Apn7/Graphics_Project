# Comprehensive Computer Graphics Project Requirements

## Assignment Requirements

## Teacher mentioned them in the lab class

* **Camera & Viewing Controls:**
    * Implement a Bird's Eye View viewing transformation.
    * Implement camera rotations: Pitch (Key `X`), Yaw (Key `Y`), and Roll (Key `Z`).
    * Implement flying simulator movement: Forward (`W`), Backward (`S`), Left (`A`), Right (`D`), Up (`E`), and Down (`R`).
    * Rotate the camera around a look-at point (Key `F`).
    * Divide the viewport into 4 equal parts. Control the scene of each viewport with user input (e.g., toggling Combine Lighting, Ambient Only, Diffuse Only, Directional Only alongside Isometric, Top, Front, and Inside views).
* **Lighting Implementation:**
    * Include multiple light sources: Point lights, Directional light, Spot light (with a single cut-off angle), and Emissive light.
    * Lighting toggles:
        * Directional light (Key `1`), Point lights (Key `2`), Spot light (Key `3`).
        * Ambient light (Key `5`), Diffuse light (Key `6`), Specular light (Key `7`).
        * General Light On/Off (Key `L`).
* **Textures & Surfaces:**
    * Implement multiple textured objects:
        * Simple texture without surface color.
        * Blended texture with surface color (color computed on both the vertex and the fragment).
    * Include textured curvy surfaced objects (must include at least a sphere and a cone).
    * Incorporate curvy objects using Bezier curves, Spline Curves, and Ruled surfaces.
* **Scene Dynamics & Interaction:**
    * Implement a rotating fan (Key `G`).
    * Include doors and windows that can be opened and closed.
    * Simulate the function of dynamic equipment.
    * Add keyboard interactions to toggle features.
    * Print all controls and key actions to the console.
* **Custom Math Function:**
    * Implement your own custom version of the `glm::rotate` function.

## Special Requirements

* **Complex Objects & Hierarchical Movement:**
    * Include scene objects with complex, hierarchical movements.
        * *Example:* A robot arm/hand where the arm and fingers have individual pivot points, movement restrictions, and follow kinematics equations.
* **Custom Tool Integration:**
    * Create at least one object using the provided "wine glass making program" by exporting the points, and use that object in the final scene.
* **Advanced Environment & Animations:**
    * Generate tree leaves using fractals.
    * Implement random motion algorithms for birds.
    * Implement motion for both the camera and light sources.
    * Tie dynamic lighting projections to the position of the sun.
* **Physics:**
    * Implement basic physics interactions, specifically collision detection (as emphasized by Masud sir).
* **Specific Object Request:**
    * Add a clock to the scene (as requested by Taj sir).

---

## Final Evaluation Guidelines (from Teacher — April 2026)

### Presentation Requirements
- Prepare a **5-minute presentation** (includes video time)
- Record a **1:50–2:00 minute video** showing interactive and dynamic motion features
- The video must demonstrate features — teacher will not ask, you must prove your work through the presentation
- Report **must include the project proposal**

### Marking Criteria (40 marks total)

| # | Category | Marks |
|---|----------|-------|
| i | Idea of the project, creation of diverse 3D objects and aesthetics | 10 |
| ii | Inclusion of lab and assignment topics: **Curves, Textures, Viewing, Lightings, Fractals** | 10 |
| iii | Presentation and Reporting (report must include project proposal) | 10 |
| iv | Dynamic motion, animation and interactive features | 10 |
| v | Additional features | Bonus marks |

> ⚠️ **Warning:** Any suspicious activity — identical project with minimal edit, previous year project, or inability to explain any implemented feature — will **drastically decrease the score**.


### Marking Criteria (40 marks total)

| # | Category | Marks | Our Status |
|---|----------|-------|------------|
| i | Idea of project, diverse 3D objects & aesthetics | 10 | ✅ Good (library scene, chairs, shelves, books, fan, lamps) |
| ii | Lab & assignment topics: **Curves, Textures, Viewing, Lightings, Fractals** | 10 | ⚠️ Partial (Textures ✅, Lighting ✅, Curves ❌, Fractals ❌, Viewing ⚠️) |
| iii | Presentation & Reporting (includes project proposal) | 10 | ⚠️ Needs recording + report finalization |
| iv | Dynamic motion, animation, interactive features | 10 | ✅ Good (fan, camera, lighting toggles, texture modes) |
| v | Additional/bonus features | Bonus | N/A |

> ⚠️ **Warning:** Identical projects, previous year work, or inability to explain features = drastic score reduction.

---

## 📋 Lab & Assignment Topics — Mandatory for Category ii (10 marks)

These 5 topics are explicitly listed by the teacher. All must be represented:

### 1. Curves ❌ Not yet done
- Bezier curves / Spline curves / Ruled surfaces
- Must have **at least a sphere and a cone** with textures (curvy surfaced objects)
- **Priority for us:** Add a textured sphere (e.g. globe on a table) or cone (lampshade)

### 2. Textures ✅ Done (Phase 6)
- Simple texture without surface color → floor (implemented)
- Blended texture with surface color on vertex stage → walls (implemented)
- Blended texture with surface color on fragment stage → table tops (implemented)

### 3. Viewing ⚠️ Partially done
- ✅ FPS camera with WASD + mouse look + sprint + scroll zoom
- ❌ Bird's Eye View transformation
- ❌ Camera pitch/yaw/roll explicit keys (X, Y, Z) *(Note: mouse already does pitch/yaw)*
- ❌ Camera rotate around look-at point (Key `F`)
- ❌ 4-viewport split with per-viewport controls
- **Minimum for marks:** Bird's Eye View toggle is most important

### 4. Lightings ✅ Done (Phase 7)
- ✅ Directional light (sun) — Key `I`
- ✅ Point lights (3 pendant lamps) — Key `O`
- ✅ Ambient / Diffuse / Specular toggles — Keys `6`, `7`, `8`
- ✅ Global light ON/OFF — Key `L`
- ✅ Day / Night mode — Key `N`
- ❌ Spot light with cutoff angle (not yet done)
- ❌ Emissive light (not yet done)

### 5. Fractals ❌ Not yet done
- Generate tree leaves using fractals
- **For a library:** Could be a potted plant in the corner using fractal branching
- This is needed for marks — should be addressed

---

## 🎮 Feature Requirements (from Lab, originally from teacher in class)

### Camera & Viewing Controls
- ✅ Flying simulator: W/A/S/D (forward/back/strafe), Q (up), E (down) — *Note: teacher says R for down, we use E*
- ✅ Mouse look (yaw + pitch)
- ✅ Scroll zoom (FOV)
- ✅ Tab toggle cursor
- ❌ Bird's Eye View (Key TBD — `B` is free)
- ❌ Explicit pitch (Key `X`), yaw (Key `Y`), roll (Key `Z`) key rotations
- ❌ Camera rotate around look-at point (Key `F`)
- ❌ 4-viewport split

### Lighting Implementation
- ✅ Directional light — Key `I`
- ✅ Point lights — Key `O` 
- ✅ Ambient — Key `6` 
- ✅ Diffuse — Key `7` 
- ✅ Specular — Key `8` 
- ✅ General Light On/Off — Key `L` 
- ❌ Spot light — Key `3` 
- ❌ Emissive light

### Textures & Surfaces
- ✅ Simple texture (floor)
- ✅ Vertex-blend texture (walls)
- ✅ Fragment-blend texture (table tops, ceiling)
- ❌ Sphere with texture
- ❌ Cone with texture
- ❌ Bezier / Spline / Ruled surface objects

### Scene Dynamics & Interaction
- ✅ Rotating fan — Key `G` *(exact match)*
- ✅ All controls printed to console
- ✅ Keyboard interactions for all features
- ❌ Door open/close interaction *(geometry exists, no animation)*
- ❌ Dynamic equipment simulation

### Custom Math
- ❌ Custom `glm::rotate` reimplementation

---

## 🌟 Special / Bonus Features (from class notes)

- ❌ Hierarchical complex object (robot arm etc.) — **Bonus**
- ❌ Wine glass tool object — **Bonus** (teacher provided a tool for this)
- ❌ Fractal tree leaves — **Needed for marks**
- ❌ Bird random motion — **Bonus**
- ❌ Camera + light source motion — **Bonus**
- ❌ Sun-tied dynamic lighting — **Bonus**
- ❌ Collision detection (Masud sir) — **Bonus**
- ❌ Clock in scene (Taj sir) — **Bonus / Partial**

---

## 🎯 Our Priority Plan (what to do next for max marks)

Given the marking breakdown and our current state:

### HIGH PRIORITY — Directly affects category ii marks (Curves + Fractals = 4/10 at risk)
1. **Fractal plant/tree** in the library corner — covers Fractals
2. **Bezier sphere or curve-based object** (textured globe or vase) — covers Curves

### MEDIUM PRIORITY — Boost category ii + iv
3. **Bird's Eye View** toggle key — covers Viewing
4. **Spot light** — completes Lighting category
5. **Door open/close** animation — boosts category iv (dynamic motion)

### LOW PRIORITY — Polish
6. Custom `glm::rotate` function — easy, small code change
7. Camera explicit rotation keys (X, Y, Z) — already works via mouse, just need key binding

### PRESENTATION
- Record 2-minute video demonstrating: fly-through, lighting toggles (L/N/I/O), texture mode cycle (T), fan (G), day/night (N)
- Finalize report with project proposal included
