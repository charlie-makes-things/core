
game_state transitionState;
int transitionStart;
Uint64 transitionScore;

void transition_init(game_state toState,int startTime,Uint64 tScore){
	//initialise the blobs in the background
	
    audio_fade_out_and_stop(_currentMusic,1000);
    
    

    transitionState=toState;
    transitionStart=startTime;
    transitionScore=tScore;

    logoPosX=640;
    logoPosY=260;
    logoTargetX=640;
    logoTargetY=-760;
    logoScale=1.0;
    logoTargetScale=1.0;

}

void transition_draw(int ticks){


	double now = ((double)SDL_GetTicks()) / 4000.0;  /* convert from milliseconds to seconds. */
    float red = (float) (0.5 + 0.5 * SDL_sin(now));
    float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    
	cg2d_set_cls_colour(&c2d,0,0,0);
    cg2d_set_layer(&c2d, effectsLayer);
         
    

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
                                                                //1-(i*(1/20))
		     float scl=(logoScale - (i*(logoScale/20)));
            float rad=logoScale/5;
            cg2d_set_scale(&c2d,rad*cg2d_sin((float)ticks*0.1)+scl,rad*cg2d_cos((float)ticks*0.1)+scl);
            cg2d_draw_image(&c2d,logoImage,logoPosX,logoPosY);
		}
  
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

game_state transition_update(int currTime,SDL_Window *win){

    logoPosX+=(logoTargetX-logoPosX)*0.1;
    logoPosY+=(logoTargetY-logoPosY)*0.1;
    logoScale+=(logoTargetScale-logoScale)*0.1;
    
	//if enough time has elapsed, return the state we are transitioning to
    if(currTime>=transitionStart+60){
        switch(transitionState){
        case STATE_TITLE:
            title_init();
            break;
        case STATE_GAME:
            game_init();
            break;
        case STATE_SCORE_ENTRY:
            //init score entry here
            score_entry_init(transitionScore,win);
            break;
        default:
            //if unknown just go to title
            title_init();
            break;
        }
        return transitionState;
    }

	return STATE_GAME_TRANSITION;
}