
void message_init(char *msg,size_t len,float x, float y, int life,float size){

	message m;
	
	SDL_strlcpy(m.str, (const char *)msg, len+1);
	
	m.x=x;
	m.y=y;
	m.scl=0.1;
	m.sclMod=size;
	m.life=life;
	m.alphaMod=1.0/(float)20;
	m.alpha=1.0;
	arrput(messages,m);

}

void message_update(){

	for(int i=0;i<arrlen(messages);i++){
		message *m=(message*)&messages[i];
		m->scl+=(m->sclMod-m->scl)*0.05;
		if(m->life<20){
			m->alpha-=m->alphaMod;
		}
		m->y-=0.1;
		m->life--;
		if(m->life<=0){
			arrdelswap(messages,i);
		}
	}

}

void messages_clear(){
	// for(int i=0;i<arrlen(messages);i++){		
	// 	arrdelswap(messages,i);
	// }
	arrfree(messages);
	messages=NULL;
}

void message_draw(){

	for(int i=0;i<arrlen(messages);i++){
		message *m=(message*)&messages[i];
		cg2d_set_alpha(&c2d,m->alpha);
		cg2d_set_scale(&c2d,m->scl,m->scl);
		cg2d_set_colour(&c2d,255,255,255);
		cg2d_set_rotation(&c2d,0);
		cg2d_set_handle(&c2d,0.5,0.0);

		cg2d_set_layer(&c2d,fontLayer);
		cg2d_draw_text(&c2d,m->str,m->x,m->y);
	}

}