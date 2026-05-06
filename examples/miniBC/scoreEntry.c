
Uint64 entryScore;
 char dispString[32];
void score_entry_init(Uint64 score,SDL_Window *win){
	
    audio_fade_out_and_stop(_currentMusic,1000);

    entryScore=score;
    
    textEntryActive=true;

    //start playing the title music
    SDL_PropertiesID options = SDL_CreateProperties();    
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_START_GAIN_FLOAT, 0.0);  /* start the first loop 1 second into the audio. */
    SDL_SetNumberProperty(options,MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER,1000);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);//-1 loop forever
    if(!play_audio(&menuMusic,1.0, options)){
    	SDL_Log("couldn't play track\n");
    }
    SDL_DestroyProperties(options);
    
    SDL_StartTextInput( win );

    logoTargetX=640;
    logoTargetY=100;
    logoScale=0.5;
    logoTargetScale=0.5;

}

void score_entry_draw(int ticks){


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
                                                                //1-(i*(1/20))
		     float scl=(logoScale - (i*(logoScale/20)));
            float rad=logoScale/5;
            cg2d_set_scale(&c2d,rad*cg2d_sin((float)ticks*0.1)+scl,rad*cg2d_cos((float)ticks*0.1)+scl);
            cg2d_draw_image(&c2d,logoImage,logoPosX,logoPosY);
		}

	
   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		char *startTxt="You Got A High Score!!!!";
        float woff=(cg2d_text_width(&c2d,startTxt)*sx)/(2);

        cg2d_draw_text(&c2d,startTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,200);

        char *typeTxt="Type your name";
        woff=(cg2d_text_width(&c2d,typeTxt)*sx)/(2);
        cg2d_draw_text(&c2d,typeTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,280);
        
        char *enterTxt="Press Enter to continue";
        woff=(cg2d_text_width(&c2d,enterTxt)*sx)/(2);
        cg2d_draw_text(&c2d,enterTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,480);
        
        if(userTypedStringLen>0){
           
            SDL_memset(dispString,0,sizeof(dispString));
            for(int i=0; i<userTypedStringLen;i++){
                dispString[i]=userTypedString[i];
            }

            cg2d_set_scale(&c2d,1.25,1.25);
            cg2d_get_scale(&c2d,&sx,&sy);
            woff=(cg2d_text_width(&c2d,dispString)*sx)/(2);
            cg2d_draw_text(&c2d,dispString,(cg2d_get_virtual_width(&c2d)/2)-woff,320);
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

game_state score_entry_update(cg_controller *active,SDL_Window *win){

    logoPosX+=(logoTargetX-logoPosX)*0.1;
    logoPosY+=(logoTargetY-logoPosY)*0.1;
    logoScale+=(logoTargetScale-logoScale)*0.1;
    
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

    if(textEntryActive==false){
        //stop text entry
        SDL_StopTextInput(win );
        //push score into list and sort
        high_score_entry s;
        SDL_strlcpy(s.name,dispString,sizeof(s.name));
        s.score=entryScore;
        arrpush(highScoreTable,s);
        SDL_qsort(highScoreTable, arrlen(highScoreTable), sizeof(high_score_entry), high_score_sort);
        //go to score screen
        scores_init();
        return STATE_SCORE_DISPLAY;
    }
   

	return STATE_SCORE_ENTRY;
}