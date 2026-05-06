
static int menuPosition=0;
static char *menuOptions[5]={"New Game","Scores","About","Settings","Quit"};

void menu_init(){
	//initialise the blobs in the background
	menuPosition=0;  
    logoTargetX=640;
    logoTargetY=260; 
    logoScale=1.0;
    logoTargetScale=1.0;     
}

void menu_draw(int ticks){


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

		     float scl=(logoScale - (i*(logoScale/20)));
            float rad=logoScale/5;
            cg2d_set_scale(&c2d,rad*cg2d_sin((float)ticks*0.1)+scl,rad*cg2d_cos((float)ticks*0.1)+scl);
            cg2d_draw_image(&c2d,logoImage,logoPosX,logoPosY);
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

game_state menu_update(cg_controller *active,int currTime){

    static int inputDelayTicker=0;

    logoPosX+=(logoTargetX-logoPosX)*0.1;
    logoPosY+=(logoTargetY-logoPosY)*0.1;
    logoScale+=(logoTargetScale-logoScale)*0.1;

    if(inputDelayTicker>0){
        inputDelayTicker--;
    }

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
                play_audio(&menuClick, 1.0,0);
                inputDelayTicker=10;
            }
        }

        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true){
            switch(menuPosition){
            case 0://new game
                transition_init(STATE_GAME,currTime,123456);
                play_audio(&menuSelect,1.0,0);
                return STATE_GAME_TRANSITION;
                break;
            case 1://score screen
                scores_init();
                play_audio(&menuSelect,1.0,0);
                return STATE_SCORE_DISPLAY;
                break;
            case 2://about
                about_init();
                play_audio(&menuSelect,1.0,0);
                return STATE_ABOUT;
                break;
            case 3://settings
                settings_init();
                play_audio(&menuSelect,1.0,0);
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
            play_audio(&menuBack,1.0,0);
            return STATE_TITLE;
        }    
    }

	return STATE_MENU;
}