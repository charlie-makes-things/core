
float arenaRotation=360;//set this to a big number
bool doneBang,doneSpawn;
int waveTicker=0;
int waveNumber=0;

void game_init(){
	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);

	audio_fade_in_and_play(&gameMusic, 1000);
	starfield_init();

	particle_init(vWidth/2,vHeight/2,0,0,0,0,6,-0.01,1.0,320,redImage,255,255,255);

	//we'll reuse the logo position for the arena position
	logoPosX=640;
    logoPosY=260;
    logoTargetX=640;
    logoTargetY=260;
    logoScale=0;
    logoTargetScale=0;
    arenaRotation=360;

    //init player
    p1.x=640;
    p1.y=360;
    p1.xvel=p1.yvel=0;
    p1.ang=0;
    p1.active=false;
    p1.score=0;
    p1.multiplier=1;
    p1.nextMultiplier=100;
    p1.lives=3;
    p1.colRad=3;
    p1.hasLasers=false;

    waveTicker=0;
    waveNumber=1;
    doneBang=false;
    doneSpawn=false;

    arrsetcap(enemies,2000);

}

game_state game_update(cg_controller *active,SDL_Window *w,int ticks){

	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);
	//update starfield;
	starfield_update();
	//update particles
	particle_update();
	
	//spin the arena in
	logoScale+=(1.0-logoScale)*0.01;
	arenaRotation+=(0.0-arenaRotation)*0.05;
	if(arenaRotation<=0.01){
		arenaRotation=0.0;
	}

	//spawn animation particles
	if(arenaRotation<20 && arenaRotation>0.001){
		if(ticks % 2 == 0){
			//draw some particles to suggest spawning the player
			float rad=(SDL_randf()*100.0)*logoScale;
			float x=(rad*cg2d_sin(SDL_rand(360)))+vWidth/2;//(SDL_randf()*100)-50;
			float y=(rad*cg2d_cos(SDL_rand(360)))+vHeight/2;//(SDL_randf()*100)-50;
			particle_init(x,y,0,0,0,0,0.5*logoScale,-0.01*logoScale,1.0,120,ringImage,255,255,255);
		}
	}

	if(arenaRotation<120 && doneSpawn==false){
		doneSpawn=true;
		play_audio(&playerSpawn1,1.0,0);
	}

	//if ready to spawn...
	if(arenaRotation==0 && doneBang==false){
		doneBang=true;
		p1.active=true;	
		waveNumber=1;	
		play_audio(&playerSpawn2,0.4,0);
		play_audio(&playerSpawn3,0.7,0);

		//draw some explodey particles
		for(float i=0; i<360;i++){
			float rad=SDL_randf()*50;
			float ang=(float)i;
			float x=rad*cg2d_sin(ang)+vWidth/2;
			float y=rad*cg2d_cos(ang)+vHeight/2;
			float xvel=(x-(vWidth/2))*0.5;
			float yvel=(y-(vHeight/2))*0.5;
			particle_init(x,y,xvel,yvel,0,0,0.25,-0.001,1.0,SDL_rand(60)+60,redImage,255,255,255);
			
			rad=10;
			x=rad*cg2d_sin(ang)+vWidth/2;
			y=rad*cg2d_cos(ang)+vHeight/2;
			xvel=(x-(vWidth/2))*2;
			yvel=(y-(vHeight/2))*2;
			particle_init(x,y,xvel,yvel,0,0,1.0,-0.001,1.0,60,redImage,255,255,255);

		}		
	}

	//now the game is running, lat the player move around and update timers
	//so enemies can spawn, waves can change etc.
	if(p1.active==true){
		waveTicker++;

		//check wave and add eneimes 
		if(waveTicker % 100 == 0){
			float x=SDL_rand(1280);
			float y=SDL_rand(720);
			float an=SDL_randf()*360;
			float xv=5*cg2d_sin(an);
			float yv=5*cg2d_cos(an);
			enemy_init(ENEMY_GRUNT,x,y,1,0,0,spinnerImage,enemy_seek_player_spinner,enemy_add_shot_fourway_spinner,enemy_explode,enemy_jet_effect,2.0,0,10,1,100,8,5,40);
		}

		//update player and add shots
		player_update(active,ticks);
		//update bullets
		bullet_update();
		//update enemies
		enemy_update();

		//test collisions
		enemy_self_collision();
		enemy_shot_collision();

		//check game rules

	}else{


	}






	return STATE_GAME;
}

void game_draw(int ticks){
	
	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);

	double now = ((double)SDL_GetTicks()) / 4000.0;  /* convert from milliseconds to seconds. */
    float red = (float) (0.5 + 0.5 * SDL_sin(now));
    float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));

	//draw starfield
		starfield_draw(spriteLayer);
	
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
      
    //draw the arena

        cg2d_set_layer(&c2d,effectsLayer);

        cg2d_set_rotation(&c2d,arenaRotation);
        cg2d_set_scale(&c2d,logoScale,logoScale);
        cg2d_set_handle(&c2d,0.5,0.5);
        cg2d_set_alpha(&c2d,0.25);
        cg2d_set_colour(&c2d,255,0,0);
        cg2d_set_outline_width(&c2d,0.01);
        cg2d_draw_rect(&c2d,vWidth/2,vHeight/2,vWidth-20,vHeight-20,true);

        cg2d_set_alpha(&c2d,1.0);
        cg2d_set_colour(&c2d,255,255,255);
        cg2d_set_outline_width(&c2d,0.0025);
        cg2d_draw_rect(&c2d,vWidth/2,360,vWidth-30,vHeight-25,true);

    //draw the particles
        cg2d_set_layer(&c2d,effectsLayer);
        particle_draw();
        
    //if active, draw the player
        if(p1.active==true){
        	//draw player here
        	cg2d_set_layer(&c2d,spriteLayer);
        	cg2d_set_scale(&c2d,1.0*1.5,1.0*1.5);
        	cg2d_set_rotation(&c2d,p1.ang);
        	cg2d_set_colour(&c2d,255,255,255);
        	cg2d_set_image_handle(blueflatImage,0.5,0.25);
        	cg2d_draw_image(&c2d,blueflatImage,p1.x,p1.y);
        }
    //draw player bullets
        bullet_draw();

    //draw enemies
        enemy_draw();

}