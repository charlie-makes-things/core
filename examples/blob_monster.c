/* Blob monster example - by charlie 2026
part of https://github.com/charlie-makes-things/core

This example demonstrates some procedural animation using cg2d's image 
transformation functions to draw a blobMonster and move it around the screen.

It also sets up core to handle input/popup messages etc

This example keeps it pretty simple, but with a few changes you can add 
more monsters and change their appearances.

If you create a blob monster of your own, please share it!

for ease of navigation, the important initialization, update and draw code
is surrounded by lots of ////////////////////////////////////////////

Things to try:
			-- randomise the colour,speed and length of the blobmonster
			   in the init section
			-- use some other images loaded in by the texture atlas
			-- have an array of different blob monsters instead of just one
			-- have the colours change/pulsate depending on how fast the monster
			   is moving
			-- have the monster(s) chase the mouse pointer
			-- have the monster(s) be controlled by the keyboard,mouse or controllers
*/


//include core files
#include "../core/core.c"

//some SDL state variables
typedef struct app_state{    
    SDL_Window* Window;
    SDL_GPUDevice* Device;   
    MIX_Mixer *mixer;   
}app_state;


//texture atlas struct
static texAtlas atlas;

//the cg2d struct.
static cg2d_t c2d;

//the blob image we'll use to create the monster
static cg2d_image *blobImage;

//a font so we can render text. Silver is an excellent TTf pixel font
//that contains a huge amount of the unicode characters and more!
//check it out here: https://poppyworks.itch.io/silver
static cg2d_font *silverFont;

//some layers to render into
static int blobLayer;
static int fontLayer;

//2d point, we'll use these for the monster's coordinates
typedef struct point{
	float x,y;
}point;

//the blobMonster struct - this holds the info for the blobMonster's
//segments. other parts of the monster are calculated on the fly.
typedef struct blobMonster{
	//x and y coords in a stretchy buffer
	point *pos;
	//speed, try changing it
	float speed;	
	//number of nodes along the body, try changing it to 100
	int segments;		
	//simple ticker
	float time;
	//the main colour for the monster
	int r,g,b;
}blobMonster;

//a blobMonster!
static blobMonster monster;


//init function
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	app_state *state = SDL_calloc(1, sizeof(app_state));    
    SDL_SetAppMetadata("Blob Monster Example", "1.0", "charliemakesthings.com");
//////////////////////////////////////////////////////////////////////
//core setup
//////////////////////////////////////////////////////////////////////

    //init core functions. you can set WINDOW_WIDTH etc in core.c
    //make sure you set the final parameter, which sets your asset path. all
    //load functions are relative to this.
    if(core_init(&state->Device,
    			 &state->Window, 
    			 1280, 
    			 720,
    			 "Blob Monster Example",
    			 &state->mixer,
    			 "assets/",
    			 3.0,
    			 ,false)==SDL_APP_FAILURE)
    {	SDL_Log("failed to init core.\n");return SDL_APP_FAILURE;    }

    *appstate = state;


// Create and load the shaders
    SDL_GPUShader* vertexShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.vert", 0, 1, 1, 0);
    if (vertexShader == NULL) {SDL_Log("Failed to create vertex shader!");return SDL_APP_FAILURE; }

    SDL_GPUShader* fragmentShader = load_shader_storage(state->Device, "shaders/compiled/cg2d.frag", 1, 1, 0, 0);
    if (fragmentShader == NULL) { SDL_Log("Failed to create fragment shader!"); return SDL_APP_FAILURE; }

//load texture atlas
    if(atlas_init(&atlas,"data/bc26Atlas.json","images/",state->Device,ATLAS_FILTER_LINEAR)==1){SDL_Log("failed to load altas\n"); return SDL_APP_FAILURE; }   
     

 //initialise cg2d
    cg2d_init(&c2d,state->Window,state->Device, WINDOW_WIDTH,WINDOW_HEIGHT);
 //get the image
    blobImage=cg2d_load_atlas_image(&c2d,&atlas,"blob.png");
 //load the font for popup messages
    silverFont=cg2d_load_image_font(&c2d,"/fonts/roboto/Roboto-Regular.ttf",50,SDL_GPU_FILTER_LINEAR);

