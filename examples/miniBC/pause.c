void pause_init(){
	
	pausePosition=0;  
    logoTargetX=640;
    logoTargetY=200; 
    logoScale=1.0;
    logoTargetScale=1.0; 
    
    pauseOptions[0]= pause_resumeMessage;
    pauseOptions[1]= pause_fullscreenMessage;
    pauseOptions[2]= pause_SFXMessage;
    pauseOptions[3]= pause_MusicMessage;
    pauseOptions[4]= pause_quitMessage;

}

void pause_draw(int ticks,SDL_Window *w){

    cg2d_set_cls_colour(&c2d,0,0,0);
    
    // //render blobs
    // render_blobs(ticks);
    
	
    //draw a rect over the game image to dim it.
    cg2d_set_layer(&c2d,spriteLayer);
    cg2d_set_colour(&c2d,0,0,0);
    cg2d_set_alpha(&c2d,0.5);
    cg2d_set_handle(&c2d,0,0);
    cg2d_set_scale(&c2d,1,1);
    cg2d_draw_rect(&c2d,0,0,1280,720,false);

    cg2d_set_colour(&c2d,255,255,255);
    cg2d_set_alpha(&c2d,1.0);

    //draw the mini bullet candy logo
    render_logo(ticks);

	//draw the by charlie text
		cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,1,1);
		char *creditTxt="PAUSED";
		float woff=cg2d_text_width(&c2d,creditTxt)/2;
		cg2d_draw_text(&c2d,creditTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)/2-50);
   
    //make the strings to draw
        if(video_is_fullscreen(w)==true){
            if(video_is_exclusive_fullscreen(w)==true){
                SDL_snprintf(pause_fullscreenMessage,sizeof(pause_fullscreenMessage),"Fullscreen (Exclusive)");
            }else{
                SDL_snprintf(pause_fullscreenMessage,sizeof(pause_fullscreenMessage),"Fullscreen (Desktop)");
            }
        }else{
            SDL_snprintf(pause_fullscreenMessage,sizeof(pause_fullscreenMessage),"Windowed");
        }

        SDL_snprintf(pause_resumeMessage,sizeof(pause_resumeMessage),"Resume Game");
      
        SDL_snprintf(pause_MusicMessage,sizeof(pause_MusicMessage),"Music Volume - %d",(int)(musicGain*100));
        SDL_snprintf(pause_SFXMessage,sizeof(pause_SFXMessage),"SFX Volume - %d",(int)(sfxGain*100));
        SDL_snprintf(pause_quitMessage,sizeof(pause_quitMessage),"Quit to Menu");


   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		
        for(int i=0;i<5;i++){
            woff=(cg2d_text_width(&c2d,pauseOptions[i])*sx)/(2);
            
            cg2d_set_alpha(&c2d,1.0 );

            cg2d_draw_text(&c2d,pauseOptions[i],
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/2)+((i+1)*40));
        
            if(i==pausePosition){
                 cg2d_get_scale(&c2d,&sx,&sy);
                 woff=(cg2d_text_width(&c2d,"->                                       <-")*sx)/(2);
                 cg2d_draw_text(&c2d,"->                                        <-",
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/2)+((i+1)*40));
            }
        }
        woff=(cg2d_text_width(&c2d,"Press Left/Right to change setting.")*sx)/(2);
        cg2d_draw_text(&c2d,"Press Left/Right to change setting.",(cg2d_get_virtual_width(&c2d)/2)-woff,(cg2d_get_virtual_height(&c2d)/2)+(6*42));
	  
    //render to fullscreen textures
    //render_textures(ticks);


}

game_state pause_update(cg_controller *active,int ticks,SDL_Window *w){

    static int inputDelayTicker=0;

    update_logo();

    if(inputDelayTicker>0){
        inputDelayTicker--;
    }

	//update blobs
	update_blobs();



    if(active!=NULL){

        if(inputDelayTicker<=0){
            
            int lastPosition=pausePosition;
            float yAxis=cg_controller_get_axis_normalized(active,SDL_GAMEPAD_AXIS_LEFTY);
            float xAxis=cg_controller_get_axis_normalized(active,SDL_GAMEPAD_AXIS_LEFTX);
            
            if(yAxis>0){
                pausePosition+=1;
            }else if(yAxis<0){
                pausePosition-=1;
            }

            if(pausePosition<0){
                pausePosition=4;
            }else if(pausePosition>4){
                pausePosition=0;
            }

            if(lastPosition!=pausePosition){
                play_audio(&menuClickSFX, 1.0,0);
                inputDelayTicker=10;
            }

           if(pausePosition==1 && xAxis>0){
               //fullscreen/windowed
                                //as we are only flicking between windowed and fullscreen at desktop size
                //we can use the code in the event handler function in miniBC.c to change
                //resolutions by putting an event in the event queue, in this case
                //simulating the F11 key being pressed.
                SDL_Event ev;
                SDL_KeyboardEvent e;
                e.type=SDL_EVENT_KEY_DOWN;                
                e.key=SDLK_F11;
                ev.key=e;
                SDL_PushEvent(&ev);
                play_audio(&menuClickSFX, 1.0,0);
                inputDelayTicker=10;
                
            }
            if(pausePosition==3 && xAxis<0){
                //music
                if(musicGain>0.0){
                    musicGain-=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                    audio_update_volumes();
                }


            }

            if(pausePosition==3 && xAxis>0){
                //music
               
                if(musicGain<1.0){
                    musicGain+=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                    audio_update_volumes();
                }
            }
            if(pausePosition==2 && xAxis<0){
                //sfx
                if(sfxGain>0.0){
                    sfxGain-=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                }
            }
            if(pausePosition==2 && xAxis>0){
                //sfx
                if(sfxGain<1.0){
                    sfxGain+=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                }
            }

        }

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true){
            switch(pausePosition){
            case 0://resume
                play_audio(&menuBackSFX,0.5,0);
                SDL_SetWindowRelativeMouseMode(w, true);
                return STATE_GAME;
                break;
            case 1://music vol
                
                
                break;
            case 2://sqf vol

                break;
            case 4://back                
                play_audio(&menuBackSFX,0.5,0);
                transition_init(STATE_TITLE,ticks,0); 
                SDL_SetWindowRelativeMouseMode(w, false);               
                return STATE_GAME_TRANSITION;
                break;
            default:
                break;
            }
        }  

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_EAST)==true){
            logoTargetX=640;
            logoTargetY=260;
            play_audio(&menuBackSFX,0.5,0);
            menu_init();
            return STATE_GAME;
        }    
    }

	return STATE_GAME_PAUSED;
}