// SDL stuff
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

// Regular C++ stuff
#include <iostream>
#include <thread>
#include <string>
#include <random>
#include <fstream>
#include <filesystem>
#include <bit>
#include <array>

// Simplex noise/worldgen library
#include "SimplexNoise.h"

// Serialization stuff
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>

// Serialization stuff
using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

// Game state
bool running = true;
// File path stuff
std::string cachedPath = SDL_GetBasePath();
std::string worldsPath = cachedPath + "worlds/";
std::string soundsPath = cachedPath + "assets/audio/";
// World gen stuff
std::mt19937 rng;
bool worlds[6] = {0};
int8_t worldCount = 5;
int32_t seed;
// Audio engine init
bool audiosuccess = MIX_Init();
MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

struct Player {
    int64_t z;
    int64_t x;
    int8_t gameState;
};
Player player;

void checkForWorlds() {
    for (int i = 1; i <= 5; i++) {
        if (std::filesystem::exists(worldsPath + "world" + std::to_string(i))) {
            worlds[i] = true;
        }
    }
}

void playSound (std::string path) {
    auto soundTrack = MIX_CreateTrack(mixer);
    path = soundsPath + path;
    auto sound = MIX_LoadAudio(mixer,path.c_str(),false);
    MIX_SetTrackAudio(soundTrack, sound);
    MIX_PlayTrack(soundTrack, 0);
}

void generateChunk(uint32_t heightmapSeed, uint32_t blockSeed, uint32_t biomeSeed, uint32_t chunkX, uint32_t chunkZ) {

    for (float x = chunkX * 16; x < chunkX * 16 + 16; x++) {
        for (float z = chunkZ * 16; z < chunkZ * 16 + 16; z++) {
            float noiseValue = SimplexNoise(heightmapSeed, 0.01).fractal(6, x, z);
            if (noiseValue > 0.5) {
                //std::cout << "M";
            } else if (noiseValue < -0.5) {
                //std::cout << ".";
            }
            else {
                //std::cout << "X";
            }
        }
        //std::cout << std::endl;
    }
}

int64_t getChunkX(int64_t x) {
    return x / 16;
}
int64_t getChunkZ(int64_t z) {
    return z / 16;
}

