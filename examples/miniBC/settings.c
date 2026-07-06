


void settings_init(){
	
	settingsPosition=0;  
    logoTargetX=640;
    logoTargetY=260; 
    logoScale=1.0;
    logoTargetScale=1.0; 
    
    settingsOptions[0]=fullscreenMessage;
    settingsOptions[1]=musicMessage;
    settingsOptions[2]=sfxMessage;
    settingsOptions[3]=backMessage;

}

void settings_draw(int ticks,SDL_Window *w){

    cg2d_set_cls_colour(&c2d,0,0,0);
    
    //render blobs
    render_blobs(ticks);
    
    //draw the mini bullet candy logo
	render_logo(ticks);

	//draw the by charlie text
		cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5,0.5);
		char *creditTxt="Charlie - 2026";
		float woff=cg2d_text_width(&c2d,creditTxt)/4;
		cg2d_draw_text(&c2d,creditTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)-50);
   
    //make the strings to draw
        if(video_is_fullscreen(w)==true){
            if(video_is_exclusive_fullscreen(w)==true){
                SDL_snprintf(fullscreenMessage,sizeof(fullscreenMessage),"Fullscreen (Exclusive)");
            }else{
                SDL_snprintf(fullscreenMessage,sizeof(fullscreenMessage),"Fullscreen (Desktop)");
            }
        }else{
            SDL_snprintf(fullscreenMessage,sizeof(fullscreenMessage),"Windowed");
        }

        
        SDL_snprintf(musicMessage,sizeof(musicMessage),"Music Volume - %d",(int)(musicGain*100));
        SDL_snprintf(sfxMessage,sizeof(sfxMessage),"SFX Volume - %d",(int)(sfxGain*100));
        SDL_snprintf(backMessage,sizeof(backMessage),"Back to Menu");


   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		
        for(int i=0;i<4;i++){
            woff=(cg2d_text_width(&c2d,settingsOptions[i])*sx)/(2);
            
            cg2d_set_alpha(&c2d,1.0 );

            cg2d_draw_text(&c2d,settingsOptions[i],
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/2)+((i+1)*40));
        
            if(i==settingsPosition){
                 cg2d_get_scale(&c2d,&sx,&sy);
                 woff=(cg2d_text_width(&c2d,"->                                       <-")*sx)/(2);
                 cg2d_draw_text(&c2d,"->                                        <-",
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/2)+((i+1)*40));
            }
        }
        woff=(cg2d_text_width(&c2d,"Press Left/Right to change setting.")*sx)/(2);
        cg2d_draw_text(&c2d,"Press Left/Right to change setting.",(cg2d_get_virtual_width(&c2d)/2)-woff,(cg2d_get_virtual_height(&c2d)/2)+(6*40));
	  
    //render to fullscreen textures
    render_textures(ticks);


}

game_state settings_update(cg_controller *active){

    static int inputDelayTicker=0;

    update_logo();

    if(inputDelayTicker>0){
        inputDelayTicker--;
    }

	//update blobs
	update_blobs();



    if(active!=NULL){

        if(inputDelayTicker<=0){
            
            int lastPosition=settingsPosition;
            float yAxis=cg_controller_get_axis_normalized(active,SDL_GAMEPAD_AXIS_LEFTY);
            float xAxis=cg_controller_get_axis_normalized(active,SDL_GAMEPAD_AXIS_LEFTX);
            
            if(yAxis>0){
                settingsPosition+=1;
            }else if(yAxis<0){
                settingsPosition-=1;
            }

            if(settingsPosition<0){
                settingsPosition=3;
            }else if(settingsPosition>3){
                settingsPosition=0;
            }

            if(lastPosition!=settingsPosition){
                play_audio(&menuClickSFX, 1.0,0);
                inputDelayTicker=10;
            }

            if(settingsPosition==0 && xAxis!=0){
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

            if(settingsPosition==1 && xAxis>0){
                if(musicGain<1.0){
                    musicGain+=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                    audio_update_volumes();
                }
            }
            if(settingsPosition==1 && xAxis<0){
                if(musicGain>0.0){
                    musicGain-=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                    audio_update_volumes();
                }
            }

            if(settingsPosition==2 && xAxis>0){
                if(sfxGain<1.0){
                    sfxGain+=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                }
            }
            if(settingsPosition==2 && xAxis<0){
                if(sfxGain>0.0){
                    sfxGain-=0.05;
                    inputDelayTicker=10;
                    play_audio(&menuClickSFX, 1.0,0);
                }
            }

        }

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true){
            switch(settingsPosition){
            case 0://fullsecreen/windowed

                break;
            case 1://music vol
                
                
                break;
            case 2://sqf vol

                break;
            case 3://back                
                play_audio(&menuBackSFX,1.0,0);
                menu_init();
                return STATE_MENU;
                break;
            default:
                break;
            }
        }  

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_EAST)==true){
            logoTargetX=640;
            logoTargetY=260;
            play_audio(&menuBackSFX,1.0,0);
            menu_init();
            return STATE_MENU;
        }    
    }

	return STATE_SETTINGS;
}