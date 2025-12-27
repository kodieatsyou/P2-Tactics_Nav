#include <SDL.h>
#include <algorithm>
#include <cstdio>

#include <tn/GridMap.h>
#include <tn/DynamicOccupancy.h>
#include <tn/Types.h>
#include <tn/Pathfinding.h>
#include "tn/Reachability.h"
#include "tn/LOS.h"
#include "tn/InfluenceField.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

static tn::IVec2 ScreenToTile(int mouseX, int mouseY, int tileSize, int originX, int originY) {
    int x = (mouseX - originX) / tileSize;
    int y = (mouseY - originY) / tileSize;

    return tn::IVec2{x, y};
}

static void DrawFilledRect(SDL_Renderer* r, int x, int y, int w, int h, Uint8 rr, Uint8 gg, Uint8 bb, Uint8 aa=255) {
    SDL_SetRenderDrawColor(r, rr, gg, bb, aa);
    SDL_Rect rc{x, y, w, h};
    SDL_RenderFillRect(r, &rc);
}

static void DrawRect(SDL_Renderer *r, int x, int y, int w, int h, Uint8 rr, Uint8 gg, Uint8 bb, Uint8 aa = 255)
{
    SDL_SetRenderDrawColor(r, rr, gg, bb, aa);
    SDL_Rect rc{x, y, w, h};
    SDL_RenderDrawRect(r, &rc);
}

static SDL_Point TileCenterToScreen(tn::IVec2 t, int tileSize, int originX, int originY)
{
    return SDL_Point{
        originX + t.x * tileSize + tileSize / 2,
        originY + t.y * tileSize + tileSize / 2};
}

static void DrawPolyline(SDL_Renderer *r, const std::vector<tn::IVec2> &pts, int tileSize, int originX, int originY)
{
    if (pts.size() < 2)
        return;
    SDL_SetRenderDrawColor(r, 80, 200, 120, 255);

    for (size_t i = 0; i + 1 < pts.size(); ++i)
    {
        SDL_Point a = TileCenterToScreen(pts[i], tileSize, originX, originY);
        SDL_Point b = TileCenterToScreen(pts[i + 1], tileSize, originX, originY);
        SDL_RenderDrawLine(r, a.x, a.y, b.x, b.y);
    }
}

