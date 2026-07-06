

void title_init(bool spawnBlobs){
	//initialise the blobs in the background
	if(spawnBlobs==true){
        for(Uint32 i=0;i<BLOB_COUNT;i++){        
            blob b;
            //only need to set a few blob values
            //the rest are set during update.
            b.spd=SDL_randf();
            b.deg=SDL_randf()*360;
            b.r=b.g=b.b=255;
            float scl=SDL_randf()+0.2;
            b.sx=b.sy=scl;
            arrput(blobs,b);
        }
    }
    audio_fade_in_and_play(&menuMusic, 1000);

    logoPosX=640;
    logoPosY=260;
    logoTargetX=640;
    logoTargetY=260;
    logoScale=1.0;
    logoTargetScale=1.0;

}

void title_draw(int ticks){

	cg2d_set_cls_colour(&c2d,0,0,0);
    
    render_blobs(ticks);

    //draw the mini bullet candy logo
    render_logo(ticks);

	//draw the by charlie text
		cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5,0.5);
		char *creditTxt="Charlie - 2026 with music by John";
		float woff=cg2d_text_width(&c2d,creditTxt)/4;
		cg2d_draw_text(&c2d,creditTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)-50);
   
   //draw press space or any button to start
        cg2d_set_layer(&c2d, fontLayer);
		cg2d_set_scale(&c2d,0.5+(0.01*cg2d_cos(ticks*3)),0.5+(0.01*cg2d_cos(ticks*3)));
		float sx=0,sy=0;
		cg2d_get_scale(&c2d,&sx,&sy);
		char *startTxt="Press Space or a Controller Button to Start";
		woff=(cg2d_text_width(&c2d,startTxt)*sx)/(2);

		cg2d_draw_text(&c2d,startTxt,(cg2d_get_virtual_width(&c2d)/2)-woff,cg2d_get_virtual_height(&c2d)/1.45);
          
    //render to fullscreen textures
    render_textures(ticks);     


}

game_state title_update(cg_controller *active){

    update_logo();
    
	//update blobs
	update_blobs();

    if(active!=NULL){
        if(cg_controller_get_button_released(active,SDL_GAMEPAD_BUTTON_SOUTH)==true){
            play_audio(&menuSelectSFX, 1.0,0);
            menu_init();
            return STATE_MENU;
        }


    }

	return STATE_TITLE;
}