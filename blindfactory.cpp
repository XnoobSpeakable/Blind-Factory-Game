#include <SDL3/SDL.h>
#include <iostream>
#include <thread>
#include <string>
#include "SimplexNoise.h"

bool running = true;
std::string cachedPath = SDL_GetBasePath();

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

struct {
    int64_t z;
    int64_t x;
    struct {} inventory;
} player;

int main() {
    // Constants:
    double targetFPS = 180.0;
    double targetFrameTime = 1e9 / targetFPS;
    bool limitFPS = true;
    bool calculateFPS = false;
    bool debugWindow = true;

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
    
    // Background music
    std::thread audioThread(playSound, "hikaru_miles.wav");
    audioThread.detach();

    // Initialize events and keyboard state
    SDL_Event event;
    const bool* keys = SDL_GetKeyboardState(nullptr);
    
    int16_t accumulateX = 0;
    int16_t accumulateZ = 0;
    int16_t playerSpeed = 1; // How many ms it takes to move one block

    for (int x = 0; x < 100; x++) {
        for (int z = 0; z < 100; z++) {
            float noiseValue = SimplexNoise(0).fractal(0, x * 0.1f, z * 0.1f);
            if (noiseValue > 0) {
                std::cout << "#";
            } else {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
    }

    // Game loop
    while(running) {
        // Set up deltaTime
        Uint64 pretime = SDL_GetTicksNS();

        // Handle quit event and mouse click events
        while (SDL_PollEvent(&event)) {
           if (event.type == SDL_EVENT_QUIT) {
               running = false;
           } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
               if (event.button.button == SDL_BUTTON_LEFT) {
                   std::cout << "l" << std::endl;
               } else if (event.button.button == SDL_BUTTON_RIGHT) {
                   std::cout << "r" << std::endl;
               } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                   std::cout << "m" << std::endl;
               }
            }
        }
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

        // Game logic
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
        
        //std::cout << "Player position: (" << player.x << ", " << player.z << ")" << std::endl;
        
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
