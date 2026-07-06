//prototypes
int enemy_jet_effect(enemy *e);
int enemy_glow_effect_green(enemy *e);
int enemy_explode(enemy *e);
int enemy_explode_small(enemy *e);


void enemy_init(enemy_type type,float x,float y, float xvel,float yvel,float ang,
				cg2d_image **img,int imgCnt,float hndx,float hndy,int (* update)(struct enemy*),int (* add_shots)(struct enemy*),
				int (* die)(struct enemy*),int (* add_effects)(struct enemy*),float scale,
				float angOffset,float colRad,int energy,int score,int shotRate,int shotSpeed,float spinSpeed,float knockbackScale){
	enemy e;
	e.type=type;
	e.x=x;
	e.y=y;
	e.xvel=xvel;
	e.yvel=yvel;
	e.txvel=xvel;
	e.tyvel=yvel;
	e.targetSpd=0.0;
	e.targetAng=0.0;
	e.ang=ang;
	e.x2=x;
	e.y2=y;
	e.timer1=0;
	
	e.update=update;
	e.add_shots=add_shots;
	e.die=die;
	e.add_effects=add_effects;
	e.scale=scale;
	for(int i=0 ; i<imgCnt;i++){
		e.img[i]=img[i];
	}
	e.hndlx=hndx;
	e.hndly=hndy;
	e.imgCnt=imgCnt;
	e.currentFrame=0;
	e.frameDelay=8;
	e.angOffset=angOffset;
	e.colRad=colRad;
	e.energy=energy;
	e.score=score;
	e.shotRate=shotRate;
	e.shotSpeed=shotSpeed;
	e.spinSpeed=spinSpeed;
	e.knockbackScale=knockbackScale;
	e.knockbackX=0.0;
	e.knockbackY=0.0;
	e.bossPhase=0;
	arrput(enemies,e);
}

void enemies_clear(){
	// for(int i=0;i<arrlen(enemies);i++){
	// 	arrdelswap(enemies,i);
	// }
	//arrdeln(enemies, int p, int n);
	arrfree(enemies);
	enemies=NULL;
}

int enemy_update(){
	bool collidedWithPlayer=false;
	for (int i=0;i<arrlen(enemies);i++){
		enemy *e=&enemies[i];
		
		e->timer1++;

		e->update(e);

		e->knockbackX=0.0;
		e->knockbackY=0.0;

		if(e->add_shots!=NULL){
			e->add_shots(e);
		}
		if(e->add_effects!=NULL){
			e->add_effects(e);
		}

		if(e->timer1>0 && e->timer1 % e->frameDelay==0){
			e->currentFrame++;
			if(e->currentFrame>=e->imgCnt){
				e->currentFrame=0;
			}
		}


		if(e->type==ENEMY_SHOT){
			if(e->x<-20 || e->x>1300 || e->y<-20 || e->y>740){
				arrdelswap(enemies,i);
			}
		}else{
			//clamp to boundary - not needed in miniBC
			
			// if(e->x<20){
			// 	e->x=20;
			// }
			// if(e->x>1260){
			// 	e->x=1260;
			// }
			// if(e->y<20){
			// 	e->y=20;
			// }
			// if(e->y>700){
			// 	e->y=700;
			// }
		}

		//test to see if the enemy is in contact with the player
		if(d_playerCollisions==true){
			if(circles_intersect(e->x,e->y,e->colRad,p1.x,p1.y,p1.colRad)==true){
				//printf("collision\n" );
				collidedWithPlayer=true;
			}
		}		
	}

	return collidedWithPlayer;

}

