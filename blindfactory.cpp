#include <SDL3/SDL.h>
#include <iostream>
#include <thread>
#include <string>
#include <random>
#include <fstream>
#include <filesystem>
#include "SimplexNoise.h"

bool running = true;
std::string cachedPath = SDL_GetBasePath();
std::mt19937 rng;
std::string worldsPath = cachedPath + "worlds/";
int8_t worlds[6] = {0};
int8_t worldCount = 5;
int32_t seed;

struct {
    int64_t z;
    int64_t x;
    struct {} inventory;
    int8_t gameState;
} player;

void checkForWorlds() {
    for (int i = 1; i <= 5; i++) {
        if (std::filesystem::exists(worldsPath + "world" + std::to_string(i))) {
            worlds[i] = 1;
        }
    }
}

SDL_AppResult playSound (std::string path) {
    static SDL_AudioStream *stream = NULL;
    static Uint8 *wav_data = NULL;
    static Uint32 wav_data_len = 0;
    SDL_AudioSpec spec;
    path = cachedPath + "assets/audio/" + path;

    if (!SDL_LoadWAV(path.c_str(), &spec, &wav_data, &wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_ResumeAudioStreamDevice(stream);


    SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
    SDL_FlushAudioStream(stream);


    SDL_free(wav_data);
    return SDL_APP_CONTINUE;
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

void generateWorld(int8_t world) {
    std::string worldPath = worldsPath + "world" + std::to_string(world) + ".bfwf";
    std::fstream worldFile;
    std::string magicString = "bfwf";
    seed = rng();
    worldFile.open (worldPath, std::ios::in | std::ios::out | std::ios::binary);
    worldFile.write((char*)&magicString, magicString.length());
    worldFile.write((char*)&seed, sizeof(seed));
    player.gameState = 1;
}

void loadWorld(int8_t world) {
    // not implemented yet
}

SDL_AppResult playMusic () {
    SDL_AppResult result = SDL_APP_CONTINUE;
    while(result == SDL_APP_CONTINUE) {
        result = playSound("hikaru_miles.wav");
        SDL_Delay(180000);
    }
    return SDL_APP_CONTINUE;
}

void playSoundThread(std::string path) {
    std::thread audioThread(playSound, path);
    audioThread.detach();
}

void readButton(int8_t button) {
    if(button == 0) {
        playSoundThread("play.wav");
    } else if (button == 1) {
        playSoundThread("options.wav");
    } else if (button == 2) {
        playSoundThread("quit_game.wav");
    }
}

void readWorld(int8_t world) {
    if(world == 0) {
        playSoundThread("create_world.wav");
    } else {
        playSoundThread("world.wav");
        playSoundThread(std::to_string(world) + ".wav");
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

    // Initialize events and keyboard state
    SDL_Event event;
    const bool* keys = SDL_GetKeyboardState(nullptr);
    
    // Background music
    //std::thread audioThread(playMusic);
    //audioThread.detach();
    
    int16_t accumulateX = 0;
    int16_t accumulateZ = 0;
    int16_t playerSpeed = 1; // How many ms it takes to move one block

    // Initialize controls
    struct {
        bool left;
        bool right;
        bool middle;
    } mouseState;
    struct {
        bool up;
        bool down;
        bool enter;
    } keyboardState;

    // Initalize player
    player.x = 0;
    player.z = 0;
    player.gameState = 0;
    //gameState 0 = main menu; 1 = playing; 2 = paused;

    // Initalize menu
    struct {
        int8_t button;
        int8_t level;
        int8_t world;
    } menu;
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
                playSoundThread("welcome.wav");
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
                    playSoundThread("select_world.wav");
                    initWorldSelect = true;
                }
                if (keyboardState.up) {
                    menu.world = (menu.world - 1) % (worldCount + 1);
                    readWorld(menu.world);
                    std::cout << std::to_string(menu.world) << "g" << std::endl;
                } else if (keyboardState.down) {
                    menu.world = (menu.world + 1) % (worldCount + 1);
                    readWorld(menu.world);
                    std::cout << std::to_string(menu.world) << "h" << std::endl;
                }
                if (keyboardState.enter) {
                    if(menu.world == 0) {
                        generateWorld(menu.world);
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
