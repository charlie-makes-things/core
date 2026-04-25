/* particle fountain example - by charlie 2026
part of https://github.com/charlie-makes-things/core

This example demonstrates some very basic particles.
hold down the mouse button to spawn particles at the current
mouse x/y. this is not an efficient way of rendering particles
but it serves to show how cg2d can scale, rotate etc. images
and primitives

switch primitive types between images and outline rects with the
space key.

It also sets up core to handle input/popup messages etc

for ease of navigation, the important initialization, update and draw code
is surrounded by lots of ////////////////////////////////////////////
		
*/


//include core files
#include "../core/core.c"

//some SDL state variables
typedef struct app_state{    
    SDL_Window* Window;
    SDL_GPUDevice* Device;   
    MIX_Mixer *mixer;   
}app_state;

/////////////////////////////////////////////////////////////////////////////////////
//global variables
/////////////////////////////////////////////////////////////////////////////////////

//texture atlas struct
texAtlas atlas;

//the cg2d struct.
cg2d_t c2d;

//the blob image we'll use to create the monster
cg2d_image *blobImage;

//a font so we can render text.
cg2d_font *roboFont;

//virtual resolution. this sets an internal dimension for resolution
//independent drawing i.e your code looks the same regardless of the screen
//size
int virtualWidth=1280;
int virtualHeight=720;

//some layers to render into
int sparkLayer;
int fontLayer;

//2d point, we'll use these for the spark coordinates etc.
typedef struct spark{
	float x,y;
	float vx,vy;
	float life;
	int r,g,b;
}spark;

//gravity
float GRAVITY = 0.15f;

//mouse x/y
float mouse_x=0,mouse_y=0;

//how many sparks to spawn per frame. Note that the initial capacity 
//for vertices in a buffer is 10000 as set by CG2D_MAX_VERTICES in cg2d.c
//you can change this number if you need more
int spawnCnt=30;

//rects or images???
bool drawRects =false;

//stretchy buffer to hold sparks
spark *sparks;

//clamp function
float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


//init function
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	app_state *state = SDL_calloc(1, sizeof(app_state));    
    SDL_SetAppMetadata("Particle Fountain Example", "1.0", "charliemakesthings.com");
//////////////////////////////////////////////////////////////////////
//core setup
//////////////////////////////////////////////////////////////////////

    //init core functions. you can set WINDOW_WIDTH etc in core.c
    //make sure you set the asset path parameter. all
    //load functions are relative to this.
    if(core_init(&state->Device,
    			 &state->Window, 
    			 0, 							/*set window width and height to */
    			 0,								/*auto find a good window size*/
    			 "Particle Fountain Example",
    			 &state->mixer,
    			 "assets/",						/*path to assets, relative to the exe directory*/
    			 3.0,							/*scalar for font sizes */
    			 false)==SDL_APP_FAILURE)		/*fullscreen ???*/
    {	SDL_Log("failed to init core.\n");return SDL_APP_FAILURE;    }

    *appstate = state;

//set the framerate to 60
    set_framerate(60);


// Create and load the shaders
    SDL_GPUShader* vertexShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.vert", 0, 1, 1, 0);
    if (vertexShader == NULL) {SDL_Log("Failed to create vertex shader!");return SDL_APP_FAILURE; }

    SDL_GPUShader* fragmentShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.frag", 1, 1, 0, 0);
    if (fragmentShader == NULL) { SDL_Log("Failed to create fragment shader!"); return SDL_APP_FAILURE; }

//load texture atlas
    if(atlas_init(&atlas,"data/bc26Atlas.json","images/",state->Device,ATLAS_FILTER_NEAREST)==1){SDL_Log("failed to load altas\n"); return SDL_APP_FAILURE; }   
  
 //initialise cg2d
    cg2d_init(&c2d,state->Window,state->Device, WINDOW_WIDTH,WINDOW_HEIGHT);
    cg2d_set_virtual_resolution(&c2d,virtualWidth,virtualHeight);
 //get the image
    blobImage=cg2d_load_atlas_image(&c2d,&atlas,"blob.png");
 //load the font for popup messages
    roboFont=cg2d_load_image_font(&c2d,"/fonts/roboto/Roboto-Regular.ttf",50,SDL_GPU_FILTER_LINEAR);