void enemy_self_collision(){
	
	for(int i=0;i<arrlen(enemies);i++){
    	enemy *e=&enemies[i];
    	if(e->type!=ENEMY_SHOT && e->type!=ENEMY_SHURIKEN && e->type!=ENEMY_GENERATOR){
	    	for(int o=0;o<arrlen(enemies);o++){
	    		enemy *e2=&enemies[o];
	    		//make sure they aren't the same enemy
	    		if(e!=e2){
	    			if(e2->type!=ENEMY_SHOT && e2->type!=ENEMY_SHURIKEN && e2->type!=ENEMY_GENERATOR){
	    				//if the collision circles overlap push the enemies away from each other.
	    				//this isn't a perfect solution, but it's good enough for this game.
	    				if(circles_intersect(e->x,e->y,e->colRad, e2->x,e2->y,e2->colRad)){	    					
	    					if(e2->type==ENEMY_GEN_GRUNT && e->type==ENEMY_GEN_GRUNT){
	    						
		    					float xdif=e->x-p1.x;
		    					float ydif=e->y-p1.y;
		    					float nx,ny;
		    					normalize(xdif,ydif,&nx,&ny);
		    					e->x-=nx*2;
		    					e->y-=ny*2;
		    					e2->x+=nx;
		    					e2->y+=ny;
	    					}
	    					float xdif=e->x-e2->x;
	    					float ydif=e->y-e2->y;
	    					float nx,ny;
	    					normalize(xdif,ydif,&nx,&ny);
	    					e->x+=nx;
	    					e->y+=ny;
	    					e2->x-=nx;
	    					e2->y-=ny;	
	    					   					
	    				}
	    			}
	    		}
	    	}	
    	}
    	
    }
}

void enemy_shot_collision(){
	for(int i=0;i<arrlen(enemies);i++){
		enemy *e=&enemies[i];
		if(e->type==ENEMY_SHOT){
			continue;
		}
		if(e->x<0 && e->x<1280 && e->y<0 && e->y<720){
			continue;
		}
		for(int o=0;o<arrlen(player_bullets);o++){
			player_bullet *b=&player_bullets[o];
			
			if(circles_intersect(e->x,e->y,e->colRad,b->x,b->y,b->colRad)){
				e->energy-=b->energy;

				if(e->energy>0){
					audio_pan(&hitSFX,e->x);
					play_audio(&hitSFX,1.0,0);
				}
				
				float nx,ny;
				normalize((e->x-b->x),(e->y-b->y),&nx,&ny);

				e->knockbackX=nx;
				e->knockbackY=ny;

				arrdelswap(player_bullets,o);			

			}
			
		}
		if(e->energy<=0){
			p1.score+=e->score*p1.multiplier;
			tripleCount++;
			bonusCount++;
			lifeCount++;
			killCount++;
			if(e->die!=NULL){
				e->die(e);
				audio_pan(&zapSFX,e->x);
				play_audio(&zapSFX,1.0,0);
			}
			arrdelswap(enemies,i);
			continue;
		}

	}
}

void enemy_kill_first_num(int cnt){

	for(int i=0;i<cnt && i<arrlen(enemies);i++){
		enemy *e=(enemy*)&enemies[i];
		if(e->die!=NULL){
			e->die(e);
			audio_pan(&zapSFX,e->x);
			play_audio(&zapSFX,1.0,0);
		}else{
			if(arrlen(enemies)<100){
			enemy_explode(e);
			}else{
				enemy_explode_small(e);
			}
			audio_pan(&zapSFX,e->x);
			play_audio(&zapSFX,1.0,0);
		}
		arrdelswap(enemies,i);
		continue;
	}

}

void enemy_draw(){
	for (int i=0;i<arrlen(enemies);i++){
		enemy *e=&enemies[i];
		if(e->type==ENEMY_SHOT || e->type ==ENEMY_EVIL_SHOT || e->type==ENEMY_BOSS1 || e->type==ENEMY_BOSS2){
			cg2d_set_layer(&c2d,effectsLayer);	
		}else{
			cg2d_set_layer(&c2d,spriteLayer);
		}
		cg2d_set_scale(&c2d,e->scale,e->scale);
		cg2d_set_rotation(&c2d,e->ang+e->angOffset);
		cg2d_set_colour(&c2d,255,255,255);
		cg2d_set_alpha(&c2d,1.0);
		cg2d_set_image_handle(e->img[e->currentFrame],e->hndlx,e->hndly);
		cg2d_draw_image(&c2d,e->img[e->currentFrame],e->x,e->y);

		if(d_showCollisionCircles==true){
	    	cg2d_set_colour(&c2d,255,255,255);
	    	cg2d_set_alpha(&c2d,1.0);
	    	cg2d_set_scale(&c2d,1.0,1.0);
	    	cg2d_set_rotation(&c2d,0);
	    	cg2d_set_layer(&c2d,spriteLayer);
	    	cg2d_draw_oval(&c2d,e->x,e->y,e->colRad,e->colRad,false);
	    }
	}
}