void generateWorld() {
    int8_t world = 0;
    for(int i = 1; i <= worldCount; i++) {
        if(!worlds[i]) {
            world = i;
            break;
        }
    }
    if(world == 0) {
        std::cout << "All world slots are full!" << std::endl;
        return;
    }
    std::string worldPathString = worldsPath + "world" + std::to_string(world) + ".bfwf";
    std::filesystem::path worldPath{worldPathString};
    std::fstream worldFile;
    std::string magicString = "bfwf";
    seed = rng();
    worldFile.open (worldPath, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    worldFile.write((char*)&magicString, magicString.length());
    worldFile.write((char*)&seed, sizeof(seed));
    worldFile.close();
    std::cout << "Generated world " << std::to_string(world) << " with seed " << std::to_string(seed) << std::endl;
    std::cout << "World file saved to " << worldPath << std::endl;
    player.gameState = 1;
}

void loadWorld(int8_t world) {
    // not implemented yet
}

void readButton(int8_t button) {
    if(button == 0) {
        playSound("play.opus");
    } else if (button == 1) {
        playSound("options.opus");
    } else if (button == 2) {
        playSound("quit_game.opus");
    }
}

void readWorld(int8_t world) {
    if(world == 0) {
        playSound("create_world.opus");
    } else {
        playSound("world.opus");
        playSound(std::to_string(world) + ".opus");
    }
}

int main() {
    // DEVELOPER FLAGS
    double targetFPS = 180.0;
    double targetFrameTime = 1e9 / targetFPS;
    bool limitFPS = true;
    bool calculateFPS = false;
    bool debugWindow = true;
    bool skipWelcome = true;
    bool skipBackgroundMusic = true;

    // Initialize Window
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window;
    SDL_Surface* surface;
    if(debugWindow) {
        window = SDL_CreateWindow("Blind Factory", 640, 480, SDL_WINDOW_RESIZABLE);
    } else {
        window = SDL_CreateWindow("Blind Factory", 1920, 1080, SDL_WINDOW_FULLSCREEN);
    }
    surface = SDL_LoadBMP("./assets/textures/h.bmp");
    SDL_BlitSurface(surface, nullptr, SDL_GetWindowSurface(window), nullptr);
    SDL_UpdateWindowSurface(window);

    if(!audiosuccess) {
        std::cout << "Audio engine failed to initialize!" << std::endl;
    } else {
        std::cout << "Audio engine initialized successfully!" << std::endl;
    }
    // Background music
    if(!skipBackgroundMusic) playSound("hikaru_miles.opus");
    
    // Initialize events and keyboard state
    SDL_Event event;
    const bool* keys = SDL_GetKeyboardState(nullptr);
    
    int16_t accumulateX = 0;
    int16_t accumulateZ = 0;
    int16_t playerSpeed = 1; // How many ms it takes to move one block

    // Initialize controls
    struct MouseState{
        bool left;
        bool right;
        bool middle;
    };
    MouseState mouseState;
    struct KeyboardState {
        bool up;
        bool down;
        bool enter;
    };
    KeyboardState keyboardState;

    // Initalize player
    player.x = 0;
    player.z = 0;
    player.gameState = 0;
    //gameState 0 = main menu; 1 = playing; 2 = paused;

    // Initalize menu
    struct Menu {
        int8_t button;
        int8_t level;
        int8_t world;
    };
    Menu menu;
    menu = {0, 0, 0};
    bool initialized = false; //init welcoming
    bool initWorldSelect = false; //init world select

    checkForWorlds();

    // Game loop
    while(running) {
        // Set up deltaTime
        Uint64 pretime = SDL_GetTicksNS();
        // Handle quit event and mouse click events
        mouseState = {false, false, false};
        keyboardState = {false, false, false};
        while (SDL_PollEvent(&event)) {
           if (event.type == SDL_EVENT_QUIT) {
               running = false;
           } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
               if (event.button.button == SDL_BUTTON_LEFT) {
                   mouseState.left = true;
               }
               if (event.button.button == SDL_BUTTON_RIGHT) {
                   mouseState.right = true;
               }
               if (event.button.button == SDL_BUTTON_MIDDLE) {
                   mouseState.middle = true;
               }
            } else if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_LEFT) {
                    mouseState.left = true;
                }
                if (event.key.key == SDLK_RIGHT) {
                    mouseState.right = true;
                }
                if (event.key.key == SDLK_UP) {
                    mouseState.middle = true;
                    keyboardState.up = true;
                }
                if (event.key.key == SDLK_DOWN) {
                    mouseState.middle = true;
                    keyboardState.down = true;
                }
                if(event.key.key == SDLK_KP_ENTER || event.key.key == SDLK_RETURN) {
                    keyboardState.enter = true;
                }
            }
        }

        // Main menu
        if(player.gameState == 0) {
            if(!initialized && !skipWelcome) {
                playSound("welcome.opus");
                initialized = true;
            }
            if(menu.level == 0) {
                if (keyboardState.up) {
                    menu.button = (menu.button - 1) % 3;
                    readButton(menu.button);
                } else if (keyboardState.down) {
                    menu.button = (menu.button + 1) % 3;
                    readButton(menu.button);
                }
                if (keyboardState.enter) {
                    if(menu.button == 0) {
                        menu.level = 1;
                        keyboardState.enter = false;
                    } else if (menu.button == 1) {
                        //menu.level = 1;
                        // options not implemented
                    } else if (menu.button == 2) {
                        running = false;
                    }
                }
            }
            if(menu.level == 1) {
                if (!initWorldSelect) {
                    playSound("select_world.opus");
                    initWorldSelect = true;
                }
                if (keyboardState.up) {
                    menu.world = menu.world - 1;
                    readWorld(menu.world);
                    std::cout << std::to_string(menu.world) << "up" << std::endl;
                } else if (keyboardState.down) {
                    menu.world = menu.world + 1;
                    readWorld(menu.world);
                    std::cout << std::to_string(menu.world) << "down" << std::endl;
                }
                if (keyboardState.enter) {
                    if(menu.world == 0) {
                        generateWorld();
                    } else {
                        loadWorld(menu.world);
                    }
                }
            }
            
        }

        // Playing
        if(player.gameState == 1) {
            // Player controls
            if(keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) {
                playerSpeed = 1;
            } else if(keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) {
                playerSpeed = 6;
            } else {
                playerSpeed = 3;
            }
            if (keys[SDL_SCANCODE_ESCAPE]) {
            running = false;
            }
            if (keys[SDL_SCANCODE_W]) {
                accumulateZ += playerSpeed;
            }
            if (keys[SDL_SCANCODE_S]) {
                accumulateZ -= playerSpeed;
            }
            if (keys[SDL_SCANCODE_A]) {
                accumulateX -= playerSpeed;
            }
            if (keys[SDL_SCANCODE_D]) {
                accumulateX += playerSpeed;
            }

            // Player Movement
            if(accumulateX >= 180) {
                player.x++;
                accumulateX = 0;
            } else if (accumulateX <= -180) {
                player.x--;
                accumulateX = 0;
            }
            if(accumulateZ >= 180) {
                player.z++;
                accumulateZ = 0;
            } else if (accumulateZ <= -180) {
                player.z--;
                accumulateZ = 0;
            }
            
            // TEMPORARY CHUNK GEN ALGORTHM, WILL BE IMPROVED LATER
            generateChunk(seed, seed, seed, getChunkX(player.x), getChunkZ(player.z));
            generateChunk(seed, seed, seed, getChunkX(player.x) + 1, getChunkZ(player.z));
            generateChunk(seed, seed, seed, getChunkX(player.x), getChunkZ(player.z) + 1);
            generateChunk(seed, seed, seed, getChunkX(player.x) + 1, getChunkZ(player.z) + 1);
            generateChunk(seed, seed, seed, getChunkX(player.x) - 1, getChunkZ(player.z));
            generateChunk(seed, seed, seed, getChunkX(player.x) - 1, getChunkZ(player.z) - 1);
            generateChunk(seed, seed, seed, getChunkX(player.x), getChunkZ(player.z) - 1);
            generateChunk(seed, seed, seed, getChunkX(player.x) + 1, getChunkZ(player.z) - 1);
            generateChunk(seed, seed, seed, getChunkX(player.x) - 1, getChunkZ(player.z) + 1);
        }
        
        // FPS stuff
        Uint64 posttime = SDL_GetTicksNS();
        double deltatime = (posttime - pretime);
        if (deltatime < targetFrameTime && limitFPS) {
            SDL_DelayNS(targetFrameTime - deltatime);
            posttime = SDL_GetTicksNS();
            deltatime = (posttime - pretime);
        }
        if (calculateFPS) {
            double FPS = 1e9 / deltatime;
            std::cout << "FPS: " << FPS << std::endl;
        }
    }
    SDL_Quit();
    return 0;
}

std::array<std::uint8_t, 4> u32_to_be_bytes(std::uint32_t v) {
    return {static_cast<std::uint8_t>(v >> 24), static_cast<std::uint8_t>(v >> 16), static_cast<std::uint8_t>(v >> 8), static_cast<std::uint8_t>(v)};
}
