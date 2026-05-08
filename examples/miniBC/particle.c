
void particle_init(float x, float y,float xvel, float yvel,float ang,float angVel,
				   float sz, float szvel,float alpha, int life,cg2d_image *img,
				   int r, int g, int b){

	particle p;
	p.x=x;
	p.y=y;
	p.yvel=yvel;
	p.xvel=xvel;
	p.ang=ang;
	p.angVel=angVel;
	p.sz=sz;
	p.szvel=szvel;
	p.alpha=alpha;
	p.life=life;
	p.img=img;
	p.r=r;
	p.g=g;
	p.b=b;
	p.alphaVel=p.alpha/p.life;
	p.delay=0;
	arrput(particles,p);
	return;
}

void particle_update(){

	for(int i=0;i<arrlen(particles);i++){
		particle *p=&particles[i];
		if(p->delay<=0){
			p->x+=p->xvel;
			p->y+=p->yvel;
			p->ang+=p->angVel;
			p->sz+=p->szvel;
			p->alpha+=(0.0-p->alpha)*p->alphaVel;
			p->life--;
			if(p->life<=0 || p->sz<0.0){
				arrdelswap(particles,i);
			}
		}else{
			p->delay--;
		}
	}

	return;
}

void particle_draw(){

	for(int i=0;i<arrlen(particles);i++){
		particle *p=&particles[i];
		cg2d_set_rotation(&c2d,p->ang);
		cg2d_set_scale(&c2d,p->sz,p->sz);
		cg2d_set_alpha(&c2d,p->alpha);
		cg2d_set_colour(&c2d,p->r,p->g,p->b);
		if(p->img!=NULL){
			cg2d_mid_handle_image(p->img);
			cg2d_draw_image(&c2d,p->img,p->x,p->y);
		}else{
			//no image so just plot
			cg2d_set_point_size(&c2d,1.0);
			cg2d_plot(&c2d,p->x,p->y,true,false);
		}
	}

	return;
}