

void scores_init(){
	
    logoTargetX=640;
    logoTargetY=100;
    logoScale=0.5;
    logoTargetScale=0.5;

}

void scores_draw(int ticks){


	cg2d_set_cls_colour(&c2d,0,0,0);
    
    render_blobs(ticks);

    //draw the mini bullet candy logo
    render_logo(ticks);

	   
   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		char *startTxt="High Scores";
		float woff=(cg2d_text_width(&c2d,startTxt)*sx)/(2);
		cg2d_draw_text(&c2d,startTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,180);

        char *backTxt="Press Space or Button A to go back";
        woff=(cg2d_text_width(&c2d,backTxt)*sx)/(2);
        cg2d_draw_text(&c2d,backTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,650);

        for(int i=0;i<10;i++){
            char posname[36];
            SDL_snprintf(posname,sizeof(posname),"%d: %s",i+1,highScoreTable[i].name);
            cg2d_draw_text(&c2d,posname,400,230+i*40);

            char str_score[32];
            #ifdef __WIN32
                SDL_snprintf(str_score,sizeof(str_score),"%lld",highScoreTable[i].score);
            #else
                SDL_snprintf(str_score,sizeof(str_score),"%ld",highScoreTable[i].score);
            #endif
            woff=cg2d_text_width(&c2d,str_score)*sx;
            cg2d_draw_text(&c2d,str_score,880-woff,230+i*40);
        }

          
    //render to fullscreen textures
    render_textures(ticks);

}

game_state scores_update(cg_controller *active){

    update_logo();

    //update blobs
    update_blobs();

    if(active!=NULL){
        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true ||
           cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_EAST)==true ){
            play_audio(&menuSelectSFX, 1.0,0);
            menu_init();
            return STATE_MENU;
        }


    }

    return STATE_SCORE_DISPLAY;
}