//seek functions
int enemy_move_straight(enemy *e){
	
	if(p1.active==true){
		e->xvel+=(e->txvel-e->xvel)*0.1;
		e->yvel+=(e->tyvel-e->yvel)*0.1;
		//e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	}else{
		e->xvel+=(0-e->xvel)*0.05;
		e->yvel+=(0-e->yvel)*0.05;
	}	

	e->x2=e->x;
	e->y2=e->y;
	
	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x+=e->xvel;
	e->y+=e->yvel;
	
	return 0;
}



int enemy_seek_player(enemy *e){
	
	if(p1.active==true){
		e->xvel+=(e->txvel-e->xvel)*0.1;
		e->yvel+=(e->tyvel-e->yvel)*0.1;
		e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	}else{
		e->xvel+=(0-e->xvel)*0.05;
		e->yvel+=(0-e->yvel)*0.05;		
	}
	
	e->x2=e->x;
	e->y2=e->y;

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	return 0;
}






int enemy_seek_player_wiggle(enemy *e){

	if(p1.active==true){
		e->xvel+=(e->txvel-e->xvel)*0.1;
		e->yvel+=(e->tyvel-e->yvel)*0.1;
		e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
		e->ang+=40*cg2d_sin(e->timer1*2);
	}else{
		e->xvel+=(0-e->xvel)*0.05;
		e->yvel+=(0-e->yvel)*0.05;		
	}
	
	e->x2=e->x;
	e->y2=e->y;

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	return 0;
}

int enemy_seek_player_pulse(enemy *e){

	if(p1.active==true){
		e->targetSpd=e->xvel+((e->xvel*0.5)*cg2d_sin(e->timer1*6));
		e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	}else{
		e->targetSpd+=(0.0-e->targetSpd)*0.1;
	}

	e->x2=e->x;
	e->y2=e->y;

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x=(e->targetSpd*cg2d_cos(e->ang))+e->x;
	e->y=(e->targetSpd*cg2d_sin(e->ang))+e->y;
	return 0;
}

int enemy_orbit_seek_player(enemy *e){

	if(p1.active==true){
		e->targetSpd=e->xvel+((e->xvel*0.5)*cg2d_sin(e->timer1*6));
		e->ang=cg2d_atan2(e->y-p1.y,e->x-p1.x)+130;
	}else{
		e->targetSpd+=(0.0-e->targetSpd)*0.1;
	}

	e->x2=e->x;
	e->y2=e->y;	

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x=e->targetSpd*cg2d_cos(e->ang)+e->x;
	e->y=e->targetSpd*cg2d_sin(e->ang)+e->y;
	return 0;	
}

int enemy_orbit_point(enemy *e){
	//yvel is used as an offset here
	
	if(p1.active==true){
		e->ang+=cg2d_sin(e->timer1*10)+e->xvel;
	}	

	e->x2=(230+e->yvel)*cg2d_sin(e->ang)+640;
	e->y2=(230+e->yvel)*cg2d_cos(e->ang)+360;

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	e->x+=(e->x2-e->x)*0.05;
	e->y+=(e->y2-e->y)*0.05;

	return 0;

}



int enemy_seek_player_XY(enemy *e){

	if(p1.active==true){
		e->xvel+=0.05;
		if(e->xvel>e->txvel){
			e->xvel=e->txvel;
		}
		
		
	}else{
		e->xvel+=(0-e->xvel)*0.05;
		e->yvel+=(0-e->yvel)*0.05;	
		
	}	

	e->x2=e->x;
	e->y2=e->y;

	e->x+=e->knockbackX * e->knockbackScale;
	e->y+=e->knockbackY * e->knockbackScale;

	//use yvel to see if we are seeking on the x or y axis
	if(SDL_fabs(e->yvel)<=0.0){
		if(e->x<p1.x){
			e->x+=e->xvel;
			e->ang=270;
		}else{
			e->x-=e->xvel;
			e->ang=90;
		}
		if(e->x>p1.x-10 && e->x<p1.x+10){
			e->xvel=1;
			e->yvel=1;
		}
	}else{
		if(e->y<p1.y){
			e->y+=e->xvel;
			e->ang=0;
		}else{
			e->y-=e->xvel;
			e->ang=180;
		}
		if(e->y>p1.y-10 && e->y<=p1.y+10){
			e->xvel=1;
			e->yvel=-0.0;
		}
	}

	
	return 0;
}


