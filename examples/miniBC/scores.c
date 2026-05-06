

void scores_init(){
	
    logoTargetX=640;
    logoTargetY=100;
    logoScale=0.5;
    logoTargetScale=0.5;

}

void scores_draw(int ticks){


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
            SDL_snprintf(str_score,sizeof(str_score),"%ld",highScoreTable[i].score);
            woff=cg2d_text_width(&c2d,str_score)*sx;
            cg2d_draw_text(&c2d,str_score,880-woff,230+i*40);
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

game_state scores_update(cg_controller *active){

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
    if(active!=NULL){
        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true ||
           cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_EAST)==true ){
            play_audio(&menuSelect, 1.0,0);
            menu_init();
            return STATE_MENU;
        }


    }

    return STATE_SCORE_DISPLAY;
}