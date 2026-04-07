# Project Proposal: 3D Library Simulation Using OpenGL 3.3

## Overview

**Project:** Interactive 3D Library Environment
**Technology:** Modern OpenGL 3.3 | C++ | GLFW | GLAD | GLM | stb_image
**Course:** CSE 4208 — Computer Graphics Laboratory | KUET

---

## Introduction

This project develops an interactive 3D Library environment using Modern OpenGL 3.3. The library represents a realistic indoor reading space containing bookshelves, reading tables, chairs, windows, and lighting elements. The objective is to demonstrate core computer graphics concepts — 3D modeling, transformations (translation, rotation, scaling), lighting, texture mapping, and camera navigation — in a structured and explainable manner. The entire scene is rendered in real-time using a custom OpenGL rendering pipeline with vertex shaders and fragment shaders written in GLSL.

---

## Proposed Features

- Complete 3D library interior: walls, floor, ceiling, bookshelves, reading tables, chairs
- Bookshelves populated with books modeled using scaled and repeated 3D primitives
- Smooth first-person camera navigation using keyboard (WASD) and mouse
- Interactive lighting system with per-light ON/OFF toggle via keyboard
- Day and night mode by adjusting light intensity and color
- Ceiling fan rotation animation using real-time deltaTime-based frame updates
- Phong lighting model using directional light (sun) and point lights (pendants)
- Texture mapping on floor, walls, tables, and bookshelves
- Perspective projection with a configurable view frustum for realistic depth perception
- Hierarchical modeling for complex objects using parent-child transformations
- Modern OpenGL 3.3 pipeline: VAO, VBO, EBO, and custom GLSL shaders (vertex + fragment)

---

## Dynamic Interactive Objects

| Object | Behavior |
|---|---|
| Camera | WASD movement + mouse look, sprint (Shift), scroll zoom |
| Ceiling Fan | Continuous Y-axis rotation, animated per-frame via deltaTime |
| Point Lights | Toggle ON/OFF individually via keyboard keys |
| Day/Night Mode | Switch by modifying directional light color and intensity |

---

## Objects with Complex Shapes

All objects are built from transformed cubes (scaled cuboids) using `Transform::TRS()` — combining **translation**, **rotation**, and **scaling** transformations:

| Object | Construction |
|---|---|
| Bookshelves | Multiple cuboids: side panels + shelves + back panel + base, arranged using hierarchical modeling |
| Chairs | Seat + cushion + backrest + 4 legs + 2 armrests using scaled cubes |
| Reading Tables | Tabletop + 4 legs + X-brace support underneath |
| Ceiling Fan | Central hub + motor housing + 5 rotating blades at 72° intervals |

---

## Graphics Concepts Demonstrated

| Concept | Implementation |
|---|---|
| 3D Transformations | Translation, Rotation, Scaling via GLM matrices |
| Hierarchical Modeling | Parent-child object composition for complex shapes |
| Phong Lighting Model | Ambient + Diffuse + Specular components |
| Vertex & Fragment Shaders | Custom GLSL shaders for all rendering |
| Texture Mapping | UV-mapped textures on floor, walls, and furniture |
| Perspective Projection | View frustum with configurable FOV using GLM |
| Real-Time Rendering | deltaTime-based animation for frame-rate independence |
| Camera System | First-person view, pitch/yaw control, zoom via scroll |

---

## Expected Outcome

An interactive, visually realistic 3D library environment where users can:
- Walk through the room using first-person camera controls
- Observe Phong-lit surfaces with directional sunlight and pendant point lights
- Toggle individual lights and switch between day/night modes
- Watch ceiling fans animate in real time using deltaTime-based rotation
- See texture mapping applied across floor, walls, and furniture
- Observe perspective projection and depth rendering in a real-time scene
- Switch texture rendering modes live via keyboard
