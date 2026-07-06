//enemyFunc.c

void spawn_grunt1(float x, float y, float angle,float spd,cg2d_image *img,float angleOffset){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&img,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player,//ai
				NULL,//shot pattern
				enemy_explode_medium,//explode effect
				enemy_jet_effect,//particles
				2.0,//scale
				angleOffset,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_grunt2(float x, float y, float angle,float spd){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&grunt2Image,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player_pulse,//ai
				NULL,//shot pattern
				enemy_explode,//explode effect
				enemy_glow_effect_green,//particles
				2.0,//scale
				270,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_grunt3(float x, float y, float angle,float spd){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&grunt3Image,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_orbit_seek_player,//ai
				NULL,//shot pattern
				enemy_explode,//explode effect
				enemy_jet_effect,//particles
				2.0,//scale
				270,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_grunt4(float x, float y, float angle,float spd){

	int r=SDL_rand(2);
	float vy=-1;
	if(r>0){
		vy=1;
	}

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				vy,//yvel
				angle,//angle
				&spikerImage,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player_XY,//ai
				NULL,//shot pattern
				enemy_explode,//explode effect
				enemy_jet_effect,//particles
				2.0,//scale
				0,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_splitter(float x, float y, float angle,float spd){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&harvesterImage,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player_pulse,//ai
				NULL,//shot pattern
				enemy_explode_add_shot_splitter,//explode effect
				enemy_glow_effect_red_pulse,//particles
				1.0,//scale
				270,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_grunt6(float x, float y, float angle,float spd){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&grunt6Image,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player_wiggle,//ai
				NULL,//shot pattern
				enemy_explode,//explode effect
				enemy_glow_effect_red_pulse,//particles
				0.5,//scale
				270,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_grunt7(float x, float y, float angle,float spd){

	
	enemy_init(ENEMY_GRUNT,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&grunt7Image,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player_wiggle,//ai
				NULL,//shot pattern
				enemy_explode,//explode effect
				enemy_glow_effect_red_pulse,//particles
				1.0,//scale
				270,//angleoffset
				10,//collision radius
				1,//energy
				10,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_tank(float x, float y, float angle,float spd,int shotRate){

	
	enemy_init(ENEMY_TANK,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				&tankImage,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player,//ai
				enemy_add_shot_burst,//shot pattern
				enemy_explode_large,//explode effect
				enemy_jet_effect,//particles
				2.0,//scale
				270,//angleoffset
				30,//collision radius
				1,//energy
				250,//score
				shotRate,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);

	
}

void spawn_electrode(float x,float y, float angle, float spd){

	cg2d_image *electrodeImages[2]={electrodeImage1,electrodeImage2};

	enemy_init(ENEMY_ELECTRODE,
				x,
				y,
				spd,//xvel
				0,//yvel
				angle,//angle
				electrodeImages,//image
				2,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_seek_player,//ai
				NULL,//shot pattern
				enemy_explode_electrode,//explode effect
				enemy_spark_effect,//particles
				1.5,//scale
				270,//angleoffset
				90,//collision radius
				1,//energy
				250,//score
				1,//shotrate
				4,//shotspeed
				40,//spinspeed
				4.0//knockback scalar
				);


}



void spawn_orbiter( float speed, int shotRate,float angle){

	float orbiterangle=angle;
	float orbiterx=800*cg2d_sin(orbiterangle)+640;
	float orbitery=800*cg2d_cos(orbiterangle)+360;

	enemy_init(ENEMY_ORBITER,
				orbiterx,
				orbitery,
				speed,//xvel
				SDL_randf()*5,//yvel
				orbiterangle,//angle
				&orbiterImage,//image
				1,//image count
				0.5,//image x handle
				0.5,//image yhandle
				enemy_orbit_point,//ai
				enemy_add_shot_orbiter,//shot pattern
				enemy_explode_medium,//explode effect
				enemy_glow_effect_green,//particles
				3.0,//scale
				270,//angleoffset
				20,//collision radius
				1,//energy
				200,//score
				shotRate,//shotrate
				1,//shotspeed
				40,//spinspeed
				0//knockback scalar
				);
		

}

