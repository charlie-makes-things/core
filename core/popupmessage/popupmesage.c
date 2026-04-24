//uses cg2d to draw message boxes in a corner of the screen. useful
//for device connect/disconnect, saving, achievements etc.

//////////////////////////////////////////////////////////////////
//																//			
//	-------------     POP - UP - MESSAGE - TITLE				//
//	|			|												//
//	|			|												//
//	|	image   |		POP - UP  - MESSAGE - DATA				//
//	|			|												//
//	-------------												//
//																//
//////////////////////////////////////////////////////////////////


typedef struct cg_popup_message{
	float x,y;
	float width;
	float life;
	char *message;
	char *data;
	cg2d_image *image;
	float fontScale;
	
}cg_popup_message;

typedef struct cg_popup_message_handler{
	cg_popup_message *messages; //stretchy buffer
	
}cg_popup_message_handler;

static cg_popup_message_handler _popup_messages;
static float _popup_font_scale;

void cg_popup_message_init(float fontScale){
	_popup_font_scale=fontScale;
}

void popup_messags_add_message(char *message,char *data,cg2d_t *c2d, cg2d_image *image,float fontScale){

	cg_popup_message p;
	p.message=message;
	p.data=data;
	p.life=180;
	p.image=image;
	p.fontScale=fontScale*_popup_font_scale;

	float mw=cg2d_text_width(c2d,message)*p.fontScale;
	float dw=cg2d_text_width(c2d,data)*p.fontScale;

	p.width=(SDL_max(mw,dw)*1.2)+100;

	p.x=0;
	p.y=WINDOW_HEIGHT;
	SDL_Log("popup window height is %f\n",p.y);			

	arrpush(_popup_messages.messages,p);

}



void popup_messages_update(){
	for(int i=0;i<arrlen(_popup_messages.messages);i++){
		
		float ty=WINDOW_HEIGHT-100-((arrlen(_popup_messages.messages)-1-i)*100);
		float tx=0.0;
		if(_popup_messages.messages[i].life<60){
			tx=-1280;
		}
		_popup_messages.messages[i].x+=(tx-_popup_messages.messages[i].x)*0.1;
		_popup_messages.messages[i].y+=(ty-_popup_messages.messages[i].y)*0.1;
		_popup_messages.messages[i].life--;
		if(_popup_messages.messages[i].life<=0){
			arrdel(_popup_messages.messages,i);
			continue;
		}
	}
}

void popup_messages_draw( cg2d_t *c2d, int spriteLayer,int fontLayer ){
	cg2d_set_handle(c2d,0,0);
	cg2d_set_scale(c2d,1,1);
	cg2d_set_rotation(c2d,0);
	cg2d_set_alpha(c2d,1.0);
	cg2d_set_skew(c2d,0.0,0.0);
	cg2d_set_layer(c2d,spriteLayer);
	for(int i=arrlen(_popup_messages.messages)-1;i>=0;i--){
		cg_popup_message *p=&_popup_messages.messages[i];
		cg2d_set_scale(c2d,1,1);
		cg2d_set_layer(c2d,spriteLayer);
		cg2d_set_outline_width(c2d,0.01);
		cg2d_set_colour(c2d,50,50,50);
		cg2d_draw_rect(c2d,p->x,p->y,p->width,90,true);
		cg2d_set_colour(c2d,75,75,75);
		cg2d_draw_rect(c2d,p->x+2,p->y+2,p->width-4,90-4,true);
		cg2d_set_colour(c2d,255,255,255);
		
		if(p->image!=NULL){
			cg2d_draw_image_rect(c2d,p->image,p->x+20,p->y+20,50,50);
		}else{
			SDL_Log("image null");
		}

		cg2d_set_layer(c2d,fontLayer);
		cg2d_set_scale(c2d,p->fontScale,p->fontScale);
		cg2d_set_colour(c2d,200,200,200);
		cg2d_draw_text(c2d,p->message,p->x+80,p->y+10);
		
		cg2d_draw_text(c2d,p->data,p->x+80,p->y+50);
		
	}
	cg2d_set_scale(c2d,1,1);
}

void popup_messages_free(){
	if(arrlen(_popup_messages.messages)>0){
		arrfree(_popup_messages.messages);
	}
	
}