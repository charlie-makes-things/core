






void game_init(SDL_Window *win){
	
	g_pressed_pause=false;

	audio_fade_in_and_play(&gameMusic, 1000);
	starfield_init();

    //init player
    player_reset_game();

    phase=PHASE_SETUP;

    waveTicker=0;
    waveNumber=1;
    waveResetTime=0;
    gameComplete=false;
    camOffsetX=0.0;
	camOffsetY=0.0;  

	tripleCount=0;
	bonusCount=0;
	lifeCount=0;
	killCount=0;
	deathTicker=0;
	camOffsetX=0.0;
	camOffsetY=0.0;
	spawnTicker=0;

	messages_clear();
	enemies_clear();
	bullets_clear();
	bonuses_clear();
	particles_clear();

	cg2d_layer_reset_transform(&c2d,spriteLayer);
	cg2d_layer_reset_transform(&c2d,effectsLayer);

    arrsetcap(enemies,2000);

    SDL_SetWindowRelativeMouseMode(win, true);

}

game_state game_update(cg_controller *active,SDL_Window *w,int ticks){


	
	//update particles
	particle_update();
	
	switch(phase){
		case PHASE_SETUP:
			//reset camera etc.
			camOffsetX+=(0.0-camOffsetX)*0.01;
			camOffsetY+=(0.0-camOffsetY)*0.01;
			//reset counters
			waveTicker=0;
			waveNumber=1;
			waveResetTime=0;
			deathTicker=0;
			spawnTicker=0;
			gameComplete=false;
			//populate enemy list
			//reset player positions etc.
			player_reset_level();
			bonusCount=0;
			tripleCount=0;
			lifeCount=0;
			killCount=0;
			deathTicker=0;
			p1.multiplier=1;
			//save data/checkpoints reached
			//move to warp phase
			phase=PHASE_WARP;
			break;
		case PHASE_WARP:
			//reset camera etc.
			camOffsetX+=(0.0-camOffsetX)*0.01;
			camOffsetY+=(0.0-camOffsetY)*0.01;
			//play spawn sound
			play_audio(&playerSpawn1SFX,1.0,0);						
			//change phase to spawn
			phase=PHASE_SPAWN;
			break;
		case PHASE_SPAWN:
			//reset camera etc.
			camOffsetX+=(0.0-camOffsetX)*0.01;
			camOffsetY+=(0.0-camOffsetY)*0.01;
			//setup player			
			player_reset_level();
			//process spawn animation
			spawnTicker++;
			if(spawnTicker==1){
				//add particles for spawn animation
				player_spawn_particles();
				message_init("Get Ready!",sizeof(char)*16,640,400,120,1);
			}			
			//reset level timer. this is set by the level in level.c
			waveTicker=waveResetTime;			
			//spawn the player once the animation is done
			if(spawnTicker==120){
				cg2d_layer_reset_transform(&c2d,spriteLayer);
				cg2d_layer_reset_transform(&c2d,effectsLayer);			
				p1.active=true;
				//play sfx
				play_audio(&playerSpawn2SFX,1.0,0);
				//change phase to play
				phase=PHASE_PLAY;
			}
			break;
		case PHASE_PLAY:
			waveTicker++;
			//main game loop
			
			//update bonuses
			bonus_update();
			//update player and add shots
			player_update(active,ticks);
			//update bullets
			bullet_update();
			//update enemies
			bool collisionWithPlayer=enemy_update();			
			//get level data for miniBC
			miniBC_get_level_data();
			//test collisions
			enemy_self_collision();
			enemy_shot_collision();

			//move the camera
			camOffsetX=-p1.xvel*0.05;
			camOffsetY=-p1.yvel*0.05;

			//player death
			if(collisionWithPlayer==true){
				p1.active=false;
				p1.multiplier=1;
				p1.lives--;
				p1.threewayTimer=0;
				p1.streak=0;
				//be nice and give the player a triple shot sooner
				tripleCount=25;
				bonusCount=0;
				deathTicker=0;	
				spawnTicker=0;					
				//kill bonuses
				bonus_kill_all();
				audio_pan(&deathSFX,p1.x);
				play_audio(&deathSFX,1.0,0);
				float dummy;
				cg2d_layer_get_translation(&c2d,spriteLayer,&camOffsetX,&camOffsetY,&dummy);
				camOffsetX=-camOffsetX*0.1;
				camOffsetY=-camOffsetY*0.1;
				//how many enemies to kill per frame
				onDeathEnemyKillPerTick=10;
				if(onDeathEnemyKillPerTick<=0){
					onDeathEnemyKillPerTick=1;
				}
				//spawn player death particles
				player_die();
				if(p1.lives<0){
					phase=PHASE_COMPLETE;
					message_init("GAME OVER!",sizeof(char)*11,640,400,120,1);
				}else{
					phase=PHASE_DIE;
					message_init("Oh Dear!",sizeof(char)*9,640,400,120,1);
				}
			}

			//game completion rules
			if(gameComplete==true){
				//fade out or something??
				if(waveTicker>(waveResetTime+120)){
					message_init("Game Complete! Nice One!",sizeof(char)*32,640,400,120,1);
					phase=PHASE_COMPLETE;
				}
			}

			//spawn bonuses
			if(tripleCount >=30){
				tripleCount-=30;
				bonus_init_offscreen(BONUS_TRIPLE);
			}
			if(lifeCount>=200){
				lifeCount-=200;
				bonus_init_offscreen(BONUS_LIFE);
			}
			if(bonusCount>=20){
				bonusCount-=20;
				bonus_init_offscreen(BONUS_SCORE);
			}
			//increase multiplier.
			if(killCount>=100){
				killCount-=100;
				p1.multiplier++;
			}

			break;
		case PHASE_DIE:
			//reset camera etc.
			camOffsetX+=(0.0-camOffsetX)*0.1;
			camOffsetY+=(0.0-camOffsetY)*0.1;
			
			//update timers
			waveTicker=waveResetTime;
			deathTicker++;
			//continue to update enemies so they dont stop dead.
			enemy_update();
			bullet_update();
			bonus_kill_all();//in case any are spawned by enemies??

			//kill a few enemies
			if(deathTicker>30){
				enemy_kill_first_num(onDeathEnemyKillPerTick);
			}
			//if there are still enemies left, kill the rest
			if(deathTicker==180){
				enemy_kill_first_num(arrlen(enemies)+100);
			}
			//change phase to spawn
			if(deathTicker==220){
				player_reset_level();
				//play spawn sounds
				play_audio(&playerSpawn1SFX,1.0,0);
					
				phase=PHASE_SPAWN;
			}
			break;
		case PHASE_COMPLETE:
			//reset camera etc.
			camOffsetX+=(0.0-camOffsetX)*1;
			camOffsetY+=(0.0-camOffsetY)*1;
			//level is complete
			enemy_update();
			bullet_update();
			bonus_update();			
			deathTicker++;
			//kill a few enemies
			if(deathTicker>30){
				enemy_kill_first_num(onDeathEnemyKillPerTick);
			}
			//if there are still enemies left, kill the rest
			if(deathTicker==120){
				enemy_kill_first_num(arrlen(enemies)+100);
			}
			//change phase to score entry
			if(deathTicker>=180){
				if(p1.score<highScoreTable[arrlen(highScoreTable)-1].score){
					transition_init(STATE_TITLE,ticks,p1.score);
				}else{
					transition_init(STATE_SCORE_ENTRY,ticks,p1.score);
				}				
				SDL_SetWindowRelativeMouseMode(w, false);
				return STATE_GAME_TRANSITION;
			}
			break;
		case PHASE_END:
			//show appropriate message - game over/complete
			//warp to transition state.
			SDL_SetWindowRelativeMouseMode(w,false); 
			break;
		default:
			break;

	}

	//update camera
	cg2d_layer_translate(&c2d,spriteLayer,camOffsetX,camOffsetY);
	cg2d_layer_translate(&c2d,effectsLayer,camOffsetX,camOffsetY);
	
	//update starfield;
	starfield_update();
	
	//update messages
	message_update();

	return STATE_GAME;
}

