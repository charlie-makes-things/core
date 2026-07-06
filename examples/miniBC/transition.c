
game_state transitionState;
int transitionStart;
Uint64 transitionScore;

void transition_init(game_state toState,int startTime,Uint64 tScore){
	//initialise the blobs in the background
	
    audio_fade_out_and_stop(_currentMusic,1000);    

    transitionState=toState;
    transitionStart=startTime;
    transitionScore=tScore;

    //logoPosX=640;
    //logoPosY=260;
    logoTargetX=640;
    logoTargetY=-760;
    logoScale=1.0;
    logoTargetScale=1.0;

}

void transition_draw(int ticks){


	cg2d_set_cls_colour(&c2d,0,0,0);
   
    

    //draw the mini bullet candy logo
    render_logo(ticks);
    
    //render texures
    render_textures(ticks);      


}

game_state transition_update(int currTime,SDL_Window *win){

    update_logo();
   
    
	//if enough time has elapsed, return the state we are transitioning to
    if(currTime>=transitionStart+60){
        switch(transitionState){
        case STATE_TITLE:
            title_init(false);
            break;
        case STATE_GAME:
            game_init(win);
            break;
        case STATE_SCORE_ENTRY:
            //init score entry here
            score_entry_init(transitionScore,win);
            break;
        default:
            //if unknown just go to title
            title_init(false);
            break;
        }
        return transitionState;
    }

	return STATE_GAME_TRANSITION;
}