

void menu_init(){
	//initialise the blobs in the background
	menuPosition=0;  
    logoTargetX=640;
    logoTargetY=260; 
    logoScale=1.0;
    logoTargetScale=1.0;     
}

void menu_draw(int ticks){


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
   
   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		
        for(int i=-1;i<2;i++){
           int index=menuPosition+i;
            if(index<0){
                index+=5;
            }else if(index>4){
                index-=5;
            }
            
            woff=(cg2d_text_width(&c2d,menuOptions[index])*sx)/(2);
            
            cg2d_set_alpha(&c2d,1.0 - SDL_abs(i)*0.8);

            cg2d_draw_text(&c2d,menuOptions[index],
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/1.50)+((i+1)*40));
        
        }

        cg2d_get_scale(&c2d,&sx,&sy);
        cg2d_set_alpha(&c2d,1.0);
        woff=(cg2d_text_width(&c2d,"->                         <-")*sx)/(2);
        cg2d_draw_text(&c2d,"->                         <-",
                            (cg2d_get_virtual_width(&c2d)/2)-woff,
                            (cg2d_get_virtual_height(&c2d)/1.50)+40);

		  
    //render to fullscreen textures
    render_textures(ticks);      


}

game_state menu_update(cg_controller *active,int currTime){

    static int inputDelayTicker=0;

    update_logo();

    if(inputDelayTicker>0){
        inputDelayTicker--;
    }


	//update blobs
	update_blobs();



    if(active!=NULL){

        if(inputDelayTicker<=0){
            
            int lastPosition=menuPosition;
            float yAxis=cg_controller_get_axis_normalized(active,SDL_GAMEPAD_AXIS_LEFTY);
           
            if(yAxis>0){
                menuPosition+=1;
            }else if(yAxis<0){
                menuPosition-=1;
            }

            if(menuPosition<0){
                menuPosition=4;
            }else if(menuPosition>4){
                menuPosition=0;
            }

            if(lastPosition!=menuPosition){
                play_audio(&menuClickSFX, 1.0,0);
                inputDelayTicker=10;
            }
        }

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true){
            switch(menuPosition){
            case 0://new game
                transition_init(STATE_GAME,currTime,123456);
                play_audio(&menuSelectSFX,1.0,0);
                return STATE_GAME_TRANSITION;
                break;
            case 1://score screen
                scores_init();
                play_audio(&menuSelectSFX,1.0,0);
                return STATE_SCORE_DISPLAY;
                break;
            case 2://about
                about_init();
                play_audio(&menuSelectSFX,1.0,0);
                return STATE_ABOUT;
                break;
            case 3://settings
                settings_init();
                play_audio(&menuSelectSFX,1.0,0);
                return STATE_SETTINGS;
                break;
            case 4://quit
                return STATE_QUIT;
                break;
            default:
                break;
            }
        }  

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_EAST)==true){
            logoTargetX=640;
            logoTargetY=260;
            play_audio(&menuBackSFX,1.0,0);
            return STATE_TITLE;
        }    
    }

	return STATE_MENU;
}