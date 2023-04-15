#include "lTexture.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>

using namespace std;

extern SDL_Renderer* gRenderer;
extern TTF_Font* gFont;

lTexture::lTexture(){
    dataTexture = NULL;
    width = 0;
    height = 0;
}
lTexture::~lTexture(){
    free();
}

void lTexture::free(){
    if(dataTexture != NULL){
        SDL_DestroyTexture(dataTexture);
        dataTexture = NULL;
        width = 0;
        height = 0;
    }
}

int lTexture::getWidth(){
    return width;
}

int lTexture::getHeight(){
    return height;
}

bool lTexture::loadFromFile(string path){
    free();
    bool success = 1;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if(loadedSurface == NULL){
        cout << "Failed to load image from file: " << IMG_GetError() << endl;
        success = 0;
    }
    else{
        width = loadedSurface -> w;
        height = loadedSurface -> h;
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface -> format, 0, 255, 255));
        dataTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if(dataTexture == NULL){
            cout << "Failed to create Texture from Surface: " << SDL_GetError() << endl;
            success = 0;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return success;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool lTexture::loadFromText(string text, SDL_Color textColor){
    free();
    bool success = 1;
    SDL_Surface* loadedText = TTF_RenderText_Blended(gFont, text.c_str(), textColor);
    if(loadedText == NULL){
        cout << "Failed to load from text: " << TTF_GetError() << endl;
        success = 0;
    }
    else{
        dataTexture = SDL_CreateTextureFromSurface(gRenderer, loadedText);
        if(dataTexture == NULL){
            cout << "Failed to create texture from surface: " << SDL_GetError() << endl;
            success = 0;
        }
        else{
            width = loadedText -> w;
            height = loadedText -> h;
        }
        SDL_FreeSurface(loadedText);
    }
    return success;
}
#endif

//void lTexture::setColor(Uint8 red, Uint8 green, Uint8 blue){
//    SDL_SetTextureColorMod( dataTexture, red, green, blue );
//}

//void lTexture::setBlendMode(SDL_BlendMode blending){
//    SDL_SetTextureBlendMode(dataTexture, blending);
//}

//void lTexture::setAlpha(Uint8 alpha){
//    SDL_SetTextureAlphaMod( dataTexture, alpha );
//}

void lTexture::render(int x, int y, SDL_Rect* clip){
    SDL_Rect desRect = {x, y, width, height};
    if(clip != NULL){
        desRect.w = clip -> w;
        desRect.h = clip -> h;
    }
    SDL_RenderCopy(gRenderer, dataTexture, clip, &desRect);
}

//void lTexture::renderAngle(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip){
//	SDL_Rect renderQuad = { x, y, width, height };
//
//	if( clip != NULL )
//	{
//		renderQuad.w = clip->w;
//		renderQuad.h = clip->h;
//	}
//
//	SDL_RenderCopyEx( gRenderer, dataTexture, clip, &renderQuad, angle, center, flip );
//}
