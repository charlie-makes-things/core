//prototypes
int enemy_jet_effect(enemy *e);

void enemy_init(enemy_type type,float x,float y, float xvel,float yvel,float ang,
				cg2d_image *img,int (* update)(struct enemy*),int (* add_shots)(struct enemy*),
				int (* die)(struct enemy*),int (* add_effects)(struct enemy*),float scale,
				float angOffset,float colRad,int energy,int score,int shotRate,int shotSpeed,float spinSpeed){
	enemy e;
	e.type=type;
	e.x=x;
	e.y=y;
	e.xvel=xvel;
	e.yvel=yvel;
	e.ang=ang;
	e.x2=x;
	e.y2=y;
	e.timer1=0;
	e.timer2=0;
	e.update=update;
	e.add_shots=add_shots;
	e.die=die;
	e.add_effects=add_effects;
	e.scale=scale;
	e.img=img;
	e.angOffset=angOffset;
	e.colRad=colRad;
	e.energy=energy;
	e.score=score;
	e.shotRate=shotRate;
	e.shotSpeed=shotSpeed;
	e.spinSpeed=spinSpeed;
	arrput(enemies,e);
}

void enemy_update(){
	for (int i=0;i<arrlen(enemies);i++){
		enemy *e=&enemies[i];
		e->update(e);
		if(e->add_shots!=NULL){
			e->add_shots(e);
		}
		if(e->add_effects!=NULL){
			e->add_effects(e);
		}

		if(e->type==ENEMY_SHOT){
			if(e->x<-20 || e->x>1300 || e->y<-20 || e->y>740){
				arrdelswap(enemies,i);
			}
		}
		
	}
}

