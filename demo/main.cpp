#include <SDL.h>
#include <algorithm>
#include <cstdio>

#include <tn/GridMap.h>
#include <tn/DynamicOccupancy.h>
#include <tn/Types.h>

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

            if(e.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                tn::IVec2 t = ScreenToTile(mx, my, tileSize, originX, originY);

                if(map.InBounds(t)) {
                    if(e.button.button == SDL_BUTTON_LEFT) {
                        //Toggle wall with left mouse
                        bool blocked = map.IsBlockedStatic(t);
                        map.SetBlocked(t, !blocked);

                        if(!blocked == true) { occ.SetOccupied(t, false); }
                    }
                    else if(e.button.button == SDL_BUTTON_RIGHT) {
                        if(!map.IsBlockedStatic(t)) {
                            bool occupied = occ.IsOccupied(t);
                            occ.SetOccupied(t, !occupied);
                        }
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
                }

                if (buttons & SDL_BUTTON_RMASK)
                {
                    if(!map.IsBlockedStatic(t)) occ.SetOccupied(t, true);
                }
            }
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