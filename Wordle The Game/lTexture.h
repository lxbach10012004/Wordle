#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<string>
using namespace std;

class lTexture{
    SDL_Texture* dataTexture;
    int width;
    int height;

public:
    string text;
    lTexture();
    ~lTexture();

    void free();
    int getWidth();
    int getHeight();

    bool loadFromFile(string path);
    #if defined(SDL_TTF_MAJOR_VERSION)
    bool loadFromText(string text, SDL_Color textColor);
    #endif

//    void setColor(Uint8 red, Uint8 green, Uint8 blue);
//    void setBlendMode(SDL_BlendMode blending);
//    void setAlpha(Uint8 alpha);

    void render(int x, int y, SDL_Rect* clip);
//    void renderAngle(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip);
};
