
void normalize(float x,float y, float *outx,float *outy){
	float mag = x * x + y * y;
	*outx=x;
	*outy=y;
	if( mag > 0){
		mag = 1 / SDL_sqrtf(mag);
		*outx*=mag;
		*outy*=mag;
	}
}

void reflect(float x, float y, float nx, float ny, float *outx,float *outy){
	float d = x * nx + y * ny;//vecDot(a, norm)
	*outx = x - 2 * nx * d;
	*outy = y - 2 * ny * d;
}

bool circles_intersect(float x1,float y1,float r1, float x2, float y2, float r2){
	float distance = SDL_sqrt(SDL_pow(x2 - x1, 2) + SDL_pow(y2 - y1, 2));
	if(distance < r1 + r2){
		return true;
	}

	return false;
}

void rotate(float x,float y,float theta, float *outx,float *outy){
	float c = cg2d_cos(theta);
	float s = cg2d_sin(theta);
	float xx = c * x - s * y;
	*outx=xx; 
	*outy=	s * x + c * y;
}

float heading(float x,float y){
	return cg2d_atan2(y,x);
}

float dot(float x0,float y0,float x1,float y1){
	return x0 * x1 + y0 * y1;
}

struct _compass_dir{
	float x,y;
};
//this returns 3 for north, 2 for east, 1 for south and 0 for west.
int compass_direction(float x, float y){

	struct _compass_dir dir[4]={{0,1},{1,0},{0,-1},{-1,0}};

	float best=0;
	int ret=-1;
	for(int i=0;i<4;i++){
		float res=dot(dir[i].x,dir[i].y,x,y);
		if(res>best){
			best=res;
			ret=i;
		}
	}

	return ret;


	// for each (float2 dir in candidates)
	// {
	//     float goodness = dot(dir, v);
	//     if (goodness > bestResult)
	//     {
	//         bestResult = goodness;
	//         bestDir = candidates[dir];
	//     }    
	// }
}