void enemy_self_collision(){
	
	for(int i=0;i<arrlen(enemies);i++){
    	enemy *e=&enemies[i];
    	if(e->type!=ENEMY_SHOT && e->type!=ENEMY_SHURIKEN){
	    	for(int o=0;o<arrlen(enemies);o++){
	    		enemy *e2=&enemies[o];
	    		//make sure they aren't the same enemy
	    		if(e!=e2){
	    			if(e2->type!=ENEMY_SHOT && e2->type!=ENEMY_SHURIKEN){
	    				//if the collision circles overlap push the enemies away from each other.
	    				//this isn't a perfect solution, but it's good enough for this game.
	    				if(circles_intersect(e->x,e->y,e->colRad, e2->x,e2->y,e2->colRad)){	    					
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
		
		for(int o=0;o<arrlen(player_bullets);o++){
			player_bullet *b=&player_bullets[o];
			if(e->type==ENEMY_SHOT && p1.hasLasers==false){
				continue;
			}
			if(circles_intersect(e->x,e->y,e->colRad,b->x,b->y,b->colRad)){
				e->energy-=b->energy;
				if(p1.hasLasers==false){
					arrdelswap(player_bullets,o);
				}
			}
			
		}
		if(e->energy<=0){
			p1.score+=e->score*p1.multiplier;
			if(e->die!=NULL){
				e->die(e);
				audio_pan(&zap,e->x);
				play_audio(&zap,1.0,0);
			}
			arrdelswap(enemies,i);
			continue;
		}

	}
}

void enemy_draw(){
	for (int i=0;i<arrlen(enemies);i++){
		enemy *e=&enemies[i];
		if(e->type==ENEMY_SHOT){
			cg2d_set_layer(&c2d,effectsLayer);	
		}else{
			cg2d_set_layer(&c2d,spriteLayer);
		}
		cg2d_set_scale(&c2d,e->scale,e->scale);
		cg2d_set_rotation(&c2d,e->ang+e->angOffset);
		cg2d_set_colour(&c2d,255,255,255);
		cg2d_set_alpha(&c2d,1.0);
		cg2d_mid_handle_image(e->img);
		cg2d_draw_image(&c2d,e->img,e->x,e->y);
	}
}

//seek functions
int enemy_move_straight(enemy *e){
	e->timer1++;
	e->x2=e->x;
	e->y2=e->y;
	e->x+=e->xvel;
	e->y+=e->yvel;
	e->ang=cg2d_atan2(e->y2-e->y,e->x2-e->x);
	return 0;
}

int enemy_seek_player(enemy *e){
	e->timer1++;
	e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	e->x2=e->x;
	e->y2=e->y;
	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	return 0;
}

int enemy_seek_player_no_rotation(enemy *e){
	e->timer1++;
	e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	e->x2=e->x;
	e->y2=e->y;
	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	e->ang=0;
	return 0;
}

int enemy_seek_player_spinner(enemy *e){
	e->timer1++;
	e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	e->x2=e->x;
	e->y2=e->y;
	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	e->ang=e->timer1*e->spinSpeed;

	return 0;

}

int enemy_seek_player_wiggle(enemy *e){
	e->timer1++;
	e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	e->ang+=40*cg2d_sin(e->timer1*6);
	e->x2=e->x;
	e->y2=e->y;
	e->x=(e->xvel*cg2d_cos(e->ang))+e->x;
	e->y=(e->xvel*cg2d_sin(e->ang))+e->y;
	return 0;
}

int enemy_seek_player_pulse(enemy *e){
	e->timer1++;
	e->ang=cg2d_atan2(p1.y-e->y,p1.x-e->x);
	e->x2=e->x;
	e->y2=e->y;
	float speed=e->xvel+((e->xvel*0.5)*cg2d_sin(e->timer1*6));
	e->x=(speed*cg2d_cos(e->ang))+e->x;
	e->y=(speed*cg2d_sin(e->ang))+e->y;
	return 0;
}

int enemy_orbit_seek_player(enemy *e){
	e->timer1+=1;
	e->x2=e->x;
	e->y2=e->y;
	float speed=e->xvel+((e->xvel*0.5)*cg2d_sin(e->timer1*6));
	e->ang=cg2d_atan2(e->y-p1.y,e->x-p1.x)+130;
	e->x=speed*cg2d_cos(e->ang)+e->x;
	e->y=speed*cg2d_sin(e->ang)+e->y;
	return 0;	
}

int enemy_bounce(enemy *e){
	e->timer1+=1;
	e->x2=e->x;
	e->y2=e->y;
	e->x+=e->xvel;
	e->y+=e->yvel;
	if(e->x<20){
		e->xvel=-e->xvel;
		e->x=20;
	}
	if(e->x>1260){
		e->xvel=-e->xvel;
		e->x=1260;
	}
	if(e->y<20){
		e->yvel=-e->yvel;
		e->y=20;
	}
	if(e->y>700){
		e->yvel=-e->yvel;
		e->y=700;
	}
	e->ang+=6;
	return 0;
}

int enemy_seek_player_XY(enemy *e){

	e->xvel+=0.05;
	if(e->xvel>4)
		e->xvel=4;

	e->x2=e->x;
	e->y2=e->y;

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

//death functions
int enemy_explode(enemy *e){

	for(float i=0; i<360;i+=2){
			float rad=SDL_randf()*50;
			float ang=(float)i;
			float x=rad*cg2d_sin(ang)+e->x;
			float y=rad*cg2d_cos(ang)+e->y;
			float xvel=(x-(e->x))*0.1;
			float yvel=(y-(e->y))*0.1;
			particle_init(x,y,xvel,yvel,0,0,SDL_randf()*0.2,-0.001,1.0,SDL_rand(30)+30,blobImage,255,255,255);
			
			if((int)i%8==0){
				rad=10;
				x=rad*cg2d_sin(ang)+e->x;
				y=rad*cg2d_cos(ang)+e->y;
				xvel=(x-(e->x))*(SDL_randf()+1);
				yvel=(y-(e->y))*(SDL_randf()+1);
				particle_init(x,y,xvel,yvel,0,0,0.5,-0.01,1.0,60,blobImage,255,255,0);
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
				enemyBulletImage,enemy_move_straight,NULL,
				NULL,NULL,1.0,
				180,2.5,1,0,0,0,0);
	}

	return 0;
}

int enemy_add_shot_triple(enemy *e){

	if(e->timer1 % e->shotRate==0){
		float ang=e->ang-45;
		for(int i=0;i<3;i++){
			float xv=e->shotSpeed*cg2d_cos(ang);
			float yv=e->shotSpeed*cg2d_sin(ang);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					enemyBulletImage,enemy_move_straight,NULL,
					NULL,NULL,1.0,
					180,2.5,1,0,0,0,0);
			ang+=45;
		}
	}

	return 0;
}

int enemy_add_shot_ring(enemy *e){

	if(e->timer1 % e->shotRate==0){
		float xv=e->shotSpeed*cg2d_cos(e->ang);
		float yv=e->shotSpeed*cg2d_sin(e->ang);
		enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
				ringImage,enemy_move_straight,NULL,
				NULL,NULL,0.5,
				180,2.5,1,0,0,0,0);
	}

	return 0;
}

int enemy_add_shot_triple_ring(enemy *e){

	if(e->timer1 % e->shotRate==0){
		float ang=e->ang-45;
		for(int i=0;i<3;i++){
			float xv=e->shotSpeed*cg2d_cos(ang);
			float yv=e->shotSpeed*cg2d_sin(ang);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					ringImage,enemy_move_straight,NULL,
					NULL,NULL,0.5,
					180,2.5,1,0,0,0,0);
			ang+=45;
		}
	}

	return 0;
}

int enemy_add_shot_burst(enemy *e){

	if(e->timer1 > e->shotRate){
		if(e->timer1 %2==0){
			float xv=e->shotSpeed*cg2d_cos(e->ang+(SDL_randf()*2)-2);
			float yv=e->shotSpeed*cg2d_sin(e->ang+(SDL_randf()*2)-2);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					enemyBulletImage,enemy_move_straight,NULL,
					NULL,NULL,1.0,
					180,2.5,1,0,0,0,0);
		}
		if(e->timer1>e->shotRate+10){
			e->timer1=0;
		}
	}

	return 0;
}

int enemy_add_shot_burst_ring(enemy *e){

	if(e->timer1 > e->shotRate){
		if(e->timer1 %2==0){
			float xv=e->shotSpeed*cg2d_cos(e->ang+(SDL_randf()*2)-2);
			float yv=e->shotSpeed*cg2d_sin(e->ang+(SDL_randf()*2)-2);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					ringImage,enemy_move_straight,NULL,
					NULL,NULL,0.5,
					180,2.5,1,0,0,0,0);
		}
		if(e->timer1>e->shotRate+10){
			e->timer1=0;
		}
	}

	return 0;
}

int enemy_add_shot_burst_triple_ring(enemy *e){

	if(e->timer1 > e->shotRate){
		if(e->timer1 %2==0){
			float ang=e->ang-45;
			for(int i=0;i<3;i++){
				float xv=e->shotSpeed*cg2d_cos(ang);
				float yv=e->shotSpeed*cg2d_sin(ang);
				enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
						ringImage,enemy_move_straight,NULL,
						NULL,NULL,0.5,
						180,2.5,1,0,0,0,0);
				ang+=45;
			}
		}
		if(e->timer1>e->shotRate+10){
			e->timer1=0;
		}
	}

	return 0;
}