//add shots functions
int enemy_add_shot_normal(enemy *e){

	if(e->timer1 % e->shotRate==0){
		float xv=e->shotSpeed*cg2d_cos(e->ang);
		float yv=e->shotSpeed*cg2d_sin(e->ang);
		enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
				&enemyBulletImage,1,0.5,0.5,enemy_move_straight,NULL,
				NULL,enemy_jet_effect,1.0,
				e->ang,2.5,1,0,0,0,0,0.0);
		audio_pan(&enemyShotSFX,e->x);
		play_audio(&enemyShotSFX,0.7,0);
	}

	return 0;
}

int enemy_add_shot_orbiter(enemy *e){

	if( e->timer1>120 && e->timer1 % e->shotRate==0){

		float xv=p1.x-e->x;
		float yv=p1.y-e->y;
		float nx,ny;
		normalize(xv,yv,&nx,&ny);
		xv=nx*e->shotSpeed;
		yv=ny*e->shotSpeed;
		enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
				&greenImage,1,0.5,0.5,enemy_move_straight,NULL,
				NULL,NULL,0.6,
				180,2.5,1,0,0,0,0,0.0);

		audio_pan(&enemyShotSFX,e->x);
		play_audio(&enemyShotSFX,0.7,0);
	}

	return 0;

}

int enemy_add_shot_burst(enemy *e){

	if(e->timer1 > e->shotRate){
		if(e->timer1 %6==0){
			float xv=e->shotSpeed*cg2d_cos(e->ang+(SDL_randf()*2)-2);
			float yv=e->shotSpeed*cg2d_sin(e->ang+(SDL_randf()*2)-2);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					&enemyBulletImage,1,0.5,0.5,enemy_move_straight,NULL,
					NULL,enemy_jet_effect,1.0,
					e->ang,2.5,1,0,0,0,0,0.0);
			
			audio_pan(&enemyShotSFX,e->x);
			play_audio(&enemyShotSFX,0.7,0);
		}
		if(e->timer1>e->shotRate+36){
			e->timer1=0;
		}
	}

	return 0;
}





//effects functions
int enemy_jet_effect(enemy *e){

	if(e->timer1 % 2 ==0){
		float xv=(e->x2-e->x)*2;
		float yv=(e->y2-e->y)*2;
		
		particle_init(e->x,e->y,xv,yv,0,0,0,0,0.25,-0.01,1.0,60,blobImage,255,150,0,0);
	}
	return 0;
}



int enemy_glow_effect_green(enemy *e){

	if(e->timer1 % 2 ==0){
		particle_init(e->x, e->y,0, 0,0,0,0,0,
				   0.8, -0.03,1.0, 10,blobImage,
				   50, 255, 50,0);
	}

	return 0;

}



int enemy_glow_effect_red_pulse(enemy *e){
	if(e->timer1 % 2 ==0){
		particle_init(e->x, e->y,0, 0,0,0,0,0,
				   0.8+(0.2*cg2d_cos(e->timer1*4)), -0.03,1.0, 10,blobImage,
				   255, 50, 50,0);
	}
	enemy_jet_effect(e);
	return 0;
}






int enemy_spark_effect(enemy *e){

	for(int i=0;i<8;i++){
		float rad=16;
		float ang=SDL_randf()*360;
		float x=rad*cg2d_cos(ang)+e->x;
		float y=rad*cg2d_sin(ang)+e->y;

		float xv=x-e->x;
		float yv=y-e->y;

		
		particle_init(e->x, e->y,xv, yv,0,0,ang+90,0,
				   1.0, -0.05,1.0, 30,zapImage,
				   255, 255, 255,0);
	}

	if(e->timer1 % 8 ==0){
		audio_pan(&electricSFX,e->x);
		play_audio(&electricSFX,1.0,0);
	}

	return 0;

}

