
void bonus_kill(bonus *b);

void bonus_init(bonus_type type,float x, float y){
	//pick a point off screen
	bonus b;
	b.ang=SDL_randf()*360;
	b.x=x;
	b.y=y;
	b.type=type;
	arrput(bonuses,b);
}

void bonuses_clear(){
	// for(int i=0;i<arrlen(bonuses);i++){
	// 	arrdelswap(bonuses,i);
	// }
	arrfree(bonuses);
	bonuses=NULL;
}

void bonus_init_offscreen(bonus_type type){
	//pick a point off screen
	bonus b;
	b.ang=SDL_randf()*360;
	b.x=660*cg2d_cos(b.ang)+640;
	b.y=370*cg2d_sin(b.ang)+360;
	b.type=type;
	arrput(bonuses,b);
}



void bonus_update(){

	for(int i=0;i<arrlen(bonuses);i++){
		bonus *b=(bonus*)&bonuses[i];

		b->x+=(p1.x-b->x)*0.05;
		b->y+=(p1.y-b->y)*0.05;

		b->ang+=10;

		if(circles_intersect(b->x,b->y,30,p1.x,p1.y,p1.colRad)==true){

			if(b->type==BONUS_SCORE){
				p1.score+=10000*p1.multiplier;
				audio_pan(&scoreUpSFX,b->x);
				play_audio(&scoreUpSFX,1.0,0);
			}else if(b->type==BONUS_TRIPLE){
				p1.threewayTimer+=300;
				audio_pan(&threewaySFX,b->x);
				play_audio(&threewaySFX,1.0,0);
			}else if(b->type==BONUS_LIFE){
				p1.lives++;
				audio_pan(&oneUpSFX,b->x);
				play_audio(&oneUpSFX,1.0,0);
			}else{
				p1.score+=10000*p1.multiplier;
			}
			bonus_kill(b);
			arrdelswap(bonuses,i);


		}
	}

}

void bonus_draw(){
	for(int i=0;i<arrlen(bonuses);i++){
		bonus *b=(bonus*)&bonuses[i];
		cg2d_set_layer(&c2d,spriteLayer);

		cg2d_set_scale(&c2d,1.0,1.0);
		cg2d_set_rotation(&c2d,b->ang+270);
		cg2d_set_colour(&c2d,255,255,255);
		cg2d_set_alpha(&c2d,1.0);
		
		cg2d_image *img=NULL;

		if(b->type==BONUS_SCORE){
			img=rescuableImage;
		}else if(b->type==BONUS_TRIPLE){
			img=rescuableBlueImage;
		}else if(b->type==BONUS_LIFE){
			img=rescuableRedImage;
		}else{
			img=rescuableImage;
			cg2d_set_colour(&c2d,0,255,255);
		}

		cg2d_set_image_handle(img,0.5,0.5);
		cg2d_draw_image(&c2d,img,b->x,b->y);

		if(d_showCollisionCircles==true){
	    	cg2d_set_colour(&c2d,255,255,255);
	    	cg2d_set_alpha(&c2d,1.0);
	    	cg2d_set_scale(&c2d,1.0,1.0);
	    	cg2d_set_rotation(&c2d,0);
	    	cg2d_set_layer(&c2d,spriteLayer);
	    	cg2d_draw_oval(&c2d,b->x,b->y,30,30,false);
	    }
				
	}
}



void bonus_kill(bonus *b){
	
	for(int i=0;i<20;i++){
		float xv=(SDL_randf()*20)-10;
		float yv=(SDL_randf()*20)-10;

		particle_init(b->x,b->y,xv,yv,0,0,0,3,2.0,-0.01,1.0,90,starImage,255,SDL_rand(255),0,0);
			

	}


}



void bonus_kill_all(){
	for(int i=0;i<arrlen(bonuses);i++){
		bonus *b=(bonus*)&bonuses[i];
		bonus_kill(b);
		arrdelswap(bonuses,i);
	}
}