int enemy_add_shot_fourway(enemy *e){
	if(e->timer1 % e->shotRate==0){
		float ang=0;
		for(int i=0;i<4;i++){
			float xv=e->shotSpeed*cg2d_cos(ang);
			float yv=e->shotSpeed*cg2d_sin(ang);
			enemy_init(ENEMY_SHOT,e->x,e->y, xv,yv,0,
					rocketImage,enemy_move_straight,NULL,
					NULL,enemy_jet_effect,1.5,
					180,2.5,1,0,0,0,0);
			ang+=90;
		}
	}
	return 0;
}

int enemy_add_shot_fourway_spinner(enemy *e){
	if(e->timer1 % e->shotRate==0){
		float ang=e->ang;
		for(int i=0;i<4;i++){
			float x=(20*e->scale)*cg2d_cos(ang)+e->x;
			float y=(20*e->scale)*cg2d_sin(ang)+e->y;
			
			float xv=e->shotSpeed*cg2d_cos(ang);
			float yv=e->shotSpeed*cg2d_sin(ang);
			enemy_init(ENEMY_SHOT,x,y, xv,yv,0,
					enemyBulletImage,enemy_move_straight,NULL,
					NULL,NULL,1.0,
					180,2.5,1,0,0,0,0);
			ang+=90;
		}
	}
	return 0;
}

//effects functions
int enemy_jet_effect(enemy *e){

	if(e->timer1 % 2 ==0){
		float xv=(e->x2-e->x)*2;
		float yv=(e->y2-e->y)*2;
		particle_init(e->x,e->y,xv,yv,0,0,0.25,-0.01,1.0,60,blobImage,255,150,0);
	}
	return 0;
}

int enemy_corners_effect(enemy *e){

	//if(e->timer1% 2==0){

		float ang=e->ang+45;
		for(int i=0;i<4;i++){
			ang+=i*90;
			float x=(13*e->scale)*cg2d_cos(ang)+e->x;
			float y=(13*e->scale)*cg2d_sin(ang)+e->y;
			particle_init(x,y,0,0,0,0,0.1,-0.005,1.0,60,blobImage,255,255,255);
		}

	//}
	return 0;
}