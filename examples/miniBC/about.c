

void about_init(){
	
    logoTargetX=640;
    logoTargetY=120;
    logoScale=0.7;
    logoTargetScale=1.0;

}

void about_draw(int ticks){


	cg2d_set_cls_colour(&c2d,0,0,0);   
         
    render_blobs(ticks);

    //draw the mini bullet candy logo
	render_logo(ticks);

	float woff=0;//=cg2d_text_width(&c2d,creditTxt)/4;
	
    //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		char *startTxt="Game by Charlie - www.charliemakesthings.com\n"     
                        "music by John Marwin - http://johnmarwin.co.nr\n\n" 
                        "How to Play\n\n"
                        "Move your ship with the WASD or Arrow Keys, mouse or the\n"
                        "left controller stick.\n\n"
                        "Use IJKL keys or right controller stick to aim\n"
                        "your shots\n\n"
                        "Hold Space, A/X on a controller or left mouse to lock\n"
                        "the direction of fire.\n\n"
                        "Good Luck!";
		woff=(cg2d_text_width(&c2d,startTxt)*sx)/(2);

		cg2d_draw_text(&c2d,startTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,240);
          
    //render to fullscreen textures
    render_textures(ticks);       


}

game_state about_update(cg_controller *active){

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

	return STATE_ABOUT;
}

