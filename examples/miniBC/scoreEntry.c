

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


	cg2d_set_cls_colour(&c2d,0,0,0);
         
    render_blobs(ticks);

    //draw the mini bullet candy logo
    render_logo(ticks);

	
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

          
    //render to fullscreen textures
    render_textures(ticks); 


}

game_state score_entry_update(cg_controller *active,SDL_Window *win){

    update_logo();
    
	//update blobs
	update_blobs();

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