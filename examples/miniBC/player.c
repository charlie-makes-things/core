
void player_update(cg_controller *c,int ticks){

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

	if(ticks % 4==0){
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
			b.colRad=10;
			b.energy=(p1.hasLasers==true)?10:1;
			arrpush(player_bullets,b);
			audio_pan(&playerShot,b.x);
			play_audio(&playerShot,0.5,0);
	}
}



