
typedef struct blob{
    float x,y;
    float sx,sy;
    float deg;
    float rad;
    float spd;
    int r,g,b;
    float alpha;
}blob;

//blobs for menu background
static Uint32 BLOB_COUNT=1000;
static blob *blobs=NULL;//stretchy buffer for blobs. see stb_ds.h for info

typedef enum cg_audio_type{
    AUDIO_TYPE_MUSIC,
    AUDIO_TYPE_SFX
}cg_audio_type;

typedef struct cg_audio_track{
    MIX_Track *track;
    MIX_Audio *audio;
    cg_audio_type type;
    float vol;
}cg_audio_track;

cg_audio_track *_currentMusic=NULL;
cg_audio_track *_audioList=NULL;

typedef struct high_score_entry{
    char name[32];
    Uint64 score;
}high_score_entry;

high_score_entry *highScoreTable=NULL;

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

float sfxGain=1.0f;
float musicGain=1.0f;

typedef struct player{
    float x,y;
    float xvel,yvel;
    float ang;
    bool active;
    Uint64 score;
    int multiplier;
    int nextMultiplier;
    int lives;
}player;

player p1;

typedef struct particle{
    float x,y;
    float xvel,yvel;
    float ang;
    float angVel;
    float sz;
    float szvel;
    float alpha;
    float alphaVel;
    int life;
    int r,g,b;
    cg2d_image *img;
}particle;

particle *particles=NULL;//stretchy buffer - remenber to free!

