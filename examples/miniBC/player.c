
void player_reset_game(){
	p1.x=640;
    p1.y=360;
    p1.xvel=p1.yvel=0;
    p1.ang=0;
    p1.active=false;
    p1.score=0;
    p1.multiplier=1;
    p1.streak=0;
    p1.lives=3;
    p1.colRad=5;
    p1.threewayTimer=0;
    p1.horse=false;
   
}

void player_reset_level(){
	p1.x=640;
    p1.y=360;
    p1.xvel=p1.yvel=0;
    p1.ang=0;
    p1.active=false;    
    p1.colRad=5;
}

void player_update(cg_controller *c,int ticks){
	
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_NORTH)==true){
		p1.active=false;
	}
	if(cg_controller_get_button_released(c,SDL_GAMEPAD_BUTTON_NORTH)==true){
		p1.active=true;
	
	}
	if(cg_controller_get_button_released(c,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)==true){
		p1.horse=!p1.horse;
	}

	if(p1.active==true){
	float xAxis=cg_controller_get_axis_normalized(c,SDL_GAMEPAD_AXIS_LEFTX);
	float yAxis=cg_controller_get_axis_normalized(c,SDL_GAMEPAD_AXIS_LEFTY);

	float x2Axis=cg_controller_get_axis_normalized(c,SDL_GAMEPAD_AXIS_RIGHTX);
	float y2Axis=cg_controller_get_axis_normalized(c,SDL_GAMEPAD_AXIS_RIGHTY);

	p1.xvel+=xAxis;
	p1.yvel+=yAxis;

	p1.xvel+=(0.0-p1.xvel)*0.25;
	p1.yvel+=(0.0-p1.yvel)*0.25;
	
	p1.x+=p1.xvel;
	p1.y+=p1.yvel;

	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_SOUTH)==false){
		float targetAng=cg2d_atan2(0.0-p1.yvel,0.0-p1.xvel)+90;
		p1.ang=targetAng;
	}

	if(SDL_fabs(x2Axis)>0 || SDL_fabs(y2Axis)>0){
		p1.ang=cg2d_atan2(0.0-y2Axis,0.0-x2Axis)+90;
	}

	if(p1.x<30){
		p1.x=30;
		p1.xvel=0;
	}else if(p1.x>1250){
		p1.x=1250;
		p1.xvel=0;
	}

	if(p1.y<30){
		p1.y=30;
		p1.yvel=0;
	}else if(p1.y>690){
		p1.y=690;
		p1.yvel=0;
	}

	if(p1.threewayTimer>0)
		p1.threewayTimer--;


	if(ticks % 5==0){
		float nx,ny;
		//normalize(p1.xvel,p1.yvel,&nx,&ny);
		nx=16*cg2d_cos(p1.ang+90);
		ny=16*cg2d_sin(p1.ang+90);
		player_bullet b;
		b.x=p1.x;
		b.y=p1.y;
		b.ang=p1.ang;
		b.xvel=nx;
		b.yvel=ny;
		b.colRad=15;
		b.r=b.g=b.b=255;
		b.energy=1;


		arrpush(player_bullets,b);
		audio_pan(&playerShotSFX,b.x);
		play_audio(&playerShotSFX,0.5,0);

		

		if(p1.threewayTimer>0){
			
			b.ang-=15;
			nx=16*cg2d_cos(p1.ang+75);
			ny=16*cg2d_sin(p1.ang+75);
			b.xvel=nx;
			b.yvel=ny;
			arrpush(player_bullets,b);
			
			b.ang+=30;
			nx=16*cg2d_cos(p1.ang+105);
			ny=16*cg2d_sin(p1.ang+105);
			b.xvel=nx;
			b.yvel=ny;
			arrpush(player_bullets,b);
		}

	}

	}

}


void player_draw(){
	 if(p1.active==true){
    	//draw player here
    	cg2d_set_layer(&c2d,spriteLayer);
    	cg2d_set_scale(&c2d,1.0*1.5,1.0*1.5);
    	cg2d_set_rotation(&c2d,p1.ang);
    	cg2d_set_colour(&c2d,255,255,255);
    	cg2d_set_alpha(&c2d,1.0);
    	if(p1.horse==false){
	    	cg2d_set_image_handle(blueflatImage,0.5,0.25);
	    	cg2d_draw_image(&c2d,blueflatImage,p1.x,p1.y);
	    }else{
	    	cg2d_set_image_handle(horseImage,0.5,0.25);
	    	cg2d_draw_image(&c2d,horseImage,p1.x,p1.y);
	    }
    	if(d_showCollisionCircles==true){
	    	cg2d_set_colour(&c2d,255,255,255);
	    	cg2d_set_alpha(&c2d,1.0);
	    	cg2d_set_scale(&c2d,1.0,1.0);
	    	cg2d_set_rotation(&c2d,0);
	    	cg2d_set_layer(&c2d,spriteLayer);
	    	cg2d_draw_oval(&c2d,p1.x,p1.y,p1.colRad,p1.colRad,false);
	    }

    }
}