void game_draw(int ticks,game_state state){
	
	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);

	//draw starfield
		starfield_draw(spriteLayer);
	
	//render textures
		render_textures(ticks);
	
    //draw the arena

        cg2d_set_layer(&c2d,effectsLayer);

        cg2d_set_rotation(&c2d,0);
        cg2d_set_scale(&c2d,1.0,1.0);
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

   
    //draw bonuses
        bonus_draw();
        
    //if active, draw the player
       player_draw();

    //draw player bullets
        bullet_draw();

    //draw enemies
        enemy_draw();

    //draw messages
        if(state==STATE_GAME){
	        message_draw();
	    }

    //draw the hud
        cg2d_set_layer(&c2d,hudLayer);
        cg2d_set_scale(&c2d,1.0,1.0);
        cg2d_set_rotation(&c2d,20*cg2d_sin(ticks*3));
        cg2d_set_alpha(&c2d,1.0);
        cg2d_set_colour(&c2d,255,255,255);
        cg2d_set_image_handle(blueflatImage,0.5,0.5);
	    cg2d_draw_image(&c2d,blueflatImage,50,120);

		cg2d_set_rotation(&c2d,0);
		cg2d_set_layer(&c2d,fontLayer);
		cg2d_set_handle(&c2d,0,0);
		cg2d_set_scale(&c2d,0.4,0.4);
		char lives[10];
		int lvs=p1.lives;
		if(lvs<0){lvs=0;}
		SDL_snprintf(lives,10,"x %d",lvs);
		cg2d_draw_text(&c2d,lives, 70,105);


		char score[128];
		#ifdef __WIN32
		SDL_snprintf(score,128,"Score: %lld x multi: %d",p1.score,p1.multiplier);
		#else
		SDL_snprintf(score,128,"Score: %ld x multi: %d",p1.score,p1.multiplier);
		#endif
		cg2d_draw_text(&c2d,score,50,30);
		char level[128];
		SDL_snprintf(level,128,"Level: %d",waveNumber);
		cg2d_draw_text(&c2d,level,50,55);
		
		char streak[10];
		SDL_snprintf(streak,10,"Streak: %d",p1.streak);
		cg2d_draw_text(&c2d,streak,50,80);

}