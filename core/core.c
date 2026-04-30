//core.c

/* include all headers and files
set defines,globals etc. for core stuff only

*/

//stretchy buffers from https://github.com/nothings/stb/blob/master/stb_ds.h
#define STB_DS_IMPLEMENTATION
#include "../include/stb_ds.h"


//SDL3 from https://wiki.libsdl.org/SDL3/FrontPage
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <math.h>
#include <stddef.h>   
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>
#include <limits.h>

//json reader - json.h from https://github.com/sheredom/json.h
#include "../include/json.h"

//sdl version printing function
void print_sdl_versions();

//init function definitions
SDL_AppResult init_sdl(char *assetPath);
SDL_AppResult init_ttf();
SDL_AppResult init_audio(MIX_Mixer **mixer);
SDL_AppResult init_graphics(SDL_GPUDevice **dev,SDL_Window **win, int width, int height,char *title,bool fullScreen);

//this function wraps the above init function into one call
SDL_AppResult core_init(SDL_GPUDevice **dev,SDL_Window **win, int width, int height,char *title,MIX_Mixer **mixer,char *assetPath,float popupFontScale,bool fullScreen);
void core_free(SDL_GPUDevice *dev,SDL_Window *win);

char WINDOW_TITLE[256];  
int WINDOW_WIDTH =  1280;
int WINDOW_HEIGHT = 720;

const char* BasePath=NULL;
const char* AssetPath=NULL;

//core stuff in order
#include "framerate.c"
#include "video_util.c"
#include "texture/texture.c"
#include "filesys/filesys.c"
#include "loaders.c"
#include "atlas/texAtlas.c"
#include "cg2d/cg2d.c"
#include "popupmessage/popupmesage.c"
#include "input/input.c"



SDL_AppResult core_init(SDL_GPUDevice **dev,SDL_Window **win, int width, int height,char *title,MIX_Mixer **mixer,char *assetPath,float popupFontScale,bool fullScreen){
	 //init stuff
    if(init_sdl(assetPath)==SDL_APP_FAILURE) return SDL_APP_FAILURE;
    if(init_ttf()==SDL_APP_FAILURE) return SDL_APP_FAILURE;
    if(init_audio(mixer)==SDL_APP_FAILURE) return SDL_APP_FAILURE;
    //this opens a window and sets up the gpu device for it.
    if(init_graphics(dev,win,width,height,title,fullScreen)==SDL_APP_FAILURE) return SDL_APP_FAILURE;
    //input plays sounds when you connect/disconnect controllers so send it the mixer handle
    //input adds a mouse and a keyboard controller by default
    if(input_init(*mixer)==SDL_APP_FAILURE) return SDL_APP_FAILURE;
    cg_popup_message_init(popupFontScale);
    init_framerate();
    return SDL_APP_CONTINUE;
}

void core_free(SDL_GPUDevice *dev,SDL_Window *win){
    input_free();
    popup_messages_free();
    SDL_ReleaseWindowFromGPUDevice(dev, win);
    SDL_Log("destroy window\n");
    SDL_DestroyWindow(win);
    SDL_Log("destroy device\n");
    SDL_DestroyGPUDevice(dev);
    SDL_Log("free ttf\n");
    TTF_Quit();
    SDL_Log("quit sdl\n");
    SDL_Quit();

}



void print_sdl_versions(){
	const int compiled = SDL_VERSION;  /* hardcoded number from SDL headers */
	const int linked = SDL_GetVersion();  /* reported by linked SDL library */
	SDL_Log("We compiled against SDL version %d.%d.%d ...\n",
        SDL_VERSIONNUM_MAJOR(compiled),
        SDL_VERSIONNUM_MINOR(compiled),
        SDL_VERSIONNUM_MICRO(compiled));

	SDL_Log("But we are linking against SDL version %d.%d.%d.\n",
        SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked),
        SDL_VERSIONNUM_MICRO(linked));
}




SDL_AppResult init_sdl(char *assetPath){
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_HAPTIC)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    BasePath = SDL_GetBasePath();
    AssetPath= assetPath;

    print_sdl_versions();
    return SDL_APP_CONTINUE;
}

