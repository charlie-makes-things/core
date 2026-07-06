//Mini Bullet Candy, by charlie 2011 - 2026

//an updated port of the original Mini Bullet Candy web game from
//2011 to demonstrate the core framework.

//use the build examples script to compile.

//this is intended to serve as an example of how to use the core framework
//functions, not as an example of how to structure your game code. Probably
//don't do it like this ;P

//while the images and sound effects used are public domain, and yours to do as you
//please with, the music is not so pleasse don't release anything publicly using
//the music files. If you'd like to use them in your release, please contact John
//and ask for permission. His website is here https://ellisvlad.co.nr/




#include "../core/core.c"

char *app_name="Mini Bullet Candy";
char *version_no="2.0";
char *your_name="com.charlie.core";

//set to true to display collision circles
bool d_showCollisionCircles=false;
//set to true to allow player to die in game.
bool d_playerCollisions=true;
//set to true to trigger pause mode
bool g_pressed_pause=false;
//make sure we don't immediately unpause
// int g_pause_ticks=0;

//game states
typedef enum game_state{
    STATE_TITLE,
    STATE_MENU,
    STATE_GAME,
    STATE_GAME_PAUSED,
    STATE_SCORE_ENTRY,
    STATE_SCORE_DISPLAY,
    STATE_ABOUT,
    STATE_QUIT,
    STATE_SETTINGS,
    STATE_GAME_TRANSITION
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
cg2d_image *blobImage=NULL;
cg2d_image *redImage=NULL;//a red blob
cg2d_image *greenImage=NULL;
cg2d_image *logoImage=NULL;//the game logo
cg2d_image *blueflatImage=NULL;//this is the player ship image
cg2d_image *ringImage=NULL;//a ring image
cg2d_image *bulletImage=NULL;//player bullet image
cg2d_image *starImage=NULL;
cg2d_image *zapImage=NULL;
cg2d_image *enemyBulletGreenImage=NULL;
cg2d_image *rescuableImage=NULL;
cg2d_image *rescuableBlueImage=NULL;
cg2d_image *rescuableRedImage=NULL;
cg2d_image *horseImage=NULL;
cg2d_image *sparkStarImage=NULL;

cg2d_image *grunt1Image=NULL;
cg2d_image *grunt2Image=NULL;
cg2d_image *grunt3Image=NULL;
cg2d_image *grunt4Image=NULL;
cg2d_image *grunt5Image=NULL;
cg2d_image *grunt6Image=NULL;
cg2d_image *grunt7Image=NULL;
cg2d_image *grunt8Image=NULL;
cg2d_image *spikerImage=NULL;
cg2d_image *shurikenImage=NULL;
cg2d_image *enemyBulletImage=NULL;
cg2d_image *rocketImage=NULL;
cg2d_image *fourwayImage=NULL;
cg2d_image *spinnerImage=NULL;
cg2d_image *crawlerImage1=NULL;
cg2d_image *crawlerImage2=NULL;
cg2d_image *tripleCrawlerImage1=NULL;
cg2d_image *tripleCrawlerImage2=NULL;
cg2d_image *mineImage=NULL;
cg2d_image *orbiterImage=NULL;
cg2d_image *generatorImage=NULL;
cg2d_image *electrodeImage1=NULL;
cg2d_image *electrodeImage2=NULL;
cg2d_image *evilLauncherImage=NULL;
cg2d_image *harvesterImage=NULL;
cg2d_image *tankImage=NULL;
cg2d_image *spreaderImage=NULL;
cg2d_image *threewayImage=NULL;
cg2d_image *ringshotImage=NULL;
cg2d_image *tripleRingImage=NULL;
cg2d_image *boss1Image=NULL;
cg2d_image *boss2Image=NULL;

//cg2d layers to draw to
static int fontLayer;
static int spriteLayer;
static int effectsLayer;
static int overlayLayer;
static int hudLayer;

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

//virtual resolution. set these values with cg2d_set_virtual_resolution(x,y);
//this sets an internal dimension for screen coordinates.
//this will mean that regardless of the size of your window/fullscreen
//dimensions, virtualWidth will always be the far left of the window, and virtualHeight
//will always be the bottom. 
//this is useful for porting titles with fixed resolutions, such as my own irukandji,
//which was coded strictly for 800*600 resolution. 
static int virtualWidth=1280;
static int virtualHeight=720;

typedef struct app_state{    
    SDL_Window* Window;
    SDL_GPUDevice* Device;   
    MIX_Mixer *mixer;   
}app_state;


//re-size the render textures if the screensize has changed
void regen_textures(void *appstate){
    app_state *state = (app_state *)appstate;
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

//a char to hold the users name if they get a high score.
char userTypedString[32];
int userTypedStringLen=0;
bool textEntryActive=false;


#include "miniBC/types.c"//some structs and globals used by the game
#include "miniBC/audio.c"//functions for audio using sdl_mixer

//some audio tracks 
cg_audio_track menuMusic;
cg_audio_track gameMusic;
cg_audio_track menuClickSFX;
cg_audio_track menuSelectSFX;
cg_audio_track menuBackSFX;
cg_audio_track playerSpawn1SFX;
cg_audio_track playerSpawn2SFX;
cg_audio_track playerSpawn3SFX;
cg_audio_track zapSFX;
cg_audio_track playerShotSFX;

cg_audio_track hitSFX;
cg_audio_track deathSFX;
cg_audio_track electricSFX;
cg_audio_track completeSFX;
cg_audio_track enemyShotSFX;
cg_audio_track gameOverSFX;
cg_audio_track oneUpSFX;
cg_audio_track perfectSFX;
cg_audio_track scoreUpSFX;
cg_audio_track threewaySFX;

//position and scale of the mini bullet candy logo, changed by some of the
//menu screens
float logoPosX,logoPosY;
float logoTargetX,logoTargetY;
float logoScale, logoTargetScale;

#include "miniBC/math.c"//some math functions
#include "miniBC/functions.c"
#include "miniBC/particle.c"
#include "miniBC/message.c"
#include "miniBC/bonus.c"
#include "miniBC/bullet.c"
#include "miniBC/player.c"
#include "miniBC/enemy.c"

#include "miniBC/menu_prototypes.c"
#include "miniBC/about.c"//about screen functions
#include "miniBC/settings.c"//settings2 screen display functions
#include "miniBC/pause.c"//pause menu2
#include "miniBC/scoreEntry.c"//score entry screen
#include "miniBC/transition.c"//screen transition
#include "miniBC/menu.c"//menu screen functions
#include "miniBC/title.c"//title screen functions
#include "miniBC/scores.c"//score display screen function
#include "miniBC/starfield.c"

#include "miniBC/enemyFunc.c"//functions to spawn enemies
#include "miniBC/level.c"//data for the game levels
#include "miniBC/game.c"//game loop



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
    if(atlas_init(&atlas,"data/bc26Atlas.json","images/",state->Device,ATLAS_FILTER_LINEAR)==1){SDL_Log("failed to load altas\n"); return SDL_APP_FAILURE; }   
   
    //load the font
    gameFont=cg2d_load_image_font(&c2d,"/fonts/roboto/Roboto-Regular.ttf",50,SDL_GPU_FILTER_LINEAR);

    //load global images
    blobImage=cg2d_load_atlas_image(&c2d,&atlas,"blob.png");
    redImage=cg2d_load_atlas_image(&c2d,&atlas,"redblob.png");
    greenImage=cg2d_load_atlas_image(&c2d,&atlas,"blobgreen.png");
    logoImage=cg2d_load_atlas_image(&c2d,&atlas,"logo.png");
    blueflatImage=cg2d_load_atlas_image(&c2d,&atlas,"blueflat.png");
    ringImage=cg2d_load_atlas_image(&c2d,&atlas,"ringbullet.png");
    bulletImage=cg2d_load_atlas_image(&c2d,&atlas,"bullet.png");
    starImage=cg2d_load_atlas_image(&c2d,&atlas,"star.png");
    zapImage=cg2d_load_atlas_image(&c2d,&atlas,"zap.png");
    enemyBulletGreenImage=cg2d_load_atlas_image(&c2d,&atlas,"enemybulletgreen.png");
    rescuableImage=cg2d_load_atlas_image(&c2d,&atlas,"rescuable.png");
    rescuableBlueImage=cg2d_load_atlas_image(&c2d,&atlas,"rescuableblue.png");
    rescuableRedImage=cg2d_load_atlas_image(&c2d,&atlas,"rescuablered.png");
    horseImage=cg2d_load_atlas_image(&c2d,&atlas,"horse.png");
    sparkStarImage=cg2d_load_atlas_image(&c2d,&atlas,"sparkstar.png");

    grunt1Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt1.png");
    grunt2Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt2.png");
    grunt3Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt3.png");
    grunt4Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt4.png");
    grunt5Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt5.png");
    grunt6Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt6.png");
    grunt7Image=cg2d_load_atlas_image(&c2d,&atlas,"grunt7.png");
    grunt8Image=cg2d_load_atlas_image(&c2d,&atlas,"launcher.png");
    spikerImage=cg2d_load_atlas_image(&c2d,&atlas,"spiker.png");
    shurikenImage=cg2d_load_atlas_image(&c2d,&atlas,"shuriken.png");
    enemyBulletImage=cg2d_load_atlas_image(&c2d,&atlas,"enemybullet.png");
    rocketImage=cg2d_load_atlas_image(&c2d,&atlas,"rocket.png");
    fourwayImage=cg2d_load_atlas_image(&c2d,&atlas,"fourway.png");
    spinnerImage=cg2d_load_atlas_image(&c2d,&atlas,"spinner.png");
    crawlerImage1=cg2d_load_atlas_image(&c2d,&atlas,"crawler1.png");
    crawlerImage2=cg2d_load_atlas_image(&c2d,&atlas,"crawler2.png");
    tripleCrawlerImage1=cg2d_load_atlas_image(&c2d,&atlas,"triplecrawler1.png");
    tripleCrawlerImage2=cg2d_load_atlas_image(&c2d,&atlas,"triplecrawler2.png");
    mineImage=cg2d_load_atlas_image(&c2d,&atlas,"spikyBall.png");
    orbiterImage=cg2d_load_atlas_image(&c2d,&atlas,"orbiter.png");
    generatorImage=cg2d_load_atlas_image(&c2d,&atlas,"generator.png");
    electrodeImage1=cg2d_load_atlas_image(&c2d,&atlas,"electrode1.png");
    electrodeImage2=cg2d_load_atlas_image(&c2d,&atlas,"electrode2.png");
    evilLauncherImage=cg2d_load_atlas_image(&c2d,&atlas,"invlauncher.png");
    harvesterImage=cg2d_load_atlas_image(&c2d,&atlas,"grunt5.png");
    tankImage=cg2d_load_atlas_image(&c2d,&atlas,"tank.png");
    spreaderImage=cg2d_load_atlas_image(&c2d,&atlas,"spreader.png");
    threewayImage=cg2d_load_atlas_image(&c2d,&atlas,"threeway.png");
    ringshotImage=cg2d_load_atlas_image(&c2d,&atlas,"ringshot.png");
    tripleRingImage=cg2d_load_atlas_image(&c2d,&atlas,"triplering.png");
    boss1Image=cg2d_load_atlas_image(&c2d,&atlas,"49boss.png");
    boss2Image=cg2d_load_atlas_image(&c2d,&atlas,"bigboss.png");
    
    //set some images for controllers connecting and disconnecting. these can
    //also be NULL (which is the default) if you dont care.
    input_set_images(cg2d_load_atlas_image(&c2d,&atlas,"tick.png"),cg2d_load_atlas_image(&c2d,&atlas,"cross.png"));

    //setup layers
    spriteLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    overlayLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    fontLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&gameFont->tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    effectsLayer=cg2d_add_layer(&c2d,CG2D_LIGHTBLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    hudLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);
    
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
    load_audio(state->mixer,"audio/Circle.mp3",&menuMusic,AUDIO_TYPE_MUSIC);
    load_audio(state->mixer,"audio/TowerOfOne.ogg",&gameMusic,AUDIO_TYPE_MUSIC);
    load_audio(state->mixer,"audio/menuback.ogg",&menuBackSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/menuclick.ogg",&menuClickSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/SineThrow1.wav",&menuSelectSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/playerspawn1.ogg",&playerSpawn1SFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/playerspawn2.ogg",&playerSpawn2SFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/playerspawn3.ogg",&playerSpawn3SFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/zap2.ogg",&zapSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/bshot.ogg",&playerShotSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/hit.ogg",&hitSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/destroy11.ogg",&deathSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/electric.ogg",&electricSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/gamecomplete.ogg",&completeSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/gover.ogg",&gameOverSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/oneup.ogg",&oneUpSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/perfect.ogg",&perfectSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/scoreup.ogg",&scoreUpSFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/threeway.ogg",&threewaySFX,AUDIO_TYPE_SFX);
    load_audio(state->mixer,"audio/eshot.ogg",&enemyShotSFX,AUDIO_TYPE_SFX);

    //init states
    title_init(true);

    //add a popup message to say hi
    popup_messags_add_message("Mini Bullet Candy Example","By Charlie in 2026",&c2d, blueflatImage,0.15);

    //add some scores to the high score table
    arrpush(highScoreTable,((high_score_entry){"Peas",100000}));
    arrpush(highScoreTable,((high_score_entry){"Turnips",99999}));
    arrpush(highScoreTable,((high_score_entry){"Spuds",9999}));
    arrpush(highScoreTable,((high_score_entry){"Carrots",999}));
    arrpush(highScoreTable,((high_score_entry){"Broccoli",12345}));
    arrpush(highScoreTable,((high_score_entry){"Parsnips",54321}));
    arrpush(highScoreTable,((high_score_entry){"Cauliflower",11223}));
    arrpush(highScoreTable,((high_score_entry){"Runner Bean",89123}));
    arrpush(highScoreTable,((high_score_entry){"Sweetcorn",44444}));
    arrpush(highScoreTable,((high_score_entry){"Swede",80085}));
    SDL_qsort(highScoreTable, arrlen(highScoreTable), sizeof(high_score_entry), high_score_sort);

    //user typed name
    SDL_snprintf(userTypedString,sizeof(userTypedString),"\n");

    //touch devices
    int touchCnt=0;
    SDL_TouchID *touch = SDL_GetTouchDevices(&touchCnt);
    SDL_Log("found %d touch devices\n",touchCnt);
    if(touchCnt>0){
        for(int i=0;i<touchCnt;i++){
            SDL_Log("touch device %d name %s\n",touchCnt,SDL_GetTouchDeviceName(*touch));
            SDL_TouchDeviceType ttype=SDL_GetTouchDeviceType(*touch);
            if(ttype==SDL_TOUCH_DEVICE_DIRECT){
                SDL_Log("touch screen with window-relative coordinates\n");
            }else if(ttype==SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE){
                SDL_Log("trackpad with absolute device coordinates\n");
            }else if(ttype==SDL_TOUCH_DEVICE_INDIRECT_RELATIVE){
                SDL_Log("trackpad with screen cursor-relative coordinates\n");
            }else{
                SDL_Log("touch type invalid\n");
            }
        }
    }

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

    //handle quitting
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    //gamepad ading and removing is handled in input.c, but you can add
    //your own extra stuff below.
    } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
    
    } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
                       
    //handle switching between fullscreen and windowed modes, use F11, alt-enter or cmd-ctrl-f
    }else if ((event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F11) 
              ||(event->type == SDL_EVENT_KEY_DOWN && (event->key.mod ==SDL_KMOD_LALT && event->key.key==SDLK_RETURN )) 
              ||(event->type == SDL_EVENT_KEY_DOWN && (event->key.mod & (SDL_KMOD_LGUI | SDL_KMOD_LCTRL) && event->key.key==SDLK_F))){
        if(video_is_fullscreen(state->Window)==true){
            video_set_windowed(state->Window);
            cg2d_set_viewport(&c2d,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
            cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);
            //we've resized the screen, so we need to resize the fullscreen texture
            regen_textures(state);
        }else{
            video_set_fullscreen_desktop(state->Window);
            cg2d_set_viewport(&c2d,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
            cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);
            //we've resized the screen, so we need to resize the fullscreen texture
            regen_textures(state);
        }
        SDL_Log("video mode change: %d x %d - vx %d vy %d\n",WINDOW_WIDTH,WINDOW_HEIGHT,cg2d_get_virtual_width(&c2d),cg2d_get_virtual_height(&c2d));
    
    //delete the last typed character if text entry is active.
    }else if(event->type== SDL_EVENT_KEY_DOWN && event->key.key==SDLK_BACKSPACE){
        if(textEntryActive==true){
            if(userTypedStringLen>0){            
                userTypedStringLen--;
            }
        }
    //cancel text entry
    }else if(event->type== SDL_EVENT_KEY_DOWN && event->key.key==SDLK_RETURN){
        //stop text entry if we hit return
        if(textEntryActive==true){
            textEntryActive=false;
        }
    //if text entry is active, add the last type char to the entry string.
    }else if (event->type==SDL_EVENT_TEXT_INPUT){

        if(userTypedStringLen<32){
            char firstChar=  event->text.text[0];
            userTypedString[userTypedStringLen]=firstChar;
            userTypedStringLen++;
        }
    }else if(event->type==SDL_EVENT_KEY_UP && event->key.key==SDLK_ESCAPE){
        if(gameState!=STATE_GAME_PAUSED){
            g_pressed_pause=!g_pressed_pause; 
        }      
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

    //update cued audio
    update_cued_audio();

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
            gameState=menu_update(input_get_active_controller(),timer);
            break;
        case STATE_ABOUT:
            gameState=about_update(input_get_active_controller());
            break;
        case STATE_SCORE_DISPLAY:
            gameState=scores_update(input_get_active_controller());
            break;
        case STATE_GAME_TRANSITION:
            gameState=transition_update(timer,state->Window);
            break;
        case STATE_SCORE_ENTRY:
            gameState=score_entry_update(input_get_active_controller(),state->Window);
            break;
        case STATE_SETTINGS:
            gameState=settings_update(input_get_active_controller());
            break;        
        case STATE_GAME:
            gameState=game_update(input_get_active_controller(),state->Window,timer);
            //handle pause mode
            if(g_pressed_pause==true && gameState==STATE_GAME){
                gameState=STATE_GAME_PAUSED;
                pause_init();
                g_pressed_pause=false;
                SDL_SetWindowRelativeMouseMode(state->Window, false);
            }
            break;
        case STATE_GAME_PAUSED:
            gameState=pause_update(input_get_active_controller(),timer,state->Window);
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
        cg2d_clear_layer(&c2d,hudLayer);
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
                menu_draw(timer);
                break;
            case STATE_ABOUT:
                about_draw(timer);
                break;
            case STATE_SCORE_DISPLAY:
                scores_draw(timer);
                break;
            case STATE_SCORE_ENTRY:
                score_entry_draw(timer);
                break;           
            case STATE_SETTINGS:
                settings_draw(timer,state->Window);
                break;
            case STATE_GAME_TRANSITION:
                transition_draw(timer);
                break;
            case STATE_GAME:
                game_draw(timer,gameState);
                break;
            case STATE_GAME_PAUSED:
                //draw game screen
                    game_draw(timer,gameState);
                //draw pause menu
                    pause_draw(timer,state->Window);
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
        cg2d_draw_layer(&c2d,renderTargetImageLayer2,cmdBuf,renderTex.tex);     
           

        //render this frames sprite data to render texture
        cg2d_draw_layer(&c2d,effectsLayer,cmdBuf,renderTex.tex);

        //render this frames sprite data to render texture
        cg2d_draw_layer(&c2d,spriteLayer,cmdBuf,renderTex.tex);

        //render the render texture to the swapchain texture
        cg2d_draw_layer(&c2d,renderTargetImageLayer,cmdBuf,swapchainTexture);     
                
        //draw the hud layer
        cg2d_draw_layer(&c2d,hudLayer,cmdBuf,swapchainTexture);

        ///draw the font layer the swapchain texture
        cg2d_draw_layer(&c2d,fontLayer,cmdBuf,swapchainTexture);

        ///draw the overlay layer to the swapchain texture
        cg2d_draw_layer(&c2d,overlayLayer,cmdBuf,swapchainTexture);





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

    //free some global lists
    starfield_free();

    if(highScoreTable!=NULL){arrfree(highScoreTable);}
    if(blobs!=NULL){arrfree(blobs);}
    if(particles!=NULL){arrfree(particles);};
    if(player_bullets!=NULL){arrfree(player_bullets);}
    if(enemies!=NULL){arrfree(enemies);}
    if(bonuses!=NULL){arrfree(bonuses);}
    if(messages!=NULL){arrfree(messages);}
    if(cued_SFX!=NULL){arrfree(cued_SFX);}


    //audio free, free any loaded sounds and the list that stores reference to them
    //strictly speaking this is unneccessary as MIX_Quit() deallocates everyhting it
    //creates, but it's probably good practice to tidy up.
    audio_free();

    //free cg2d and the texture atlas
    cg2d_free(&c2d);
    atlas_free(&atlas,state->Device);


    //finally, free core
    core_free(state->Device,state->Window,state->mixer);    
    SDL_Log("free state\n");
    SDL_free(state);   
}