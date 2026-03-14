// SDL stuff
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
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
#include <limits>
#include <map>

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

// Application running state
bool running = true;

// File path stuff
std::string cachedPath = SDL_GetBasePath();
std::string worldsPath = cachedPath + "worlds/";
std::string soundsPath = cachedPath + "assets/audio/";

// Rng stuff
std::random_device rd;
std::mt19937 eng(rd());
std::uniform_int_distribution<> distr(INT32_MIN, INT32_MAX);
int32_t rn() {
    return distr(eng);
} 

// World gen stuff
std::array<bool, 6> worlds = {0};
int8_t worldCount = 5;
int32_t heightSeed;
int32_t temperatureSeed;
int32_t qualitySeed;

// Audio engine init
bool audiosuccess = MIX_Init();
MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

struct Player {
    int64_t z;
    int64_t x;
    int8_t gameState;
};
Player player;

// Initialize blocks
class Block {
    private:
        std::string group;
        std::string name;
        uint16_t hardness;
        bool interactable;
        bool breakable;
        bool hasData;
        uint16_t breaksTo;
    public:
        Block(std::string ingroup, std::string inname, uint16_t inhardness, bool ininteractable, bool inbreakable, bool inhasData, uint16_t inBreaksTo = 75) {
            group = ingroup;
            name = inname;
            hardness = inhardness;
            interactable = ininteractable;
            breakable = inbreakable;
            hasData = inhasData;
            breaksTo = inBreaksTo;
        }
        const std::string& getGroup() {
            return group;
        }
        const std::string& getName() {
            return name;
        }
        uint16_t getHardness() {
            return hardness;
        }
        bool isInteractable() {
            return interactable;
        }
        bool isBreakable() {
            return breakable;
        }
        bool hasBlockData() {
            return hasData;
        }
        uint16_t getBreaksTo() {
            return breaksTo;
        }
};



// Block ID organization is by worldgen criteria
auto blocks{std::to_array<Block>({
    // too complicated for now, keeping just in case
    /*
    {"liquids/natural", "very_cold_water", 0, false, false, false},
    {"solids/other", "deep_ice", 150, false, true, false},
    {"solids/other", "ocean_ice", 120, false, true, false},
    {"solids/other", "ice", 120, false, true, false},
    {"solids/other", "pure_snow", 10, false, true, false},
    */

    // Extremely cold biome
    // Low elevation
    {"solids/other", "ice", 120, false, true, false}, //0
    {"unobtainables", "redirect", 0, false, false, false, 0},
    {"unobtainables", "redirect", 0, false, false, false, 0},
    {"unobtainables", "redirect", 0, false, false, false, 0},
    {"unobtainables", "redirect", 0, false, false, false, 0},
    // Medium elevation
    {"unobtainables", "redirect", 0, false, false, false}, //5
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // High elevation
    {"unobtainables", "redirect", 0, false, false, false}, //10
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},

    // Cold biome
    //Low elevation
    {"unobtainables", "redirect", 0, false, false, false}, //15
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // Medium elevation
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // High elevation
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},

    // Temperate biome
    //Low elevation
    {"unobtainables", "redirect", 0, false, false, false}, //30
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // Medium elevation
    {"solids/dirts", "dirt", 50, false, true, false}, //35
    {"unobtainables", "redirect", 0, false, false, false, 35},
    {"unobtainables", "redirect", 0, false, false, false, 35},
    {"unobtainables", "redirect", 0, false, false, false, 35},
    {"unobtainables", "redirect", 0, false, false, false, 35},
    // High elevation
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},

    // Hot biome
    //Low elevation
    {"unobtainables", "redirect", 0, false, false, false}, //45
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // Medium elevation
    {"solids/dirts", "dry_dirt", 60, false, true, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // High elevation
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},

    // Extremely hot biome
    //Low elevation
    {"unobtainables", "redirect", 0, false, false, false}, //60
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // Medium elevation
    {"unobtainables", "redirect", 0, false, false, false}, //65
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    // High elevation
    {"unobtainables", "redirect", 0, false, false, false}, //70
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},
    {"unobtainables", "redirect", 0, false, false, false},


    {"unobtainables", "air", 0, false, false, false}, //75

    {"liquids/natural", "warm_water", 0, false, false, false},
    {"liquids/natural", "cold_water", 0, false, false, false},
    {"liquids/natural", "lava", 0, false, false, false},
    {"liquids/natural", "magma", 0, false, false, false},
    {"liquids/natural", "crude_oil", 0, false, false, false}, //80



    {"solids/stones", "granite", 120, false, true, false},
    {"solids/stones", "chalk", 60, false, true, false},
    {"solids/stones", "claystone", 80, false, true, false},
    {"solids/stones", "basalt", 200, false, true, false},
    {"solids/stones", "sandstone", 90, false, true, false}, //85
    {"solids/stones", "limestone", 100, false, true, false},
    {"solids/stones", "mudstone", 70, false, true, false},
    {"solids/stones", "shale", 110, false, true, false},
    {"solids/stones", "gneiss", 180, false, true, false},
    {"solids/stones", "diorite", 130, false, true, false}, //90
    {"solids/stones", "gravel", 50, false, true, false},


    {"solids/dirts", "wet_dirt", 50, false, true, false},
    {"solids/dirts", "beach_sand", 40, false, true, false},
    {"solids/dirts", "desert_sand", 40, false, true, false},
    {"solids/dirts", "clay", 40, false, true, false}, //95
})};