//death functions
int enemy_explode_small(enemy *e){
	for(int i=0;i<360;i+=32){
		float rad=6.0;
		float xv=(SDL_randf()*rad)-(rad*0.5);//rad*cg2d_sin((float)i);
		float yv=(SDL_randf()*rad)-(rad*0.5);//rad*cg2d_cos((float)i);
		
		particle_init(e->x,e->y,xv*2,yv*2,0,0,0,0,0.4,-0.001,1.0,60,blobImage,255,0,0,0);
		particle_init(e->x,e->y,xv*2,yv*2,0,0,0,0,0.2,-0.001,1.0,60,blobImage,255,255,255,0);
		//particle_init(e->x,e->y,xv,yv,0,0,0,0,0.1,-0.001,1.0,SDL_rand(30)+60,blobImage,255,255,255,0,NULL);
				
	}
	particle_init(e->x, e->y,0, 0,0,0,0,0,
				1.0, -0.063,1.0, 20,blobImage,
				   255, SDL_rand(50), 0,0);
	return 0;
}

int enemy_explode(enemy *e){

	for(float i=0; i<360;i+=2){
			float rad=SDL_randf()*50;
			float ang=(float)i;
			float x=rad*cg2d_sin(ang)+e->x;
			float y=rad*cg2d_cos(ang)+e->y;
			float xvel=(x-(e->x))*0.1;
			float yvel=(y-(e->y))*0.1;
			

			particle_init(x,y,xvel,yvel,0,0,0,0,SDL_randf()*0.2,-0.001,1.0,SDL_rand(30)+30,blobImage,255,255,255,0);
			
			if((int)i%8==0){
				rad=10;
				x=rad*cg2d_sin(ang)+e->x;
				y=rad*cg2d_cos(ang)+e->y;
				xvel=(x-(e->x))*(SDL_randf()+1);
				yvel=(y-(e->y))*(SDL_randf()+1);
				particle_init(x,y,xvel,yvel,0,0,0,0,0.5,-0.025,1.0,20,blobImage,255,255,0,0);
				
			}

	}	
	particle_init(e->x, e->y,0, 0,0,0,0,0,
					3, -0.063,1.0, 20,blobImage,
				   255, SDL_rand(50), 0,0);
	return 0;
}


int enemy_explode_medium(enemy *e){
	
	for(int r=0;r<4;r++){
		float offset=(SDL_randf()*100) -50;
			

		for(float i=0; i<360;i+=2){
				
				float rad=SDL_randf()*50;
				float ang=(float)i;
				float x=rad*cg2d_sin(ang)+e->x;
				float y=rad*cg2d_cos(ang)+e->y;
				float xvel=(x-(e->x))*0.1;
				float yvel=(y-(e->y))*0.1;				

				int cr=255,cg=150+SDL_rand(100),cb=SDL_rand(255);
				if((int)i%2==0){
					particle_init(x+offset,y+offset,xvel,yvel,0,0,0,0,SDL_randf()*0.2,-0.001,1.0,SDL_rand(30)+30,blobImage,cr,cg,cb,r*5);
				}

				if((int)i==0){
					particle_init(x+offset,y+offset,0,0,0,0,0,0,1.0,0.25,1.0,10,blobImage,cr,cg,cb,r*5);
					cue_sound(&zapSFX,0.6,0,r*5,e->x);
				}

				if((int)i%2==0 && r==0){
					rad=SDL_rand(10);
					x=rad*cg2d_sin(ang)+e->x;
					y=rad*cg2d_cos(ang)+e->y;
					xvel=(x-(e->x))*2;
					yvel=(y-(e->y))*2;
					particle_init(x,y,xvel,yvel,0,0,0,0,0.5,-0.05,1.0,10,blobImage,255,SDL_rand(255),0,0);
				}

		}
			
	}

	//center glow
		particle_init(e->x, e->y,0, 0,0,0,0,0,
				   2, -0.033,1.0, 60,blobImage,
				   255, SDL_rand(50), 0,0);

	return 0;
}

int enemy_explode_add_shot_splitter(enemy *e){

	audio_pan(&enemyShotSFX,e->x);
	play_audio(&enemyShotSFX,0.7,0);

	enemy_explode_medium(e);

	float ang=e->ang-45;
	for(int i=0;i<3;i++){
		float xv=e->shotSpeed*cg2d_cos(ang);
		float yv=e->shotSpeed*cg2d_sin(ang);
		enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
				&enemyBulletImage,1,0.5,0.5,enemy_move_straight,NULL,
				NULL,enemy_jet_effect,1.5,
				ang,2.5,1,0,0,0,0,0.0);
		ang+=45;
	}

	return 0;
}



