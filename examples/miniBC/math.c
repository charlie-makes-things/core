
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