SDL_AppResult init_ttf(){
	if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult init_audio(MIX_Mixer **mixer){
	if (!MIX_Init()) {
        SDL_Log("Couldn't init SDL_mixer library: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_Mixer *mix;
     mix = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mix) {
        SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    *mixer=mix;
    return SDL_APP_CONTINUE;
}

SDL_AppResult init_graphics(SDL_GPUDevice **dev,SDL_Window **win, int width, int height,char *title,bool fullScreen){
	
 	SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY; 
    SDL_DisplayMode *found=NULL;

    SDL_strlcpy(WINDOW_TITLE,title,sizeof(WINDOW_TITLE));
    WINDOW_WIDTH=width;
    WINDOW_HEIGHT=height;


    if(video_init()==SDL_APP_FAILURE){return SDL_APP_FAILURE;}

    if(WINDOW_WIDTH==0 || WINDOW_HEIGHT==0){
        SDL_Log("display resolution unset, calculating window size...");
        WINDOW_WIDTH=video_largest_window->w;
        WINDOW_HEIGHT=video_largest_window->h;
        video_current_mode=video_largest_window;
    }else{
        
        found=video_find_mode(WINDOW_WIDTH,WINDOW_HEIGHT);
        WINDOW_WIDTH=found->w;
        WINDOW_HEIGHT=found->h;
        video_current_mode=found;
        SDL_Log("using closest matched mode %d x %d ' %.3f\n",found->w,found->h,found->refresh_rate);
    }

    if(fullScreen==true){
        //flags |= SDL_WINDOW_FULLSCREEN;
    //if it's an excluse mode - i.e the screen res will actually change, use the current
        //mode set above
        if(video_fullscreen_is_exclusive==true){
            WINDOW_WIDTH=video_current_mode->w;
            WINDOW_HEIGHT=video_current_mode->h;
            flags = flags & ~SDL_WINDOW_HIGH_PIXEL_DENSITY;
    //otherwise resize to desktop dimensions and open in non-exclusive mode
        }else{
            WINDOW_WIDTH=video_desktop_mode->w;
            WINDOW_HEIGHT=video_desktop_mode->h;
            video_current_mode=video_desktop_mode;
            SDL_Log("opening fullscreen window at desktop size %d x %d \n",video_desktop_mode->w,video_desktop_mode->h);
        }
    }else{
        SDL_Log("opening %d * %d window: %s\n",WINDOW_WIDTH,WINDOW_HEIGHT, WINDOW_TITLE);
    }


    SDL_GPUDevice *device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_MSL,
        true,   /* debug mode */
        NULL    /* no backend preference */
    );
    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Log("GPU backend: %s", SDL_GetGPUDeviceDriver(device));



    SDL_Window *window = SDL_CreateWindow(
    WINDOW_TITLE, WINDOW_WIDTH,WINDOW_HEIGHT,  flags);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        return SDL_APP_FAILURE;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_DestroyGPUDevice(device);
        return SDL_APP_FAILURE;
    }

    
    SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_VSYNC;
    if (SDL_WindowSupportsGPUPresentMode(
        device,
        window,
        SDL_GPU_PRESENTMODE_IMMEDIATE
    )) {
        SDL_Log("supports immediate mode\n");
        presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    }
    else if (SDL_WindowSupportsGPUPresentMode(
        device,
        window,
        SDL_GPU_PRESENTMODE_MAILBOX
    )) {
        SDL_Log("supports mailbox mode\n");
        presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
    }

    SDL_SetGPUSwapchainParameters(
        device,
        window,
        SDL_WindowSupportsGPUSwapchainComposition(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR) ? SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR:SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        presentMode
    );
////////////////////////////////
    if(fullScreen==true){
        if(video_fullscreen_is_exclusive==true){
            WINDOW_WIDTH=video_current_mode->w;
            WINDOW_HEIGHT=video_current_mode->h;
            SDL_SetWindowFullscreenMode(window, video_current_mode);
            SDL_Log("opening fullscreen window at size %d x %d \n",video_current_mode->w,video_current_mode->h);

        }
        SDL_SetWindowFullscreen(window, true);
        int ww=0,wh=0;
        SDL_GetWindowSize(window, &ww, &wh);
        SDL_Log("window is %d x %d\n",ww,wh);
    }




    video_display_scale= SDL_GetWindowDisplayScale(window);
    SDL_Log("display scale is %f\n",video_display_scale);

    *dev=device;
    *win=window;
    return SDL_APP_CONTINUE;
}