//layers to draw to
    blobLayer=cg2d_add_layer(&c2d,CG2D_LIGHTBLEND,&atlas.tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_CLEAR);
    fontLayer=cg2d_add_layer(&c2d,CG2D_ALPHABLEND,&silverFont->tex,vertexShader,fragmentShader,SDL_GPU_LOADOP_LOAD);

//set some images for controllers connecting and disconnecting. these can
    //also be NULL (which is the default) if you dont care.
    input_set_images(blobImage,blobImage);

//add a popup message with a long name to test the variable width of messages is working
    popup_messags_add_message("Blob Monster","example",&c2d, blobImage,0.2);
  

//////////////////////////////////////////////////
// initialise the blobMonster - change the values for different appearances
//////////////////////////////////////////////////

//set the default values for the blob monster
    //try changing these to different values!
    monster.speed=1.0f;//speed
    monster.time=0.0f;
    monster.segments=10;//number of tail segments
    monster.pos=NULL;//set to null for stretchy buffer
    monster.r=255;//you can change the colour of the 
    monster.g=200;//monster using these 3 variables
    monster.b=150;//colours in the range rgb 0..255
	//add tail segnents    
    for(int i=0;i<monster.segments;i++){
    	point t=(point){10.0f,10.0f};
    	arrpush(monster.pos,t);
    }
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

	return SDL_APP_CONTINUE;
}


//event handling
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    //this updates the input handler stuff, controllers, mouse etc.
    int controllerChangeIndex =0;
    input_update_events(event,&controllerChangeIndex,&c2d);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
    
    } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
                       
    }
     return SDL_APP_CONTINUE;
}




//main loop
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_state *state = (app_state *)appstate;


    //update input - call every frame
    input_update();

    //update popup messages - connect/disconnect a controller to spawn one
    //or press space to spawn a test message
    //call every frame
    popup_messages_update();
   
//////////////////////////////////////////////////
// update the blob monster's position and make the tail follow
//////////////////////////////////////////////////