int main(int, char**) {

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const int windowW = 1540;
    const int windowH = 720;

    SDL_Window* window = SDL_CreateWindow(
        "Tactics Nav - Grid Debug",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowW, windowH,
        SDL_WINDOW_SHOWN
    );

    if(!window) {
        std::printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if(!renderer) {
        std::printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Setup IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    const int gridW = 50;
    const int gridH = 30;

    tn::GridMap map(gridW, gridH);
    tn::DynamicOccupancy occ(gridW, gridH);

    tn::IVec2 start(2, 2);
    tn::IVec2 goal{map.Width() - 3, map.Height() - 3};

    //Path settings
    tn::PathSettings pathSettings{};
    pathSettings.allowDiagonal = true;
    pathSettings.preventCornerCuts = true;
    pathSettings.allowPartial = true;
    tn::PathFinder pathfinder(map.Width(), map.Height());
    tn::PathResult pathResult{};
    bool pathDirty = true;

    //Reach settings
    tn::ReachSettings reachSettings{};
    reachSettings.allowDiagonal = true;
    reachSettings.preventCornerCut = true;
    reachSettings.moveBudget = 12.0f;
    tn::ReachableSet reachableSet{};
    bool reachDirty = true;
    bool showReachableOverlay = true;
    bool showReachableGradient = true;

    //LOS settings
    bool showLOS = true;
    tn::LOSSettings losSettings{};
    losSettings.blockByStatic = true;
    losSettings.blockByOccupancy = false;
    losSettings.includeEndpoints = false;
    tn::IVec2 losOrigin;

    //Enemied
    std::vector<tn::Enemy> enemies;

    //Influence
    tn::InfluenceSettings influenceSettings{};
    influenceSettings.maxRange = 10.0f;
    influenceSettings.baseThreat = 1.0f;
    influenceSettings.gateByLOS = true;
    tn::InfluenceField influence{};
    bool influenceDirty = true;
    bool showInfluence = true;

    int tileSize = 22;
    int originX = 20;
    int originY = 20;

    bool showGridLines = true;
    bool paintWalls = true;
    bool paintOccupancy = true;

    bool running = true;

    while(running) {
        //Handle events
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);

            if(e.type == SDL_QUIT) running = false;
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;

            if(io.WantCaptureMouse) continue;

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                tn::IVec2 t = ScreenToTile(mx, my, tileSize, originX, originY);

                if (!map.InBounds(t))
                    continue;

                const bool shift = (SDL_GetModState() & KMOD_SHIFT) != 0;
                const bool ctrl = (SDL_GetModState() & KMOD_CTRL) != 0;

                //Shift to use path tools
                if (shift)
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        if (!map.IsBlockedStatic(t) && !occ.IsOccupied(t))
                        {
                            start = t;
                            pathDirty = true;
                            reachDirty = true;
                            influenceDirty = true;
                        }
                    }
                    else if (e.button.button == SDL_BUTTON_RIGHT)
                    {
                        if (!map.IsBlockedStatic(t) && !occ.IsOccupied(t))
                        {
                            goal = t;
                            pathDirty = true;
                            reachDirty = true;
                            influenceDirty = true;
                        }
                    }
                    continue;
                }

                //Control for editing emeies
                if (ctrl && e.button.button == SDL_BUTTON_LEFT)
                {
                    enemies.push_back({t, 1.0f});
                    influenceDirty = true;
                    continue;
                }

                //Non shift to use edit tools
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    bool blocked = map.IsBlockedStatic(t);
                    map.SetBlocked(t, !blocked);

                    if (!blocked)
                    {
                        occ.SetOccupied(t, false);
                    }
                    pathDirty = true;
                    reachDirty = true;
                    influenceDirty = true;
                }
                else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    if (!map.IsBlockedStatic(t))
                    {
                        bool occupied = occ.IsOccupied(t);
                        occ.SetOccupied(t, !occupied);
                        pathDirty = true;
                        reachDirty = true;
                        influenceDirty = true;
                    }
                }
            }

            if(e.type == SDL_MOUSEMOTION) {
                Uint32 buttons = SDL_GetMouseState(nullptr, nullptr);
                if(buttons == 0) continue;

                int mx, my;
                SDL_GetMouseState(&mx, &my);
                tn::IVec2 t = ScreenToTile(mx, my, tileSize, originX, originY);
                if(!map.InBounds(t)) continue;

                if(buttons & SDL_BUTTON_LMASK) {
                    map.SetBlocked(t, true);
                    occ.SetOccupied(t, false);
                    pathDirty = true;
                    reachDirty = true;
                    influenceDirty = true;
                }

                if (buttons & SDL_BUTTON_RMASK)
                {
                    if(!map.IsBlockedStatic(t)){
                        occ.SetOccupied(t, true);
                        pathDirty = true;
                        reachDirty = true;
                        influenceDirty = true;
                    }
                }
            }
        }

        if (pathDirty)
        {
            pathResult = pathfinder.FindPath(map, occ, start, goal, pathSettings);
            pathDirty = false;
        }

        if (reachDirty)
        {
            reachableSet = tn::ComputeReachableSet(map, occ, start, reachSettings);
            reachDirty = false;
        }

        if (influenceDirty)
        {
            influence = tn::ComputeInfluenceField(map, occ, enemies, influenceSettings);
            influenceDirty = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug");
        ImGui::Text("LMB: Toggle wall | RMG: Toggle Occupancy");
        ImGui::SliderInt("Tile Size", &tileSize, 0, 48);
        ImGui::SliderInt("Origin X", &originX, 0, 200);
        ImGui::SliderInt("Origin Y", &originY, 0, 200);
        ImGui::Checkbox("Show Grid Lines", &showGridLines);
        ImGui::Separator();
        ImGui::Text("Shift+LMB: Start | Shift+RMB: Goal");
        ImGui::Text("Start (%d,%d)  Goal (%d,%d)", start.x, start.y, goal.x, goal.y);

        //PATH
        bool changed = false;
        changed |= ImGui::Checkbox("Diagonal", &pathSettings.allowDiagonal);
        changed |= ImGui::Checkbox("Prevent Corner Cut", &pathSettings.preventCornerCuts);
        changed |= ImGui::Checkbox("Allow Partial", &pathSettings.allowPartial);
        if (changed)
            pathDirty = true;

        ImGui::Text("Path: %s | Points: %d | Cost: %.2f",
                    pathResult.reachedGoal ? "reached" : "partial",
                    (int)pathResult.points.size(),
                    pathResult.totalCost);

        //INFLUENCE
        ImGui::Separator();
        ImGui::Checkbox("Show Influence", &showInfluence);
        ImGui::Checkbox("Influence gated by LOS", &influenceSettings.gateByLOS);
        ImGui::SliderFloat("Influence Range", &influenceSettings.maxRange, 1.0f, 20.0f);
        ImGui::SliderFloat("Base Threat", &influenceSettings.baseThreat, 0.1f, 5.0f);
        if (ImGui::Button("Recompute Influence"))
            influenceDirty = true;

        //REACH
        ImGui::Separator();
        ImGui::Checkbox("Show Reachable Overlay", &showReachableOverlay);
        ImGui::Checkbox("Reachable Gradient", &showReachableGradient);

        bool reachChanged = false;
        reachChanged |= ImGui::Checkbox("Reach Diagonal", &reachSettings.allowDiagonal);
        reachChanged |= ImGui::Checkbox("Reach Prevent Corner Cut", &reachSettings.preventCornerCut);
        reachChanged |= ImGui::SliderFloat("Move Budget (cost)", &reachSettings.moveBudget, 1.0f, 60.0f, "%.1f");
        if (reachChanged) {
            reachDirty = true;
        }
        int reachableCount = 0;
        for (uint8_t v : reachableSet.reachable) {
            reachableCount += (v != 0);
        }
        ImGui::Text("Reachable tiles: %d | MaxCostInSet: %.2f", reachableCount, reachableSet.maxCostInSet);

        //LOS
        ImGui::Separator();
        ImGui::Checkbox("Show LOS (hover)", &showLOS);
        ImGui::Checkbox("LOS blocked by occupancy", &losSettings.blockByOccupancy);
        ImGui::Checkbox("LOS include endpoints", &losSettings.includeEndpoints);

        int mx, my;
        SDL_GetMouseState(&mx, &my);
        tn::IVec2 hover = ScreenToTile(mx, my, tileSize, originX, originY);
        if(map.InBounds(hover)) {
            ImGui::SeparatorText("");
            ImGui::Text("Hover Tile: (%d, %d)", hover.x, hover.y);
            ImGui::Text("Blocked: %s", map.IsBlockedStatic(hover) ? "True" : "False");
            ImGui::Text("Occupied: %s", occ.IsOccupied(hover) ? "True" : "False");
        }
        ImGui::End();
        ImGui::Render();

        //Background
        SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
        SDL_RenderClear(renderer);

        //Grid
        for(int y = 0; y < map.Height(); ++y) {
            for(int x = 0; x < map.Width(); ++x) {
                tn::IVec2 p{x, y};
                int sx = originX + x * tileSize;
                int sy = originY + y * tileSize;

                //base tile
                if(map.IsBlockedStatic(p)) {
                    DrawFilledRect(renderer, sx, sy, tileSize, tileSize, 35, 35, 35, 255);
                } else {
                    DrawFilledRect(renderer, sx, sy, tileSize, tileSize, 55, 55, 55, 255);
                }

                //Occupied overlay
                if(!map.IsBlockedStatic(p) && occ.IsOccupied(p)) {
                    DrawFilledRect(renderer, sx + 3, sy + 3, tileSize - 6, tileSize - 6, 160, 60, 60, 255);
                }

                //Reachable
                if (showReachableOverlay && !map.IsBlockedStatic(p) && !occ.IsOccupied(p))
                {
                    int id = y * map.Width() + x;
                    if (id >= 0 && id < (int)reachableSet.reachable.size() && reachableSet.reachable[id])
                    {

                        // Determine intensity: either constant or gradient by cost
                        float tNorm = 1.0f;
                        if (showReachableGradient && reachableSet.maxCostInSet > 0.0001f)
                        {
                            float c = reachableSet.costTo[id];
                            tNorm = 1.0f - (c / reachableSet.maxCostInSet);
                            if (tNorm < 0.15f)
                                tNorm = 0.15f;
                            if (tNorm > 1.0f)
                                tNorm = 1.0f;
                        }

                        Uint8 a = (Uint8)(80 + 120 * tNorm);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        DrawFilledRect(renderer, sx, sy, tileSize, tileSize, 70, 140, 90, a);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    }
                }

                // Influence heatmap
                if (showInfluence && influence.maxThreat > 0.0001f)
                {
                    int id = y * map.Width() + x;
                    float t = influence.threat[id] / influence.maxThreat;
                    if (t > 0.01f)
                    {
                        Uint8 a = (Uint8)(200 * t);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        DrawFilledRect(renderer, sx, sy, tileSize, tileSize, 200, 60, 60, a);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    }
                }

                if(showGridLines) {
                    DrawRect(renderer, sx, sy, tileSize, tileSize, 25, 25, 25, 255);
                }
            }
        }

        //Hover
        {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            tn::IVec2 t = ScreenToTile(mx, my, tileSize, originX, originY);
            if (map.InBounds(t))
            {
                int sx = originX + t.x * tileSize;
                int sy = originY + t.y * tileSize;
                DrawRect(renderer, sx, sy, tileSize, tileSize, 220, 220, 220, 255);
            }
        }

        if (showLOS)
        {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            tn::IVec2 hover = ScreenToTile(mx, my, tileSize, originX, originY);

            losOrigin = start;

            if (map.InBounds(hover) && map.InBounds(losOrigin))
            {
                tn::LOSRay ray = tn::TraceLOS(map, occ, losOrigin, hover, losSettings);

                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                for (const auto &c : ray.cells)
                {
                    if (!map.InBounds(c))
                        continue;
                    int sx = originX + c.x * tileSize;
                    int sy = originY + c.y * tileSize;

                    // Clear = bluish blocked path = reddish
                    if (!ray.blocked)
                    {
                        DrawFilledRect(renderer, sx + tileSize / 4, sy + tileSize / 4, tileSize / 2, tileSize / 2, 80, 140, 255, 90);
                    }
                    else
                    {
                        DrawFilledRect(renderer, sx + tileSize / 4, sy + tileSize / 4, tileSize / 2, tileSize / 2, 255, 110, 90, 90);
                    }
                }
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                SDL_Point a = TileCenterToScreen(losOrigin, tileSize, originX, originY);
                SDL_Point b = TileCenterToScreen(hover, tileSize, originX, originY);

                if (!ray.blocked)
                    SDL_SetRenderDrawColor(renderer, 80, 140, 255, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 255, 110, 90, 255);

                SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);

                // If blocked mark the blocking tile
                if (ray.blocked && map.InBounds(ray.blockedAt))
                {
                    int sx = originX + ray.blockedAt.x * tileSize;
                    int sy = originY + ray.blockedAt.y * tileSize;
                    SDL_SetRenderDrawColor(renderer, 255, 230, 120, 255);
                    SDL_Rect rc{sx, sy, tileSize, tileSize};
                    SDL_RenderDrawRect(renderer, &rc);
                }
            }
        }

        // Start/goal tile highlight
        auto drawTileOutline = [&](tn::IVec2 p, Uint8 r, Uint8 g, Uint8 b)
        {
            int sx = originX + p.x * tileSize;
            int sy = originY + p.y * tileSize;
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_Rect rc{sx, sy, tileSize, tileSize};
            SDL_RenderDrawRect(renderer, &rc);
        };

        drawTileOutline(start, 90, 180, 255);
        drawTileOutline(goal, 255, 200, 90);

        // Path polyline
        DrawPolyline(renderer, pathResult.points, tileSize, originX, originY);

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    //Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}