//layers to draw to
    sparkLayer=cg2d_add_layer(&c2d,CG2D_LIGHTBLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_CLEAR);
    fontLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&roboFont->tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);

//set some images for controllers connecting and disconnecting. these can
    //also be NULL (which is the default) if you dont care.
    input_set_images(blobImage,blobImage);

//add a popup message with a long name to test the variable width of messages is working
    popup_messags_add_message("Particle Fountain","example",&c2d, blobImage,0.2);
  
    


	return SDL_APP_CONTINUE;
}


//event handling
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{

	app_state *state = (app_state *)appstate;
    //this updates the input handler stuff, controllers, mouse etc.
    int controllerChangeIndex =0;
    input_update_events(event,&controllerChangeIndex,&c2d);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;

    }else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_SPACE) {
        drawRects=!drawRects;
        
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
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_state *state = (app_state *)appstate;


    //update input - call every frame
    input_update(&c2d);

    //update popup messages - connect/disconnect a controller to spawn one
    //or press space to spawn a test message
    //call every frame
    popup_messages_update();
   
//////////////////////////////////////////////////
// add some sparks if the left mouse is being held down
//////////////////////////////////////////////////
    double now = ((double)SDL_GetTicks()) / 1000.0;  
	
	mouse_x=input_get_mouseX();
    mouse_y=input_get_mouseY();   

    //if the left mouse button is being held, add sparks!
    if(input_get_mouse_down(MOUSE_LEFT)==true){
    	for(int i=0;i<spawnCnt;i++){
	    	spark s;
	    	s.x=mouse_x;
	    	s.y=mouse_y;
	    	s.vx=(-4 + SDL_randf()*8);
	    	s.vy=-((SDL_randf()*4)+4);
	    	s.life=SDL_randf();
	    	s.r=( (float) (0.5 + 0.5 * SDL_sin(now)))*255;
	        s.g=( (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3)))*255;
	        s.b=( (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3)))*255;
	        arrpush(sparks,s);
	    }
    }


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


//////////////////////////////////////////////////
// update the sparks
//////////////////////////////////////////////////
    for(int i=0;i<arrlen(sparks);i++){
    	//apply gravity
    	sparks[i].vy+=GRAVITY;

    	//update the position
    	sparks[i].x+=sparks[i].vx;
    	sparks[i].y+=sparks[i].vy;

    	//make them bounce and slow if they hit the bottom
    	//border of the screen
    	if(sparks[i].y>cg2d_get_virtual_height(&c2d)){
    		sparks[i].y=cg2d_get_virtual_height(&c2d);
    		sparks[i].vx=-sparks[i].vx*0.1;
    		sparks[i].vy=-sparks[i].vy*0.1;
    	}

    	//make them bounce off the left and right edges
    	if(sparks[i].x<0 || sparks[i].x>cg2d_get_virtual_width(&c2d) ){
    		//clamp to the window
    		clamp(sparks[i].x,0,cg2d_get_virtual_width(&c2d));    		
    		//negate the x velocity and dampen
    		sparks[i].vx=-sparks[i].vx*0.75;
    	}

    	//decrement the life
    	sparks[i].life-=0.005;

    	//remove from the array if life is <=0.0;
    	if(sparks[i].life<=0.0){
    		arrdelswap(sparks,i);
    		continue;
    	}
    }


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