//update the head and tail segments of the blobMonster.

    //increase the timer for the monster
    monster.time+=monster.speed;

    //move the monsters head. this uses the basic rule for rotating a point
    //around an origin.
    	//	x = radius * sin(angle) + originx
    	//	y = radius * cos(angle) + originy
    //this method is essentially how everything moves in this example
    //note here that cg2d uses degrees not radians

    //the head is the first array element and all the points in the tail follow this
    //point, so you could make it user controllable by changing these variables
    
    //final position =  radius*sin/cos 					  + origin (the center of the window plus a varying offset)
    monster.pos[0].x = (15 * cg2d_sin(monster.time * -6)) + ((WINDOW_WIDTH/2 )+((WINDOW_WIDTH/3 )*cg2d_sin(monster.time/1.5f))) ;
    monster.pos[0].y = (15 * cg2d_cos(monster.time * -6)) + ((WINDOW_HEIGHT/2)+((WINDOW_HEIGHT/3)*cg2d_sin(monster.time*1.3f))) ;
						

	//update the tail points so they follow the head at a distance
	for(int i=1;i<monster.segments;i++){
		//calculate the distance between this position and the previous one
		float dx=monster.pos[i-1].x - monster.pos[i].x;
		float dy=monster.pos[i-1].y - monster.pos[i].y;
		float dist = SDL_sqrtf(dx * dx + dy * dy);

		//move the tail segment towards the previous segment 
		//if it's too far away. dampen the motion to make it smoother looking
		if(dist > 7.0f){
			monster.pos[i].x += dx * 0.3f;
			monster.pos[i].y += dy * 0.3f;
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
// use cg2d functions to draw the blob monster.
//////////////////////////////////////////////////
    		//clear the layers so we dont draw the previous frame as well or run
    		//out of space in the buffer (and thus crash!)
			cg2d_clear_layer(&c2d,blobLayer);
			cg2d_clear_layer(&c2d,fontLayer);

			//set the active layer to be blobLayer
			cg2d_set_layer(&c2d,blobLayer);
			//begin drawing the monster to the blob layer
			//start by setting the colour
			cg2d_set_colour(&c2d,monster.r,monster.g,monster.b);
			cg2d_set_scale(&c2d,1.0f,1.0f);
			cg2d_set_rotation(&c2d,0.0f);

		//draw the main part of the body
			//------------------------------		
			//start looping throught the tail
			for (int i=0;i<monster.segments;i++){
			
			//first draw the main segments of the body
				//set alpha transparency to 0.15, pretty well transparent!
				cg2d_set_alpha(&c2d,0.1f);
				cg2d_set_colour(&c2d,monster.r,monster.g,monster.b);
				cg2d_set_rotation(&c2d,0.0f);
				//set the image handle. this is the origin of the image/primitive 
				//being draw. i.e 0,0 is draw from top left, 1,1 is draw from top 
				//right. 
				cg2d_mid_handle_image(blobImage);
				//set the scale so that each segment bulges the size of
				//the point being drawn as it gets closer to the center of
				//the tail, and tapering off as it gets closer to the end			
				cg2d_set_scale(&c2d,1.0f+(0.5f*cg2d_sin(i*35)),1.0f+(0.5f*cg2d_sin(i*35)));
				
				//draw the blob image at the tail position
				cg2d_draw_image(&c2d,blobImage,monster.pos[i].x,monster.pos[i].y);
				
				//draw a small dot in the center of each tail position as a
				//highlight
				cg2d_set_alpha(&c2d,0.8f);
				cg2d_set_scale(&c2d,0.1f,0.1f);
				cg2d_draw_image(&c2d,blobImage,monster.pos[i].x,monster.pos[i].y);

			//next, draw the spikes on the end of tail.
				if(i==monster.segments-1){
					cg2d_set_colour(&c2d,255,255,255);
					cg2d_set_scale(&c2d,0.6,0.1);
					cg2d_set_alpha(&c2d,0.5);
					//move the handle to the middle of the left hand edge
					cg2d_set_image_handle(blobImage,0.0f,0.5f);
					//rotate the 1st tail image. basically, we're calculating
					//the angle between the last 2 points of the tail, and then 
					//adding an extra wobble (the 10*sin(time*10) bit) to make 
					//the pincer type effect.
					float angle=cg2d_atan2(monster.pos[monster.segments-1].y
										   - monster.pos[monster.segments-5].y,
										   monster.pos[monster.segments-1].x
										   - monster.pos[monster.segments-5].x);

					cg2d_set_rotation(&c2d,10 * cg2d_sin(monster.time * 10) + angle );
					cg2d_draw_image(&c2d,blobImage,monster.pos[monster.segments-1].x,
												   monster.pos[monster.segments-1].y);
					//negate time to make the rotation in the opposite direction
					cg2d_set_rotation(&c2d,10 * cg2d_sin(-monster.time * 10) + angle );
					cg2d_draw_image(&c2d,blobImage,monster.pos[monster.segments-1].x,
												   monster.pos[monster.segments-1].y);
				}

			//next, draw the little fins
				if(i>0 && i<monster.segments-2){
					cg2d_set_colour(&c2d,monster.r,monster.g,monster.b);
					//like the bulging body, we want the fins to grow larger 
					//in the center, and smaller at the end, so the same sort
					//of thing is used here.
					cg2d_set_scale(&c2d, 0.1 + (0.6 * cg2d_sin(i * 30)), 0.05);
					//move the handle to the middle of the top edge
					cg2d_set_image_handle(blobImage,0.0f,0.5f);
					//rotate the image. We want the fins to stick out sideways
					//from the body (the atan2() bit) and also to move
					//a little on their own. the 33 * Sin(time * 5 + i * 30) 
					//makes the fin rotate based in the i index variable, so that 
					//all the fins look like they're moving one after the other.
					float angle=cg2d_atan2(monster.pos[i].y
										   - monster.pos[i-1].y,
										   monster.pos[i].x
										   - monster.pos[i-1].x);
					cg2d_set_rotation( &c2d,   33 * cg2d_sin(-monster.time * 5 + -i * 30) +angle +90);
					//draw the fin
					cg2d_draw_image(&c2d,blobImage,monster.pos[i].x,monster.pos[i].y);

					//negate time and i in the rotation calculation to reflect 
					//the rotation of the second fin, then add 180 degrees to the angle
					cg2d_set_rotation(&c2d, 33 * cg2d_sin(monster.time * 5 + i * 30) +  angle-90);
					//draw the fin
					cg2d_draw_image(&c2d,blobImage,monster.pos[i].x,monster.pos[i].y);
				}
			}

			//finally decorate the head
				cg2d_mid_handle_image(blobImage);
				//draw the eyes - at +/-50 degrees to the head of the tail
				//draw the red part first
				cg2d_set_colour(&c2d,255,0,0);
				cg2d_set_scale(&c2d,0.6,0.6);
				cg2d_set_alpha(&c2d,0.3);
				float ang=cg2d_atan2( monster.pos[0].y-monster.pos[1].y, monster.pos[0].x-monster.pos[1].x);
				cg2d_draw_image(&c2d,blobImage, monster.pos[0].x + (7 * cg2d_cos(ang + 50)), monster.pos[0].y + (7 * cg2d_sin(ang + 50)));
				cg2d_draw_image(&c2d,blobImage, monster.pos[0].x + (7 * cg2d_cos(ang-50)), monster.pos[0].y + (7 * cg2d_sin(ang -50)));
				//then the white dot in the center of the eyes
				cg2d_set_colour(&c2d,255,255,255);
				cg2d_set_scale(&c2d,0.2,0.2);
				cg2d_set_alpha(&c2d,0.5);
				cg2d_draw_image(&c2d,blobImage, monster.pos[0].x + (7 * cg2d_cos(ang + 50)), monster.pos[0].y + (7 * cg2d_sin(ang + 50)));
				cg2d_draw_image(&c2d,blobImage, monster.pos[0].x + (7 * cg2d_cos(ang-50)), monster.pos[0].y + (7 * cg2d_sin(ang -50)));
				
				//draw the beaky thing
				cg2d_set_colour(&c2d,monster.r,monster.g,monster.b);
				cg2d_set_scale(&c2d,0.6, 0.1);
				cg2d_set_alpha(&c2d, 0.8);
				cg2d_set_image_handle( blobImage, 0.0, 0.5);
				cg2d_set_rotation( &c2d,ang );
				cg2d_draw_image(&c2d, blobImage, monster.pos[0].x, monster.pos[0].y);

				//finally draw a big 'light' halo
				cg2d_mid_handle_image(blobImage);
				cg2d_set_colour(&c2d,monster.r,monster.g,monster.b);
				cg2d_set_alpha(&c2d,0.1);
				cg2d_set_scale(&c2d,4.0f,4.0f);
				cg2d_draw_image(&c2d,blobImage,monster.pos[0].x,monster.pos[0].y);



			//draw any popup messages - best to draw this at the end so they appear 
			//over everythings else, for convenience we'll draw any images to the blob layer
			popup_messages_draw(&c2d, blobLayer,fontLayer);

			//render the layers to the swapchain texture
			cg2d_draw_layer(&c2d,blobLayer,cmdBuf,swapchainTexture);
			//fontLayer is drawn to by the popup message handler.
			cg2d_draw_layer(&c2d,fontLayer,cmdBuf,swapchainTexture);


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


	}
	//draw everything
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    return SDL_APP_CONTINUE;

}


//tidy up and quit
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	app_state *state = (app_state *)appstate;

	SDL_Log("free input handler\n");
    input_free();    
    SDL_Log("free popup message handler\n");
    popup_messages_free();

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