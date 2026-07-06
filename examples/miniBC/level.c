int miniBC_get_level_data(){
	
	//see if the enemies have stopped
	if(waveTicker==1100){
		levelEndCondition=true;
		// wave ended
		char *str="Level Up!!!";
		message_init(str,SDL_strlen(str),640,600,120,1);
		//level up message
		play_audio(&perfectSFX,1.0,0);
	}
	
	//increase the wave number if the exit condition is met, 
	//or go to the game complete screen
	if(waveTicker>1200){
		if(levelEndCondition==true){
			if(waveNumber<11){
				waveNumber+=1;
				waveTicker=0;
				p1.streak++;
				long s=(10000*p1.streak)*p1.multiplier;
				p1.score+=s;
				char streak[128];				
				SDL_snprintf(streak,sizeof(char)*128,"Streak Bonus: %ld",s);
				message_init(streak,SDL_strlen(streak),640,600,120,1);
			}else{
				if(gameComplete==false){
					play_audio(&completeSFX,1.0,0);
					gameComplete=true;
					char *str="Game Complete!!!";
					message_init(str,SDL_strlen(str),640,600,120,1);
					waveResetTime=waveTicker;//use this as a time stamp after game complete.
				}
			}
		}
	}

	//work out which side the player is most facing so we dont spawn enemies
	//directly infornt of them.
	float fx=(1.0)*cg2d_cos(p1.ang);
	float fy=(1.0)*cg2d_sin(p1.ang);
	
	//this returns 3 for north, 2 for east, 1 for south and 0 for west.
	int cdir=compass_direction(fx,fy);

	//fill an array with the directions the player isn't facing
	int sides[3];
	if(cdir==3){
		sides[0]=0;
		sides[1]=1;
		sides[2]=2;
	}else if( cdir==0){
		sides[0]=1;
		sides[1]=2;
		sides[2]=3;
	}else if(cdir==1){
		sides[0]=0;
		sides[1]=2;
		sides[2]=3;
	}else if(cdir==2){
		sides[0]=0;
		sides[1]=1;
		sides[2]=3;
	}else{
		sides[0]=0;
		sides[1]=1;
		sides[2]=2;
	}

	int rn=SDL_rand(4);
	float thex=0;
	float they=0;	
	float theang=0.000000f;

	//pick a random side to spawn an enemy.
	if(sides[rn]==3){//north
		thex=SDL_randf()*1280;
		they=-10.000000f;
		theang=0.000000f;
	}else if(sides[rn]==0){//south
		thex=-10.0f;
		they=SDL_randf()*720;
		theang=90.0f;
	}else if (sides[rn]==1){//east
		thex=SDL_randf()*1280;
		they=730.0f;
		theang=180.0f;

	}else{//west
		thex=1290.0f;
		they=SDL_randf()*720;
		theang=270.0f;
	}	

	//check against the ticker to see if we should spawn an enemy.
	if(waveTicker<1100 && waveNumber<12){
		//levelEndCondition=true;
		
		if(waveNumber==1){
			if(waveTicker % 20==0){
				spawn_grunt1(thex,they,theang,1.5,grunt1Image,270);				
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==2){
			if(waveTicker % 20==0){
				spawn_grunt2(thex,they,theang,2.0);				
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==3){
			if(waveTicker % 20==0){
				spawn_grunt3(thex,they,theang,4);				
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==4){
			if(waveTicker % 20==0){
				spawn_grunt4(thex,they,0,6.0);			
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==5){
			if(waveTicker<1000){
				if(waveTicker % 80==0){
					spawn_tank(thex,they,theang,1.8,120);
				}
				if(waveTicker % 60==0){
					spawn_grunt1(thex,they,theang,2,grunt4Image,270);
				}
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==6){
			if(waveTicker % 60==0){
				spawn_splitter(thex,they,theang,2.0);

			}
			waveResetTime=waveTicker;

		}else if(waveNumber==7){
			if(waveTicker<500){
				if(waveTicker % 20==0){
					spawn_grunt6(thex,they,theang,3.0);
				}
			}
			if(waveTicker>400 && waveTicker<1000){
				if(waveTicker % 10==0){
					spawn_grunt6(thex,they,theang,3.0);
				}
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==8){
			if(waveTicker % 40==0){
				if(waveTicker<500){
					spawn_electrode(thex,they,theang,3.5);				
				}else{
					spawn_electrode(thex,they,theang,4.5);
				}
			}
			waveResetTime=waveTicker;

		}else if(waveNumber==9){
			if(waveTicker<500){
				if(waveTicker % 20==0){
					spawn_grunt7(thex,they,theang,3.0);
				}
			}
			if(waveTicker>400 && waveTicker<1000){
				if(waveTicker % 10==0){
					spawn_grunt7(thex,they,theang,3.0);
				}
			}
			// if(waveTicker % 5==0 ){
			// 	spawn_grunt7(thex,they,theang,4);				
			// }
			// if(waveTicker % 160==0 && waveTicker>0 && waveTicker<900){
			// //	spawn_spreader(thex,they,theang,1.0,20);				
			// }
			waveResetTime=waveTicker;

		}else if(waveNumber==10){
			if(waveTicker<70){
				spawn_orbiter( 1,60,waveTicker*6);

			}

			if(waveTicker>70){
				int ecnt=0;
				for(int i=0;i<arrlen(enemies);i++){
					enemy *e=(enemy*)&enemies[i];
					if(e->type!=ENEMY_SHOT){
						ecnt++;
					}
				}
				//no non-shot enemies left, so skip to the end.
				if(ecnt==0){

					if(waveTicker<1060){
						if(arrlen(enemies)>0){
							enemy_kill_first_num(5);
						}else{
							waveTicker=1060;
						}
					}
				}
			}

			waveResetTime=0;

		}else if(waveNumber==11){
			if(waveTicker<600){
					waveTicker=600;
			}

			if(waveTicker==100){
				char *msg="Bonus Wave!";
				message_init(msg,SDL_strlen(msg),640,500,120,1);
			}

			if(waveTicker % 4==0){

				bonus_init(BONUS_SCORE,thex,they);
			}

		}
	}

	

	return 0;
}