void player_die(){

	float rad=10;
	float x,y,z,zx,zy;;
	float slo,sla,clo,cla;
	float offset=SDL_randf()*90;
	for(float i=0;i<180;i++){
		rad=150+i*6;

		for(float longitude=offset+i;longitude<offset+i+180;longitude+=12){
			for(float latitude=offset+i;latitude+i<offset+360;latitude+=18){
				slo=cg2d_sin(longitude );
				sla=cg2d_sin(latitude);
				clo=cg2d_cos(longitude);
				cla=cg2d_cos(latitude);
				
				x=slo * cla * rad;
				y=sla * rad;
				z=clo *cla * rad;
				z+=500;
				
				 zx=rad/z;
				 zy=rad/z;

				x=x*zx;
				y=y*zy;

				
				
				particle_init(x+p1.x,y+p1.y,0.0,0.0,0,0,0,0,0.05,-0.001,1.0,20,blobImage,255,255,255,i);
				particle_init(x+p1.x,y+p1.y,0.0,0.0,0,0,0,0,0.4,-0.002,0.25,20,blobImage,255,0,0,i);
				
				
			}
		}
	}

}

void player_spawn_particles(){

	particle_init(p1.x,p1.y,0,0,0,0,0,0,5.0,-0.01,1.0,180,blobImage,0,0,255,0);
	particle_init(p1.x,p1.y,0,0,0,0,0,0,2.0,-0.001,1.0,180,blobImage,255,255,255,0);

	for(int i=0;i<180;i++){
		float rad=SDL_randf()*100;
		float ang=SDL_randf()*360;
		float x=p1.x+(rad*cg2d_cos(ang));
		float y=p1.y+(rad*cg2d_sin(ang));
		float xv=(SDL_randf()*6)-3;
		float yv=(SDL_randf()*6)-3;

		particle_init(x,y,xv,yv,0,0.01,0,0,0.5,-0.005,0.25,120,blobImage,0,0,255,i);
		particle_init(x,y,xv,yv,0,0.01,0,0,0.1,-0.001,1.0,120,blobImage,255,255,255,i);
		
		
	}
	int delay=0;
	for(int i=0;i<360;i++){
		float rad=SDL_randf()*6;
		float ang=SDL_randf()*360;
		float x=p1.x+(rad*cg2d_cos(ang));
		float y=p1.y+(rad*cg2d_sin(ang));
		float va=cg2d_atan2(y-p1.y,x-p1.x);
		float xv=((SDL_randf()*12)-6)*cg2d_cos(va);
		float yv=((SDL_randf()*12)-6)*cg2d_sin(va);
		if(i%2==0){
			delay++;
		}

		particle_init(x,y,xv,yv,0,0.01,0,0,0.5,-0.005,0.25,120,blobImage,0,255,255,delay);
		particle_init(x,y,xv,yv,0,0.01,0,0,0.1,-0.001,1.0,120,blobImage,255,255,255,delay);
		
		ang=(float)i;
		if(i%2==0){
			rad=10;
			x=rad*cg2d_sin(ang)+p1.x;
			y=rad*cg2d_cos(ang)+p1.y;
			xv=(x-(p1.x))*2;
			yv=(y-(p1.y))*2;
			particle_init(x,y,xv,yv,0,0,0,0,1.5,-0.01,1.0,60,blobImage,255,255,255,120);
		}

		rad=SDL_randf()*50;
		ang=(float)i;
		x=rad*cg2d_sin(ang)+p1.x;
		y=rad*cg2d_cos(ang)+p1.y;
		xv=(x-(p1.x))*0.1;
		yv=(y-(p1.y))*0.1;
			

		particle_init(x,y,xv,yv,0,0,0,0,SDL_randf()*0.8,-0.008,1.0,SDL_rand(30)+60,blobImage,255,255,255,120);
			
	}



}
