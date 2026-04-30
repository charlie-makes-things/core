
static Uint32 BLOB_COUNT=1000;
static blob *blobs=NULL;//stretchy buffer for blobs. see stb_ds.h for info

void title_init(){
	//initialise the blobs in the background
	for(int i=0;i<BLOB_COUNT;i++){        
        blob b;
        //only need to set a few blob values
        //the rest are set during update.
        b.spd=SDL_randf();
        b.deg=SDL_randf()*360;
        b.r=b.g=b.b=255;
        float scl=SDL_randf()+0.2;
        b.sx=b.sy=scl;
        arrput(blobs,b);
    }
    SDL_PropertiesID options = SDL_CreateProperties();
    
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_START_GAIN_FLOAT, 0.0);  /* start the first loop 1 second into the audio. */
    SDL_SetNumberProperty(options,MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER,1000);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);//-1 loop forever
    if(!MIX_PlayTrack(menuMusic, options)){
    	SDL_Log("couldn't play track\n");
    }
    SDL_DestroyProperties(options);
}

void title_draw(int ticks){


	 double now = ((double)SDL_GetTicks()) / 4000.0;  /* convert from milliseconds to seconds. */
    float red = (float) (0.5 + 0.5 * SDL_sin(now));
    float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
        

	cg2d_set_cls_colour(&c2d,0,0,0);
    cg2d_set_layer(&c2d, effectsLayer);
         
    for(int i=0;i<BLOB_COUNT;i++){
        blob *b=&blobs[i];       

        cg2d_set_scale(&c2d,b->sx*0.2,b->sy*0.2);
        cg2d_set_alpha(&c2d,b->alpha);
        if(i % 2 ==0){
            cg2d_set_scale(&c2d,b->sx*0.2,b->sy*0.2);
            cg2d_set_colour(&c2d,b->r,50+(cg2d_sin(ticks+i))+100,b->b);
            cg2d_draw_image(&c2d,redImage,b->x,b->y);
        }else{
            cg2d_set_scale(&c2d,b->sx*0.1,b->sy*0.1);
            cg2d_set_colour(&c2d,b->r,SDL_rand(150),0);
            cg2d_draw_image(&c2d,redImage,b->x,b->y);

        }
    }

    //draw the mini bullet candy logo
	    cg2d_set_layer(&c2d,spriteLayer);
	    	    
	    cg2d_mid_handle_image(logoImage);	    
	    for(float i=10;i>=0;i--){
		    if(i==0){
		    	cg2d_set_alpha(&c2d,1.0);
	    		cg2d_set_colour(&c2d,255,255,255);
	    		cg2d_set_rotation(&c2d,0);
	    		cg2d_set_layer(&c2d,spriteLayer);
		    }else{
		    	cg2d_set_alpha(&c2d,0.25-i*0.025);
		    	cg2d_set_colour(&c2d,red*255,green*255,blue*255);
		    	cg2d_set_rotation(&c2d,(i*2)*cg2d_sin((float)ticks*0.4));
		    	cg2d_set_layer(&c2d,effectsLayer);
		    }

		    cg2d_set_scale(&c2d,0.2*cg2d_sin((float)ticks*0.1)+(1.0-i*0.05),0.2*cg2d_cos((float)ticks*0.1)+(1.0-i*0.05));
		    cg2d_draw_image(&c2d,logoImage,cg2d_get_virtual_width(&c2d)/2,(cg2d_get_virtual_height(&c2d)/2)-100);
		}

	//draw the by charlie text
		cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5,0.5);
		char *creditTxt="Charlie - 2026";
		float woff=cg2d_text_width(&c2d,creditTxt)/4;
		cg2d_draw_text(&c2d,creditTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)-50);
   
   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		char *startTxt="Press Space or a Controller Button to Start";
		woff=(cg2d_text_width(&c2d,startTxt)*sx)/(2);

		cg2d_draw_text(&c2d,startTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)/1.45);
          
    //draw the copy of the last frame scaled, recoloured and rotated etc. 
        cg2d_mid_handle_image(renderTexImage2);
        cg2d_set_layer(&c2d,renderTargetImageLayer2);                     
       cg2d_set_colour(&c2d,red*255,green*255,blue*255);
        cg2d_set_scale(&c2d,0.001*cg2d_sin((float)ticks*0.1)+1.01,0.0025*cg2d_sin(-(float)ticks*0.07)+1.01);
        cg2d_set_rotation(&c2d,cg2d_sin((float)ticks)*0.1);
        cg2d_set_alpha(&c2d,0.99801);
        cg2d_draw_image(&c2d,renderTexImage, cg2d_get_virtual_width(&c2d)/2,cg2d_get_virtual_height(&c2d)/2);
                
    //draw a screen size image without transformation. this will show the data for this frame.
        cg2d_mid_handle_image(renderTexImage);
        cg2d_set_layer(&c2d,renderTargetImageLayer);  
        cg2d_set_colour(&c2d,255,255,255);
        cg2d_set_scale(&c2d,1.0f,1.0f);
        cg2d_set_rotation(&c2d,0);
        cg2d_set_alpha(&c2d,1.0);
        cg2d_draw_image(&c2d,renderTexImage, cg2d_get_virtual_width(&c2d)/2,cg2d_get_virtual_height(&c2d)/2);
            


}

game_state title_update(cg_controller *active){

	//update blobs
	for(int i=0;i<BLOB_COUNT;i++){
        blob *b=&blobs[i];
        
        b->deg+=b->spd*0.5;
        if(b->deg>360){
            b->deg-=360;
        }
        float ang=b->deg;

        b->rad=i*(cg2d_sin(ang));
        b->x=b->rad*(cg2d_cos( (i%2==0)? i:-i))+640;
        b->y=b->rad*(cg2d_sin(( i%2==0)? -i:i))+360;        
        b->alpha=(0.9*cg2d_sin((float)i))+0.5;
        
    }

	return STATE_TITLE;
}