//start rendering.
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(state->Device);
    if (cmdBuf == NULL){SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError()); return SDL_APP_FAILURE;}

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, state->Window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());return SDL_APP_FAILURE; }

    if (swapchainTexture != NULL)
    {
			
//////////////////////////////////////////////////
// use cg2d functions to draw the sparks
//////////////////////////////////////////////////
    		//clear the layers so we dont draw the previous frame as well or run
    		//out of space in the buffer (and thus crash!)
			cg2d_clear_layer(&c2d,sparkLayer);
			cg2d_clear_layer(&c2d,fontLayer);

			//set the active layer to be sparkLayer
			cg2d_set_layer(&c2d,sparkLayer);			

			//loop through the sparks array and draw the sparks
			//the life value is used to do multiple things here
			
			//set a few default states
			cg2d_mid_handle_image(blobImage);//images have thier own handles
			cg2d_set_handle(&c2d,0.5,0.5);//handle for the rects
			cg2d_set_point_size(&c2d,5.0);//point size
			cg2d_set_outline_width(&c2d,0.05);//outline width
			
			//begin running through the array of sparks
			for(int i=0;i<arrlen(sparks);i++){
		    	//draw ehe coloured blob
		    	cg2d_set_rotation(&c2d,sparks[i].vx*((float)i+(float)now*300));
		    	cg2d_set_alpha(&c2d,0.5*(sparks[i].life*(i+1)));//plus one here so that index 0 is visible
		    	cg2d_set_colour(&c2d,sparks[i].r,sparks[i].g,sparks[i].b);
		    	
		    //if we're drawing rects
		    	if(drawRects==false){
			    	cg2d_set_scale(&c2d,sparks[i].life*2.0,sparks[i].life*1.0);
			    	cg2d_draw_image(&c2d,blobImage,sparks[i].x,sparks[i].y);
			    	cg2d_set_scale(&c2d,sparks[i].life*1.0,sparks[i].life*2.0);
			    	cg2d_draw_image(&c2d,blobImage,sparks[i].x,sparks[i].y);
			   
			   //if we're drawing the blob image
			    }else{
			    	cg2d_set_scale(&c2d,sparks[i].life*4.0,sparks[i].life*4.0);
			    	cg2d_draw_rect(&c2d,sparks[i].x,sparks[i].y,20,20,true);
			    }
		    //draw a point in the center to aid visibility
		    	cg2d_set_colour(&c2d,255,255,255);
		    	cg2d_set_alpha(&c2d,1.0);
		    	cg2d_plot(&c2d,sparks[i].x,sparks[i].y,!drawRects,true);
		    }

		    //change the layer and render some text instructions
		    cg2d_set_layer(&c2d,fontLayer);
		    cg2d_set_scale(&c2d,1,1);
		    cg2d_set_rotation(&c2d,0);
		    cg2d_set_colour(&c2d,255,255,255);
		    cg2d_set_alpha(&c2d,1.0);
		    cg2d_set_handle(&c2d,0.0,0.0);
		    cg2d_draw_text(&c2d,"Hold the left mouse button to make sparks",10,10);
		    
		    cg2d_set_scale(&c2d,0.75,0.75);
		    cg2d_draw_text(&c2d,"Press space to toggle rects/images",10,60);
		    
		    //draw the FPS counter
		    char fps[10];
		    SDL_snprintf(fps,sizeof(fps),"FPS:%d",get_framerate());
		    cg2d_draw_text(&c2d,fps,1150,10);
			

			//draw any popup messages - best to draw this at the end so they appear 
			//over everythings else, for convenience we'll draw any images to the blob layer
			popup_messages_draw(&c2d, sparkLayer,fontLayer);

			//render the layers to the swapchain texture
			cg2d_draw_layer(&c2d,sparkLayer,cmdBuf,swapchainTexture);
			
			//if a layer has nothing to draw, drawing will be skipped
			//so set the font layer to clear if there are no sparks to be drawn
			//and vice versa
			if(cg2d_get_layer_vertex_count(&c2d,sparkLayer)==0){
				cg2d_set_layer_clear(&c2d,fontLayer,true);
			}else{
				cg2d_set_layer_clear(&c2d,fontLayer,false);
			}
			//fontLayer is drawn to by the popup message handler, the instruction
			//test and for the FPS display
			cg2d_draw_layer(&c2d,fontLayer,cmdBuf,swapchainTexture);


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


	}
	//draw everything
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    //limit framerate
    framerate_delay();

    return SDL_APP_CONTINUE;

}


//tidy up and quit
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	app_state *state = (app_state *)appstate;

	atlas_free(&atlas,state->Device);

	arrfree(sparks);

	SDL_Log("free input handler\n");
    input_free();    
    SDL_Log("free popup message handler\n");
    popup_messages_free();

    cg2d_free(&c2d);

    SDL_Log("release window\n");
    SDL_ReleaseWindowFromGPUDevice(state->Device, state->Window);
    SDL_Log("destroy window\n");
    SDL_DestroyWindow(state->Window);
    SDL_Log("destroy device\n");
    SDL_DestroyGPUDevice(state->Device);
    
    SDL_Log("free state\n");
    SDL_free(state);
    SDL_Log("free ttf\n");
    TTF_Quit();
    SDL_Log("quit sdl\n");
    SDL_Quit();
   
    /* SDL will clean up the window/renderer for us. */
}