Block getBlock(uint16_t id) {
    if(id >= blocks.size()) {
        return Block("invalid", "invalid", 0, true, false, false);
    }
    return blocks[id];
}

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

void debugWorldGen(uint32_t heightSeed, uint32_t temperatureSeed, uint32_t qualitySeed) {
    // block noise
    for (float x = 0; x < 85; x++) {
        for (float z = 0; z < 370; z++) {
            float hnoiseValue = SimplexNoise(temperatureSeed, 0.001, 1, 2, 0.4).fractal(5, x*16, z*16);
            int8_t noiseValue = std::floor((hnoiseValue + 1) * 10);
            if (noiseValue >= 18) {
                std::cout << "M";
            } else if (noiseValue >= 13) {
                std::cout << "X";
            } else if (noiseValue >= 7) {
                std::cout << "+";
            } else if (noiseValue >= 2) {
                std::cout << ".";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // heightmap noise
    for (float x = 0; x < 85; x++) {
        for (float z = 0; z < 370; z++) {
            float hnoiseValue = SimplexNoise(heightSeed, 0.01).fractal(6, x, z);
            int8_t noiseValue = std::floor((hnoiseValue + 1) * 10);
            if (noiseValue > 15) {
                std::cout << "M";
            } else if (noiseValue < 5) {
                std::cout << ".";
            }
            else {
                std::cout << "+";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // heightmap noise on four
    for (float x = 0; x < 85; x++) {
        for (float z = 0; z < 370; z++) {
            float hnoiseValue = SimplexNoise(heightSeed, 0.01, 1, 2).fractal(6, x*4, z*4);
            int8_t noiseValue = std::floor((hnoiseValue + 1) * 10);
            if (noiseValue > 15) {
                std::cout << "M";
            } else if (noiseValue < 5) {
                std::cout << ".";
            }
            else {
                std::cout << "+";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // temperature noise
    for (float x = 0; x < 85; x++) {
        for (float z = 0; z < 370; z++) {
            float hnoiseValue = SimplexNoise(temperatureSeed, 0.001, 1, 2, 0.4).fractal(5, x, z);
            int8_t noiseValue = std::floor((hnoiseValue + 1) * 10);
            if (noiseValue >= 18) {
                std::cout << "M";
            } else if (noiseValue >= 13) {
                std::cout << "X";
            } else if (noiseValue >= 7) {
                std::cout << "+";
            } else if (noiseValue >= 2) {
                std::cout << ".";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // temperature noise on 16
    for (float x = 0; x < 300; x++) {
        for (float z = 0; z < 370; z++) {
            float hnoiseValue = SimplexNoise(temperatureSeed, 0.001, 1, 2, 0.4).fractal(5, x*16, z*16);
            int8_t noiseValue = std::floor((hnoiseValue + 1) * 10);
            if (noiseValue >= 18) {
                std::cout << "M";
            } else if (noiseValue >= 13) {
                std::cout << "X";
            } else if (noiseValue >= 7) {
                std::cout << "+";
            } else if (noiseValue >= 2) {
                std::cout << ".";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::array<uint16_t, 256> generateChunk(uint32_t heightSeed, uint32_t temperatureSeed, uint32_t qualitySeed, uint32_t chunkX, uint32_t chunkZ) {
    uint8_t i = 0;
    std::array<uint16_t, 256> chunk = {0};

    for (float x = chunkX * 16; x < chunkX * 16 + 16; x++) {
        for (float z = chunkZ * 16; z < chunkZ * 16 + 16; z++) {
            float qualityNoiseValue = SimplexNoise(qualitySeed, 0.25, 1, 2).fractal(3, x, z);
            float heightNoiseValue = SimplexNoise(heightSeed, 0.01).fractal(6, x, z);
            float temperatureNoiseValue = SimplexNoise(temperatureSeed, 0.001, 1, 2, 0.4).fractal(5, x, z);

            uint8_t quality = std::floor((qualityNoiseValue + 1) * 10);
            uint8_t height = std::floor((heightNoiseValue + 1) * 10);
            uint8_t temperature = std::floor((temperatureNoiseValue + 1) * 10);

            uint8_t temperatureLevel;
            uint8_t heightLevel;
            uint8_t qualityLevel;

            if (temperature >= 18) {
                temperatureLevel = 4;
            } else if (temperature >= 13) {
                temperatureLevel = 3;
            } else if (temperature >= 7) {
                temperatureLevel = 2;
            } else if (temperature >= 2) {
                temperatureLevel = 1;
            } else {
                temperatureLevel = 0;
            }

            if (height > 15) {
                heightLevel = 2;
            } else if (height < 5) {
                heightLevel = 1;
            }
            else {
                heightLevel = 0;
            }

            if (quality >= 18) {
                qualityLevel = 4;
            } else if (quality >= 13) {
                qualityLevel = 3;
            } else if (quality >= 7) {
                qualityLevel = 2;
            } else if (quality >= 2) {
                qualityLevel = 1;
            } else {
                qualityLevel = 0;
            }

            uint16_t generatedID = temperatureLevel * 15 + heightLevel * 5 + qualityLevel;

            // Handle redirect blocks;
            Block genBlock = getBlock(generatedID);
            if(genBlock.getName() == "redirect") {
                generatedID = genBlock.getBreaksTo();
            }

            chunk[i] = generatedID;

            i++;
        }
    }

    return chunk;
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
    temperatureSeed = rn();
    heightSeed = rn();
    qualitySeed = rn();
    worldFile.open (worldPath, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    worldFile.write((char*)&magicString, magicString.length());
    //worldFile.write((char*)&seed, sizeof(seed));
    worldFile.close();
    std::cout << "Generated world " << std::to_string(world) << std::endl;
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

int main(int argc, char *argv[]) {
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

    std::cout << getBlock(0).getGroup() << std::endl;

    //debugWorldGen(rn(), rn(), rn());

    std::map<int,int> idMap =  {{0, 0}, {1, 0}, {2, 0}};

    std::cout << idMap[0] << idMap[1] << idMap[2] << std::endl;

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
            
            std::array<uint16_t, 256> currentChunk = generateChunk(temperatureSeed, heightSeed, qualitySeed, getChunkX(player.x), getChunkZ(player.z));
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