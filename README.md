# Tactics Navigation & Evaluation Module (C++)

A standalone, engine-agnostic **grid-based navigation and tactical evaluation system** implemented in modern C++ (C++20).  
This project focuses on correctness, determinism, and tooling—prioritizing **debuggability and extensibility** over visual polish.

The module is designed to resemble real gameplay/AI infrastructure used in turn-based or tactical games.

---

## Overview

This project implements a complete tactical movement decision pipeline:

1. **Pathfinding (A\*)**
2. **Reachable set computation (Dijkstra, movement budget)**
3. **Line-of-Sight raycasting (supercover grid tracing)**
4. **Influence / threat heatmap**
5. **Multi-factor tactical tile evaluation**
6. **Interactive debug visualization**

All systems are live-editable and recompute in real time through a lightweight SDL2 + ImGui debug viewer.

---

## Key Features

### Navigation
- Grid-based A* pathfinding
- 4-way / 8-way movement
- Optional diagonal corner-cut prevention
- Static terrain blocking
- Dynamic occupancy blocking
- Partial path fallback
- Deterministic results

### Reachability
- Dijkstra-based reachable set within a movement cost budget
- Terrain-aware movement costs
- Parent reconstruction for cheap path-to-any-tile
- Visual overlay with optional cost gradient

### Line of Sight (LOS)
- Deterministic **supercover Bresenham raycasting**
- Optional blocking by:
  - static terrain
  - dynamic occupancy
- Debug visualization:
  - ray path
  - blocking cell identification
- Reused across cover, threat, and attack evaluation

### Influence / Threat Field
- Per-tile scalar threat values
- Distance falloff
- Optional LOS gating
- Additive contributions from multiple enemies
- Heatmap visualization

### Tactical Evaluation
Each reachable tile is scored using a weighted combination of:
- **Cover** (LOS-based, smooth 0–1 gradient)
- **Threat** (from influence field)
- **Attack opportunity** (range + LOS)
- **Objective proximity**
- **Movement cost efficiency**
- **Local mobility / openness**

Outputs:
- Best tile selection
- Top-N ranked tiles
- Per-tile score breakdown
- Path reconstruction to chosen tile

All weights and behaviors are tunable live via ImGui.

---

## Debug Viewer

The included SDL2 demo acts as a **tactical debugger**, not a game.

### Controls
- **LMB**: toggle static wall
- **RMB**: toggle dynamic occupancy
- **Drag**: paint walls / occupancy
- **Shift + LMB**: set start position
- **Shift + RMB**: set goal position
- **Alt + LMB**: set objective
- **Ctrl + LMB / RMB**: add / remove enemy units

### Visual Overlays
- Pathfinding result
- Reachable set (with cost gradient)
- LOS raycast (hover-based)
- Influence heatmap
- Best tactical tile highlight
- Path to best tile
- Top-N scoring breakdown (ImGui)

The viewer is intentionally minimal and exists to **validate and reason about the systems**.
