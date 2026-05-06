//struct for a star field point
typedef struct starfield_point{
    float x,y,z;
    float dispX,dispY;
    float speed;
}starfield_point;

#define MAX_STARS 250
starfield_point *starfield=NULL;//stretchy buffer

void starfield_init(){
	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);
	for(int i=0;i<MAX_STARS;i++){
		starfield_point s;		
		//some points are initialised offscreen here. this is to make the
		//fake 3d perspective work as the scale is centered around 0,0
		s.x=(SDL_randf()*vWidth)-vWidth/2;
		s.y=(SDL_randf()*vHeight)-vHeight/2;
		s.z=SDL_randf();
		s.speed=SDL_randf()*0.002f;
		arrput(starfield,s);
	}
}

void starfield_update(){
	int vWidth=cg2d_get_virtual_width(&c2d);
	int vHeight=cg2d_get_virtual_height(&c2d);
	for(int i=0;i<MAX_STARS;i++){
		starfield_point *p=&starfield[i];
		p->z-=p->speed;
		// work out 2d position. adding the width and height translates the points
		//across into the display area while keeping the perspective correct.
		p->dispX=(p->x/p->z)+(vWidth/2);
		p->dispY=(p->y/p->z)+(vHeight/2);        
        //if the star cant be rendered, reset it
        if ((p->z < 0.0f )|| (p->dispX < 0) || (p->dispY < 0.0f) ||(p->dispX > vWidth) || (p->dispY> vHeight)) {
			p->x=(SDL_randf()*vWidth)-vWidth/2;
			p->y=(SDL_randf()*vHeight)-vHeight/2;
            p->z = 1.0f;
        }
	}
}


void starfield_draw(int layer){
	
	//draw the starfield points
	cg2d_set_alpha(&c2d,0.5);
	cg2d_set_scale(&c2d,1.0,1.0);
	cg2d_set_colour(&c2d,255,255,255);
	cg2d_set_rotation(&c2d,0);
	cg2d_set_layer(&c2d,layer);
	for(int i=0;i<MAX_STARS;i++){
		//scale the point size so they appear bigger the closer to the camera they get, otherwise
		//known as distance attenuation
		//this is a trick, and not true 3d projection, but it looks fine in this instance.
		cg2d_set_point_size(&c2d,1.0+(5.0-starfield[i].z*5));
		cg2d_plot(&c2d,starfield[i].dispX,starfield[i].dispY,true,false);
	}
}

void starfield_free(){
	if(starfield!=NULL){
		arrfree(starfield);
	}
}

