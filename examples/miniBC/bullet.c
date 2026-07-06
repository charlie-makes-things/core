void bullets_clear(){
	// for(int i=0;i<arrlen(player_bullets);i++){
	// 	arrdelswap(player_bullets,i);
	// }
	arrfree(player_bullets);
	player_bullets=NULL;
}

void bullet_update(){
	
	for(int i=0;i<arrlen(player_bullets);i++){
		player_bullet *b=&player_bullets[i];
		b->x+=b->xvel;
		b->y+=b->yvel;

		if(b->x<-0){
			float rx,ry;
			reflect(b->xvel,b->yvel,1.0,0.0,&rx,&ry);
			
			for(int o=0; o<10;o++){
				rx+=(SDL_randf());
				ry+=(SDL_randf()-0.5)*10;
				particle_init(10,b->y,rx*0.5,ry*0.5,0,0,0,0,0.2,-0.01,1.0,30,redImage,255,255,255,0);
			}
			arrdelswap(player_bullets,i);
			continue;
		}
		if(b->x>1280){
			float rx,ry;
			reflect(b->xvel,b->yvel,-1.0,0.0,&rx,&ry);
			for(int o=0; o<10;o++){
			rx+=(SDL_randf());
			ry+=(SDL_randf()-0.5)*10;
				particle_init(1270,b->y,rx*0.5,ry*0.5,0,0,0,0,0.2,-0.01,1.0,30,redImage,255,255,255,0);
			}
			arrdelswap(player_bullets,i);
			continue;
		}

		if(b->y<-0){
			float rx,ry;
			reflect(b->xvel,b->yvel,0.0,1.0,&rx,&ry);
			for(int o=0; o<10;o++){
			rx+=(SDL_randf()-0.5)*10;
			ry+=(SDL_randf());
				particle_init(b->x,10,rx*0.5,ry*0.5,0,0,0,0,0.2,-0.01,1.0,30,redImage,255,255,255,0);
			}
			arrdelswap(player_bullets,i);
			continue;
		}
		if(b->y>720){
			float rx,ry;
			reflect(b->xvel,b->yvel,0.0,-1.0,&rx,&ry);
			for(int o=0; o<10;o++){
			rx+=(SDL_randf()-0.5)*10;
			ry+=(SDL_randf());
				particle_init(b->x,710,rx*0.5,ry*0.5,0,0,0,0,0.2,-0.01,1.0,30,redImage,255,255,255,0);
			}
			arrdelswap(player_bullets,i);
			continue;
		}
		
	}
}

void bullet_draw(){
	for(int i=0;i<arrlen(player_bullets);i++){
		player_bullet *b=&player_bullets[i];
		cg2d_set_layer(&c2d,effectsLayer);
		cg2d_set_scale(&c2d,1.0*3.5,1.0*1.5);
	    cg2d_set_rotation(&c2d,b->ang+90);
	    cg2d_set_colour(&c2d,b->r,b->g,b->b);
	    cg2d_set_image_handle(bulletImage,0,0.5);
	    cg2d_draw_image(&c2d,bulletImage,b->x,b->y);

	    if(d_showCollisionCircles==true){
	    	cg2d_set_colour(&c2d,255,255,255);
	    	cg2d_set_alpha(&c2d,1.0);
	    	cg2d_set_scale(&c2d,1.0,1.0);
	    	cg2d_set_rotation(&c2d,0);
	    	cg2d_set_layer(&c2d,spriteLayer);
	    	cg2d_draw_oval(&c2d,b->x,b->y,b->colRad,b->colRad,false);
	    }

	}
}