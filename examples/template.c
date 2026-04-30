
#include "../core/core.c"

char *app_name="Core Template";
char *version_no="1.0";
char *your_name="com.charlie.core";

//a cg2d struct
static cg2d_t c2d;

//virtual resolution. this sets an internal dimension for pixel resolution.
//to your code, the window will always be 1280 x 720 regardless of window size
static int virtualWidth=1280;
static int virtualHeight=720;

typedef struct app_state{    
    SDL_Window* Window;
    SDL_GPUDevice* Device;   
    MIX_Mixer *mixer;   
}app_state;

//init function
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    //create state
	app_state *state = SDL_calloc(1, sizeof(app_state));
    //set metadata
    SDL_SetAppMetadata(app_name, version_no, your_name);
    //init core
    core_init(&state->Device,&state->Window, 0, 0,app_name,&state->mixer,"assets/",3.0,false);
    //set framerate.
    set_framerate(60);
    
    // Create and load the default cg2d shaders
    SDL_GPUShader* vertexShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.vert", 0, 1, 1, 0); if (vertexShader == NULL) { SDL_Log("Failed to create vertex shader!");return SDL_APP_FAILURE;}
    SDL_GPUShader* fragmentShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.frag", 1, 1, 0, 0);if (fragmentShader == NULL) {SDL_Log("Failed to create fragment shader!");return SDL_APP_FAILURE;}   

    //init cg2d
    cg2d_init(&c2d,state->Window,state->Device, WINDOW_WIDTH,WINDOW_HEIGHT);
    //set the virtual resolution
    cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);

    //set some images for controllers connecting and disconnecting. these can
    //also be NULL (which is the default) if you dont care.
    //input_set_images(cg2d_load_atlas_image(&c2d,&atlas,"tick.png"),cg2d_load_atlas_image(&c2d,&atlas,"cross.png"));


    //set the appstate
    *appstate = state;
    //carry on!
	return SDL_APP_CONTINUE;
}


//update any events
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    
    app_state *state = (app_state *)appstate;

    //input update events handles controllers, mouse and keyboard stuff, 
    //handles controllers etc. being added and removed
    //and updates things there. you can use the input_xxxx functions
    //to access them. see core/input/input.c or the input example
    int controllerChangeIndex =0;
    input_update_events(event,&controllerChangeIndex,&c2d);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
    
    } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
                       
    //handle switching between fullscreen and windowed modes               
    }else if ((event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F11) ||
              (event->type == SDL_EVENT_KEY_DOWN && (event->key.mod ==SDL_KMOD_LALT && event->key.key==SDLK_RETURN )) ){
        if(video_is_fullscreen(state->Window)==true){
            video_set_windowed(state->Window);
            cg2d_set_viewport(&c2d,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
            cg2d_set_virtual_resolution(&c2d,virtualWidth, virtualHeight);
        }else{
            video_set_fullscreen_desktop(state->Window);
            cg2d_set_viewport(&c2d,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
            cg2d_set_virtual_resolution(&c2d,virtualWidth, virtualHeight);
        }
        SDL_Log("video mode change: %d x %d - vx %d vy %d\n",WINDOW_WIDTH,WINDOW_HEIGHT,cg2d_get_virtual_width(&c2d),cg2d_get_virtual_height(&c2d));
        
    } 
     return SDL_APP_CONTINUE;
}



//main loop
SDL_AppResult SDL_AppIterate(void *appstate){
    
    app_state *state = (app_state *)appstate;
    //update input - call every frame
    input_update(&c2d);
    //update popup messages
    popup_messages_update(cg2d_get_virtual_width(&c2d),cg2d_get_virtual_height(&c2d));

    ////////////////////////////////////////
    //here is a good place to update things
    ////////////////////////////////////////



    ////////////////////////////////////////
    //begin drawing
    ////////////////////////////////////////
    //get a command buffer and swapchain texture to draw with/to
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(state->Device);if (cmdBuf == NULL){ SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());return SDL_APP_FAILURE;}
    SDL_GPUTexture* swapchainTexture;if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, state->Window, &swapchainTexture, NULL, NULL)) { SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());return SDL_APP_FAILURE;}
    //if the swapchain texture exists, we're good to start drawing with cg2d
    if (swapchainTexture != NULL)
    {
	    ///////////////////////////////////////
        //draw stuff into layers here
        ///////////////////////////////////////
        





        //draw any popup messages - best to draw this at the end so they appear 
        //over everythings else, for convenience we'll draw any images to the blob layer
        //popup_messages_draw(&c2d, spriteLayer,fontLayer);

        //////////////////////////////////////
        //draw layers here
        //////////////////////////////////////







	}
    //submit the command buffer to render to the screen
    SDL_SubmitGPUCommandBuffer(cmdBuf);
    //delay to match framerate
    framerate_delay();
    //carry on!
    return SDL_APP_CONTINUE;
}

//free stuff and exit
void SDL_AppQuit(void *appstate, SDL_AppResult result){
	app_state *state = (app_state *)appstate;
    core_free(state->Device,state->Window);    
    SDL_Log("free state\n");
    SDL_free(state);   
}