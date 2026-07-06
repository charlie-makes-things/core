
void cue_sound(cg_audio_track *track,float vol,SDL_PropertiesID props,int delay,float pan){
    cued_audio c;
    c.track=track;
    c.vol=vol;
    c.props=props;
    c.delay=delay;
    c.pan=pan;
    arrpush(cued_SFX,c);
}

void update_cued_audio(){
    for(int i=0;i<arrlen(cued_SFX);i++){
        cued_audio *c=(cued_audio*)&cued_SFX[i];
        c->delay--;
        if(c->delay<=0){
            audio_pan(c->track,c->pan);
            play_audio(c->track,c->vol,c->props);
            arrdelswap(cued_SFX,i);
        }
    }
}

int SDLCALL high_score_sort(const void *a, const void *b)
{
    const high_score_entry *A = (const high_score_entry *)a;
    const high_score_entry *B = (const high_score_entry *)b;

    if (A->score < B->score) {
        return 1;
    } else if (B->score < A->score) {
        return -1;
    } else {
        return 0;
    }
}

void update_blobs(){
	for(Uint32 i=0;i<BLOB_COUNT;i++){
        blob *b=&blobs[i];
        
        b->deg+=b->spd*0.5;
        if(b->deg>360){
            b->deg-=360;
        }
        float ang=b->deg;

        b->rad=i*(cg2d_sin(ang));
        b->x=b->rad*(cg2d_cos( (i%2==0)? i:-i))+640;
        b->y=b->rad*(cg2d_sin(( i%2==0)? -i:i))+360;        
        b->alpha=(0.9*cg2d_sin((float)i))+0.5;
        
    }
}

void render_blobs(int ticks){

    cg2d_set_layer(&c2d, effectsLayer);
	for(Uint32 i=0;i<BLOB_COUNT;i++){
        blob *b=&blobs[i];       

        cg2d_set_scale(&c2d,b->sx*0.2,b->sy*0.2);
        cg2d_set_alpha(&c2d,b->alpha);
        if(i % 2 ==0){
            cg2d_set_scale(&c2d,b->sx*0.2,b->sy*0.2);
            cg2d_set_colour(&c2d,b->r,50+(cg2d_sin(ticks+i))+100,b->b);
            cg2d_draw_image(&c2d,redImage,b->x,b->y);
        }else{
            cg2d_set_scale(&c2d,b->sx*0.1,b->sy*0.1);
            cg2d_set_colour(&c2d,b->r,SDL_rand(150),0);
            cg2d_draw_image(&c2d,redImage,b->x,b->y);

        }
    }
}

void update_logo(){
	logoPosX+=(logoTargetX-logoPosX)*0.1;
    logoPosY+=(logoTargetY-logoPosY)*0.1;
    logoScale+=(logoTargetScale-logoScale)*0.1;
}

void render_logo(int ticks){

	double now = ((double)SDL_GetTicks()) / 4000.0;  /* convert from milliseconds to seconds. */
    float red = (float) (0.5 + 0.5 * SDL_sin(now));
    float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    

	cg2d_set_layer(&c2d,spriteLayer);
	    	    
    cg2d_mid_handle_image(logoImage);	    
    for(float i=10;i>=0;i--){
	    if(i==0){
	    	cg2d_set_alpha(&c2d,1.0);
    		cg2d_set_colour(&c2d,255,255,255);
    		cg2d_set_rotation(&c2d,0);
    		cg2d_set_layer(&c2d,spriteLayer);
	    }else{
	    	cg2d_set_alpha(&c2d,0.25-i*0.025);
	    	cg2d_set_colour(&c2d,red*255,green*255,blue*255);
	    	cg2d_set_rotation(&c2d,(i*2)*cg2d_sin((float)ticks*0.4));
	    	cg2d_set_layer(&c2d,effectsLayer);
	    }
                                                            //1-(i*(1/20))
	     float scl=(logoScale - (i*(logoScale/20)));
        float rad=logoScale/5;
        cg2d_set_scale(&c2d,rad*cg2d_sin((float)ticks*0.1)+scl,rad*cg2d_cos((float)ticks*0.1)+scl);
        cg2d_draw_image(&c2d,logoImage,logoPosX,logoPosY);
	}
}

void render_textures(int ticks){

	double now = ((double)SDL_GetTicks()) / 4000.0;  /* convert from milliseconds to seconds. */
    float red = (float) (0.5 + 0.5 * SDL_sin(now));
    float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));

	//draw the copy of the last frame scaled, recoloured and rotated etc. 
        cg2d_mid_handle_image(renderTexImage2);
        cg2d_set_layer(&c2d,renderTargetImageLayer2);                     
       cg2d_set_colour(&c2d,red*255,green*255,blue*255);
        cg2d_set_scale(&c2d,0.001*cg2d_sin((float)ticks*0.1)+1.01,0.0025*cg2d_sin(-(float)ticks*0.07)+1.01);
        cg2d_set_rotation(&c2d,cg2d_sin((float)ticks)*0.1);
        cg2d_set_alpha(&c2d,0.99801);
        cg2d_draw_image(&c2d,renderTexImage, cg2d_get_virtual_width(&c2d)/2,cg2d_get_virtual_height(&c2d)/2);
                
    //draw a screen size image without transformation. this will show the data for this frame.
        cg2d_mid_handle_image(renderTexImage);
        cg2d_set_layer(&c2d,renderTargetImageLayer);  
        cg2d_set_colour(&c2d,255,255,255);
        cg2d_set_scale(&c2d,1.0f,1.0f);
        cg2d_set_rotation(&c2d,0);
        cg2d_set_alpha(&c2d,1.0);
        cg2d_draw_image(&c2d,renderTexImage, cg2d_get_virtual_width(&c2d)/2,cg2d_get_virtual_height(&c2d)/2);
     
}