int enemy_explode_large(enemy *e){
	for(int r=0;r<20;r++){
		float offset=(SDL_randf()*150) -75;
			

		for(float i=0; i<360;i+=2){
				
				float rad=SDL_randf()*50;
				float ang=(float)i;
				float x=rad*cg2d_sin(ang)+e->x;
				float y=rad*cg2d_cos(ang)+e->y;
				float xvel=(x-(e->x))*0.1;
				float yvel=(y-(e->y))*0.1;				

				int cr=255,cg=150+SDL_rand(100),cb=SDL_rand(255);
				if((int)i%2==0){
					particle_init(x+offset,y+offset,xvel,yvel,0,0,0,0,SDL_randf()*0.2,-0.001,1.0,SDL_rand(30)+30,blobImage,cr,cg,cb,r*5);
				}

				if((int)i==0){
					particle_init(x+offset,y+offset,0,0,0,0,0,0,1.0,0.25,1.0,10,blobImage,cr,cg,cb,r*5);
					cue_sound(&zapSFX,0.6,0,r*5,e->x);
				}

				if((int)i%2==0 && r==0){
					rad=10;
					x=rad*cg2d_sin(ang)+e->x;
					y=rad*cg2d_cos(ang)+e->y;
					xvel=(x-(e->x))*2;
					yvel=(y-(e->y))*2;
					particle_init(x,y,xvel,yvel,0,0,0,0,1.0,-0.01,1.0,60,blobImage,255,255,0,0);
				}

		}
			
	}

	//center glow
		particle_init(e->x, e->y,0, 0,0,0,0,0,
				   6, -0.033,1.0, 180,blobImage,
				   255, SDL_rand(50), 0,0);

	return 0;
}


int enemy_explode_electrode(enemy *e){
	//center glow
	particle_init(e->x, e->y,0, 0,0,0,0,0,
				   4, -0.033,1.0, 180,blobImage,
				   50, SDL_rand(50), 255,0);

	for(int r=0;r<6;r++){
		float offset=(SDL_randf()*150) -75;
			

		for(float i=0; i<360;i+=4){
				
				float rad=SDL_randf()*50;
				float ang=(float)i;
				float x=rad*cg2d_sin(ang)+e->x;
				float y=rad*cg2d_cos(ang)+e->y;
				float xvel=(x-(e->x))*0.1;
				float yvel=(y-(e->y))*0.1;				

				int cr=255,cg=150+SDL_rand(100),cb=SDL_rand(255);
				if((int)i%4==0){
					particle_init(x+offset,y+offset,xvel,yvel,0,0,0,0,SDL_randf()*0.2,-0.001,1.0,SDL_rand(30)+30,blobImage,cr,cg,cb,r*5);
				}

				if((int)i==0){
					particle_init(x+offset,y+offset,0,0,0,0,0,0,1.0,0.25,1.0,10,blobImage,cr,cg,cb,r*5);
					cue_sound(&zapSFX,1.0,0,r*5,e->x);
				}

				if((int)i%2==0 && r==0){
					rad=10;
					x=rad*cg2d_sin(ang)+e->x;
					y=rad*cg2d_cos(ang)+e->y;
					xvel=(x-(e->x))*2;
					yvel=(y-(e->y))*2;
					particle_init(x,y,xvel,yvel,0,0,0,0,1.5,-0.01,1.0,60,blobImage,255,255,255,0);
				}

				if((int)i%2==0 && r<5){
					rad=6;
					x=rad*cg2d_cos(ang)+e->x;
					y=rad*cg2d_sin(ang)+e->y;
					xvel=(x-(e->x))*SDL_randf()*4;
					yvel=(y-(e->y))*SDL_randf()*4;
					float ofst=(SDL_randf()*150)-75;
					float ofst2=(SDL_randf()*150)-75;
					particle_init(x+ofst,y+ofst2,xvel,yvel,0,0,ang+90,0,0.8,-0.07,1.0,SDL_rand(60)+20,zapImage,255,255,255,SDL_rand(30));
				}

		}
			
	}
	enemy_explode_large(e);
	return 0;
}
