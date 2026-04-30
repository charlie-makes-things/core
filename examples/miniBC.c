
#include "../core/core.c"

char *app_name="Mini Bullet Candy";
char *version_no="1.0";
char *your_name="com.charlie.core";

//game states
typedef enum game_state{
    STATE_TITLE,
    STATE_MENU,
    STATE_GAME,
    STATE_GAME_PAUSED,
    STATE_GAME_OVER,
    STATE_SCORE_ENTRY,
    STATE_SCORE_DISPLAY,
    STATE_CREDITS,
    STATE_QUIT
}game_state;

//the game state
static game_state gameState=STATE_TITLE;

//a cg2d struct
static cg2d_t c2d;

//texture atlas file
static texAtlas atlas;

//game font
static cg2d_font *gameFont;

//some global images
cg2d_image *redImage=NULL;//a red blob
cg2d_image *logoImage=NULL;//the game logo
cg2d_image *blueflatImage=NULL;//this is the player ship image

//cg2d layers to draw to
static int fontLayer;
static int spriteLayer;
static int effectsLayer;
static int overlayLayer;

//textures to render to. one texture holds what is being rendered
//this frame, and the other holds a copy of the previous frame's
//data. This is used for the 'feedback' effect in the background
static cg_texture renderTex;
static cg_texture renderTex2;
//images for the two textures
static cg2d_image *renderTexImage;
static cg2d_image *renderTexImage2;
//layers for the two texture images
static int renderTargetImageLayer;
static int renderTargetImageLayer2;

//virtual resolution. this sets an internal dimension for pixel resolution.
//to your code, the window will always be 1280 x 720 regardless of window size
static int virtualWidth=1280;
static int virtualHeight=720;

typedef struct app_state{    
    SDL_Window* Window;
    SDL_GPUDevice* Device;   
    MIX_Mixer *mixer;   
}app_state;

#include "miniBC/types.c"
#include "miniBC/loadAudio.c"

#include "miniBC/title.c"


