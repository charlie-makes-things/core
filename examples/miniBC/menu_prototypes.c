
void about_init();
void about_draw(int ticks);
game_state about_update(cg_controller *active);

void menu_init();
void menu_draw(int ticks);
game_state menu_update(cg_controller *active,int currTime);

void title_init();
void title_draw(int ticks);
game_state title_update(cg_controller *active);

void scores_init();
void scores_draw(int ticks);
game_state scores_update(cg_controller *active);

void settings_init();
void settings_draw(int ticks,SDL_Window *w);
game_state settings_update(cg_controller *active,SDL_Window *w);

void score_entry_init(Uint64 score,SDL_Window *win);
void score_entry_draw(int ticks);
game_state score_entry_update(cg_controller *active,SDL_Window *win);

void transition_init(game_state toState,int startTime,Uint64 tScore);
void transition_draw(int ticks);
game_state transition_update(int currTime,SDL_Window *win);

void game_init();
game_state game_update(cg_controller *active,SDL_Window *w,int ticks);
void game_draw(int ticks);