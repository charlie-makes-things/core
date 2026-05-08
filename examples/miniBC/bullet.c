void bullet_update(){
	
	for(int i=0;i<arrlen(player_bullets);i++){
		player_bullet *b=&player_bullets[i];
		b->x+=b->xvel;
		b->y+=b->yvel;

		if(b->x<-0){
			float rx,ry;
			reflect(b->xvel,b->yvel,1.0,0.0,&rx,&ry);
			
			for(int i=0; i<10;i++){
				rx+=(SDL_randf());
				ry+=(SDL_randf()-0.5)*10;
				particle_init(10,b->y,rx*0.5,ry*0.5,0,0,0.2,-0.01,1.0,30,redImage,255,255,255);
			}
			arrdelswap(player_bullets,i);
		}
		if(b->x>1280){
			float rx,ry;
			reflect(b->xvel,b->yvel,-1.0,0.0,&rx,&ry);
			for(int i=0; i<10;i++){
			rx+=(SDL_randf());
			ry+=(SDL_randf()-0.5)*10;
				particle_init(1270,b->y,rx*0.5,ry*0.5,0,0,0.2,-0.01,1.0,30,redImage,255,255,255);
			}
			arrdelswap(player_bullets,i);
		}

		if(b->y<-0){
			float rx,ry;
			reflect(b->xvel,b->yvel,0.0,1.0,&rx,&ry);
			for(int i=0; i<10;i++){
			rx+=(SDL_randf()-0.5)*10;
			ry+=(SDL_randf());
				particle_init(b->x,10,rx*0.5,ry*0.5,0,0,0.2,-0.01,1.0,30,redImage,255,255,255);
			}
			arrdelswap(player_bullets,i);
		}
		if(b->y>720){
			float rx,ry;
			reflect(b->xvel,b->yvel,0.0,-1.0,&rx,&ry);
			for(int i=0; i<10;i++){
			rx+=(SDL_randf()-0.5)*10;
			ry+=(SDL_randf());
				particle_init(b->x,710,rx*0.5,ry*0.5,0,0,0.2,-0.01,1.0,30,redImage,255,255,255);
			}
			arrdelswap(player_bullets,i);
		}
		
	}
}

void bullet_draw(){
	for(int i=0;i<arrlen(player_bullets);i++){
		player_bullet *b=&player_bullets[i];
		cg2d_set_layer(&c2d,effectsLayer);
		cg2d_set_scale(&c2d,1.0*3.5,1.0*1.5);
	    cg2d_set_rotation(&c2d,b->ang+90);
	    cg2d_set_colour(&c2d,255,255,255);
	    cg2d_set_image_handle(bulletImage,0,0.5);
	    cg2d_draw_image(&c2d,bulletImage,b->x,b->y);
	}
}