int timer=0;
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
    
    //set the appstate
    *appstate = state;

    // Create and load the default cg2d shaders
    SDL_GPUShader* vertexShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.vert", 0, 1, 1, 0); if (vertexShader == NULL) { SDL_Log("Failed to create vertex shader!");return SDL_APP_FAILURE;}
    SDL_GPUShader* fragmentShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.frag", 1, 1, 0, 0);if (fragmentShader == NULL) {SDL_Log("Failed to create fragment shader!");return SDL_APP_FAILURE;}   

    //init cg2d
    cg2d_init(&c2d,state->Window,state->Device, WINDOW_WIDTH,WINDOW_HEIGHT);
    //set the virtual resolution
    cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);

    //load texture atlas
    if(atlas_init(&atlas,"data/bc26Atlas.json","images/",state->Device,ATLAS_FILTER_NEAREST)==1){SDL_Log("failed to load altas\n"); return SDL_APP_FAILURE; }   
    //load the font
    gameFont=cg2d_load_image_font(&c2d,"/fonts/roboto/Roboto-Regular.ttf",50,SDL_GPU_FILTER_LINEAR);

    //load global images
    redImage=cg2d_load_atlas_image(&c2d,&atlas,"redblob.png");
    logoImage=cg2d_load_atlas_image(&c2d,&atlas,"logo.png");
    blueflatImage=cg2d_load_atlas_image(&c2d,&atlas,"blueflat.png");
    

    //set some images for controllers connecting and disconnecting. these can
    //also be NULL (which is the default) if you dont care.
    input_set_images(cg2d_load_atlas_image(&c2d,&atlas,"tick.png"),cg2d_load_atlas_image(&c2d,&atlas,"cross.png"));

    //setup layers
    spriteLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    overlayLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    fontLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&gameFont->tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    effectsLayer=cg2d_add_layer(&c2d,CG2D_LIGHTBLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    
    //generate the render textures
    cg_texture_gen_2d(&renderTex,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_LINEAR,state->Device,state->Window);
    cg_texture_gen_2d(&renderTex2,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_LINEAR,state->Device,state->Window);
    //load the render textures into cg2d images
    renderTexImage=cg2d_create_image(&c2d,"render target",&renderTex,virtualWidth,virtualHeight);
    renderTexImage2=cg2d_create_image(&c2d,"render target 2",&renderTex2,virtualWidth,virtualHeight);
    //create some layer to draw the trender texture to
    renderTargetImageLayer=cg2d_add_layer(&c2d,CG2D_LIGHTBLEND,&renderTex,vertexShader,fragmentShader,SDL_GPU_LOADOP_CLEAR);
    renderTargetImageLayer2=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&renderTex2,vertexShader,fragmentShader,SDL_GPU_LOADOP_CLEAR);
    

    //load audio
    load_audio(state->mixer);

    //init states
    title_init();

    //add a popup message to say hi
    popup_messags_add_message("Mini Bullet Candy Example","By Charlie in 2026",&c2d, blueflatImage,0.15);



    
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
    }else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key==SDLK_SPACE) {
        //eturn SDL_APP_SUCCESS;
         popup_messags_add_message("Mini Bullet Candy Example","By Charlie in 2026",&c2d, blueflatImage,0.15);

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
            cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);
            cg_destroy_texture(&renderTex,state->Device);
            cg_destroy_texture(&renderTex2,state->Device);
            cg_texture_gen_2d(&renderTex,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_NEAREST,state->Device,state->Window);
            cg_texture_gen_2d(&renderTex2,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_NEAREST,state->Device,state->Window);
            cg2d_delete_image(&c2d,renderTexImage);
            cg2d_delete_image(&c2d,renderTexImage2);
            renderTexImage=cg2d_create_image(&c2d,"render target",&renderTex,virtualWidth,virtualHeight);
            renderTexImage2=cg2d_create_image(&c2d,"render target",&renderTex2,virtualWidth,virtualHeight);
            cg2d_set_layer_texture(&c2d, renderTargetImageLayer, &renderTex);
            cg2d_set_layer_texture(&c2d, renderTargetImageLayer2, &renderTex2);

        }else{
            video_set_fullscreen_desktop(state->Window);
            cg2d_set_viewport(&c2d,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
            cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);
            cg_destroy_texture(&renderTex,state->Device);
            cg_destroy_texture(&renderTex2,state->Device);
            cg_texture_gen_2d(&renderTex,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_NEAREST,state->Device,state->Window);
            cg_texture_gen_2d(&renderTex2,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GPU_FILTER_NEAREST,state->Device,state->Window);
            cg2d_delete_image(&c2d,renderTexImage);
            cg2d_delete_image(&c2d,renderTexImage2);
            renderTexImage=cg2d_create_image(&c2d,"render target",&renderTex,virtualWidth,virtualHeight);
            renderTexImage2=cg2d_create_image(&c2d,"render target",&renderTex2,virtualWidth,virtualHeight);
            cg2d_set_layer_texture(&c2d, renderTargetImageLayer, &renderTex);
            cg2d_set_layer_texture(&c2d, renderTargetImageLayer2, &renderTex2);

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
   
    timer++;
    switch(gameState){

        case STATE_TITLE:
            //change the game state if needed. 
            gameState=title_update(input_get_active_controller());
            break;
        case STATE_MENU:

            break;
        case STATE_CREDITS:

            break;
        case STATE_SCORE_DISPLAY:

            break;
        case STATE_SCORE_ENTRY:

            break;
        case STATE_GAME_OVER:

            break;
        case STATE_GAME:

            break;
        case STATE_GAME_PAUSED:

            break;
        case STATE_QUIT:
            return SDL_APP_SUCCESS;
            break;
        default:
            SDL_Log("oops! we should be here! returning failure\n");
            return SDL_APP_FAILURE;  

    }


    ////////////////////////////////////////
    //begin drawing
    ////////////////////////////////////////
    //get a command buffer and swapchain texture to draw with/to
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(state->Device);if (cmdBuf == NULL){ SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());return SDL_APP_FAILURE;}
    SDL_GPUTexture* swapchainTexture;if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, state->Window, &swapchainTexture, NULL, NULL)) { SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());return SDL_APP_FAILURE;}
    //if the swapchain texture exists, we're good to start drawing with cg2d
    if (swapchainTexture != NULL)
    {
        //clear the layers of the last frames data
        cg2d_clear_layer(&c2d,spriteLayer);
        cg2d_clear_layer(&c2d,overlayLayer);
        cg2d_clear_layer(&c2d,effectsLayer);
        cg2d_clear_layer(&c2d,fontLayer);
        cg2d_clear_layer(&c2d,renderTargetImageLayer);
        cg2d_clear_layer(&c2d,renderTargetImageLayer2);
        //copy the some of the last rendered fame to another so it can be used to make
        //a feedback/hall of mirrons effect.
        cg_texture_copy_texture(cmdBuf,state->Window,&renderTex, &renderTex2);

	    ///////////////////////////////////////
        //draw stuff into layers here
        ///////////////////////////////////////
        switch(gameState){

            case STATE_TITLE:
                //title_draw is in miniBC/title.c
                title_draw(timer);
                break;
            case STATE_MENU:

                break;
            case STATE_CREDITS:

                break;
            case STATE_SCORE_DISPLAY:

                break;
            case STATE_SCORE_ENTRY:

                break;
            case STATE_GAME_OVER:

                break;
            case STATE_GAME:

                break;
            case STATE_GAME_PAUSED:

                break;
            case STATE_QUIT:
                return SDL_APP_SUCCESS;
                break;
            default:
                SDL_Log("oops! we should be here! returning failure\n");
                return SDL_APP_FAILURE;  

        }





        //draw any popup messages - best to draw this at the end so they appear 
        //over everythings else, for convenience we'll draw any images to the blob layer
        popup_messages_draw(&c2d, overlayLayer,fontLayer);

        //////////////////////////////////////
        //draw layers here
        //////////////////////////////////////

        //render the previous frame with transformations to render texture     
       // cg2d_set_layer_clear(&c2d,renderTargetImageLayer2,true);       
        cg2d_draw_layer(&c2d,renderTargetImageLayer2,cmdBuf,renderTex.tex);     
           
        //render this frames sprite data to render texture
       // cg2d_set_layer_clear(&c2d,spriteLayer,false);
        cg2d_draw_layer(&c2d,spriteLayer,cmdBuf,renderTex.tex);

        //render this frames sprite data to render texture
       // cg2d_set_layer_clear(&c2d,effectsLayer,false);
        cg2d_draw_layer(&c2d,effectsLayer,cmdBuf,renderTex.tex);


        //render the render texture to the swapchain texture
      //  cg2d_set_layer_clear(&c2d,renderTargetImageLayer,true);       
        cg2d_draw_layer(&c2d,renderTargetImageLayer,cmdBuf,swapchainTexture);     
        
        ///draw the overlay layer to the swapchain texture
      //  cg2d_set_layer_clear(&c2d,overlayLayer,false);
        cg2d_draw_layer(&c2d,overlayLayer,cmdBuf,swapchainTexture);

        ///draw the font layer the swapchain texture
      //  cg2d_set_layer_clear(&c2d,fontLayer,false);
        cg2d_draw_layer(&c2d,fontLayer,cmdBuf,swapchainTexture);





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

    cg2d_free(&c2d);
    atlas_free(&atlas,state->Device);



    core_free(state->Device,state->Window);    
    SDL_Log("free state\n");
    SDL_free(state);   
}