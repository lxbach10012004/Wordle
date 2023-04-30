#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <ctime>
#include <stdlib.h>
#include <algorithm>
#include "lTexture.h"
using namespace std;

//Window Size
const int WindowSizeW = 1536;
const int WindowSizeH = 864;

//Letter blocks settings
int blockSize = 70;
int blockSpacing = 5;
int numBlocks = 5;

//Keyboard letters' order
char keyboardLetter[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
                         'a', 's' , 'd', 'f', 'g', 'h', 'j', 'k', 'l',
                         'z', 'x', 'c', 'v', 'b', 'n', 'm'};

struct Key{
    SDL_Rect pos;
    char letter;
};
Key keyboardBox[26];
SDL_Rect Enter, BackSpace;
bool initKeyboard = 1;

//Some colors
SDL_Color BLACK = {0, 0, 0};
SDL_Color WHITE = {254, 254, 254};
SDL_Color GREY = {60, 60, 60};
SDL_Color YELLOW = {195, 160, 0};
SDL_Color GREEN = {0, 160, 0};
SDL_Color RED = {200, 0, 0};

//General
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
Mix_Music* backgroundMusic = NULL;
Mix_Chunk* clickEff = NULL;
Mix_Chunk* loseEff = NULL;
Mix_Chunk* winEff = NULL;
Mix_Chunk* win2Eff = NULL;

//Store some words texture
lTexture lettersBlack[26];
lTexture lettersWhite[26];
lTexture youWin, youLose, theLetterWas, numberOfGuesses, pressPlayAgain, startingScreen, pressAnyToStart;
lTexture backdrop, WIN, LOSE;

SDL_Cursor* defaultCursor;
SDL_Cursor* clickCursor;

//Load words from text file used to generate secret words
void questions(vector<string> &hidden, int y){
    ifstream questionsFile;
    switch(y){
    case 1:
        questionsFile.open("wordle-list-main/animals.txt");
        break;
    case 2:
        questionsFile.open("wordle-list-main/foods.txt");
        break;
    case 3:
        questionsFile.open("wordle-list-main/items.txt");
        break;
    case 5:
        questionsFile.open("wordle-list-main/questions.txt");
    }
    string inputString;
    while(getline(questionsFile, inputString))
        hidden.push_back(inputString);
    questionsFile.close();
    return;
}

//Load words from text file used to check if a word guess is valid
void possibleGuess(vector<string> &guesses){
    ifstream guessesFile;
    guessesFile.open("wordle-list-main/possibleGuesses.txt");
    if(!guessesFile.is_open()){
        cout << "Failed to open guesses file!\n";
        return;
    }
    string inputString;
    while(getline(guessesFile, inputString))
        guesses.push_back(inputString);
    guessesFile.close();
    return;
}

void makeHint(vector<string> &allHints, int y){
    ifstream questionsFile;
    switch(y){
    case 1:
        questionsFile.open("wordle-list-main/animals-hint.txt");
        break;
    case 2:
        questionsFile.open("wordle-list-main/foods-hint.txt");
        break;
    case 3:
        questionsFile.open("wordle-list-main/items-hint.txt");
        break;
    }

    string inputString;
    while(getline(questionsFile, inputString))
        allHints.push_back(inputString);
    questionsFile.close();
    return;
}

//Check if a character is a letter and convert it to lower case
bool isLetterAndLowerCase(char &x){
    if(x >= 'a' && x <= 'z')
        return 1;
    else if(x >= 'A' && x <= 'Z'){
        x += 32;
        return 1;
    }
    return 0;
}

//Convert a string to upper case
string convertToUpper(string words){
    int len = words.length();
    for(int i = 0; i < len; i++){
        if(words[i] >= 'a' && words[i] <= 'z')
            words[i] -= 32;
    }
    return words;
}

//Initialize SDL, IMG and TTF
bool init(){
    bool success = 1;
    //Init SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        cout << "Failed to init SDL: " << SDL_GetError() << endl;
        success = 0;
    }
    else{
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			cout << "Warning: Linear texture filtering not enabled!\n";
		}
        //Create window
        gWindow = SDL_CreateWindow("Wordle", 0, 0, WindowSizeW, WindowSizeH, SDL_WINDOW_SHOWN);
//        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        if(gWindow == NULL){
            cout << "Failed to create window: " << SDL_GetError() << endl;
            success = 0;
        }
        else{
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if(gRenderer == NULL){
                cout << "Failed to link renderer to window: " << SDL_GetError() << endl;
                success = 0;
            }
            else{
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
                SDL_RenderClear(gRenderer);

                //Init SDL_IMG
                int imgFlag = IMG_INIT_PNG;
                if(!(imgFlag & IMG_Init(imgFlag))){
                    cout << "Failed to init IMG: " << IMG_GetError() << endl;
                    success = 0;
                }

                //Init SDL_TTF
                if(TTF_Init() == -1){
                    cout << "Failed to init TTF: " << TTF_GetError() << endl;
                    success = 0;
                }

                //Init SDL_MIX
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
            }
        }
    }
    return success;
}

//Load some medias
bool loadMedia(){
    bool success = 1;

    SDL_Surface* cursorSurface = IMG_Load("Images/cursor1.png");
    defaultCursor = SDL_CreateColorCursor(cursorSurface, 8, 3);
    SDL_SetCursor(defaultCursor);
    SDL_FreeSurface(cursorSurface);

    backdrop.loadFromFile("Images/GamePic.png");
    WIN.loadFromFile("Images/win.png");
    LOSE.loadFromFile("Images/lose.png");
    //Open Font
    TTF_SetFontHinting(gFont, TTF_HINTING_NORMAL);
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Medium.ttf", 40);
    if(gFont == NULL){
        cout << "Failed to open font: " << TTF_GetError() << endl;
        success = 0;
    }
    else{
        //Load black and white letters
        char x = 'A';
        for(int i = 0; i < 26; i++){
            lettersBlack[i].loadFromText(string("") + x, BLACK);
            lettersWhite[i].loadFromText(string("") + x, WHITE);
            x++;
        }
    }
    //Load some sentences
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Bold.ttf", 64);
    youWin.loadFromText("YOU WIN!", BLACK);
    youLose.loadFromText("YOU LOSE!", BLACK);
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Medium.ttf", 28);

    //Load Audio
    backgroundMusic = Mix_LoadMUS("Audio/FunnyMusicBackground.wav");
    if( backgroundMusic == NULL )
    {
        cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError();
        success = false;
    }

    clickEff = Mix_LoadWAV("Audio/click.wav");
    if( clickEff == NULL )
    {
        cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError();
        success = false;
    }

    loseEff = Mix_LoadWAV("Audio/lose.wav");
    if( loseEff == NULL )
    {
        cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError();
        success = false;
    }

    winEff = Mix_LoadWAV("Audio/win.wav");
    if( winEff == NULL )
    {
        cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError();
        success = false;
    }

    win2Eff = Mix_LoadWAV("Audio/win2.wav");
    if( win2Eff == NULL )
    {
        cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError();
        success = false;
    }

    return success;
}

//Free memory
void close(){
    //free black and white letters
    for(int i = 0; i < 26; i++){
        delete &lettersBlack[i];
        delete &lettersWhite[i];
    }
    //free sentences
    delete &youLose;
    delete &youWin;
    delete &theLetterWas;
    delete &pressPlayAgain;


    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(clickEff);
    Mix_FreeChunk(loseEff);
    Mix_FreeChunk(winEff);
    Mix_FreeChunk(win2Eff);
    backgroundMusic = NULL;
    clickEff = NULL;
    loseEff = NULL;
    winEff = NULL;
    win2Eff = NULL;

    //Destroy everything
    TTF_CloseFont(gFont);
    gFont = NULL;
    SDL_DestroyWindow(gWindow);
    SDL_DestroyRenderer(gRenderer);
    gWindow = NULL;
    gRenderer = NULL;

    //Close TTF, IMG and SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


int renderMainMenu() {
    int num_frames = 31;
    SDL_Texture* textures[num_frames];
    for(int i = 1; i < 22; i++) {
        std::string filename = "Videos/MainMenuIn/scene" + std::to_string(i + 4) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    int fps = 30;
    int length = 5;

    const int SCREEN_TICKS_PER_FRAME = 1000 / fps;
    Uint32 frame_start_time = SDL_GetTicks();
    bool quit = 0;

    for(int i = 1; i < 22; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    SDL_Rect startGame = {732, 260, 481, 107};
    SDL_Rect howToPlay = {732, 409, 481, 107};
    SDL_Rect quitButton = {732, 546, 481, 107};
    quit = 0;
    SDL_Event event;
    int retVal;
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT) quit = 1;
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                Mix_PlayChannel(-1, clickEff, 0);
                int x = event.button.x;
                int y = event.button.y;
                if(x >= startGame.x && x <= startGame.x + startGame.w && y >= startGame.y && y <= startGame.y + startGame.h){
                    retVal = 1;
                    quit = 1;
                }
                else if(x >= howToPlay.x && x <= howToPlay.x + howToPlay.w && y >= howToPlay.y && y <= howToPlay.y + howToPlay.h){
                    retVal = 2;
                    quit = 1;
                }
                else if(x >= quitButton.x && x <= quitButton.x + quitButton.w && y >= quitButton.y && y <= quitButton.y + quitButton.h){
                    retVal = 3;
                    quit = 1;
                }
            }
        }
    }

    for(int i = 5; i < 23; i++) {
        std::string filename = "Videos/MainMenuOut/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    quit = 0;
    for(int i = 5; i < 23; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    return retVal;
}

int renderChooseTopic(){
    int num_frames = 33;
    SDL_Texture* textures[num_frames];
    for(int i = 1; i < num_frames; i++) {
        std::string filename = "Videos/ChooseTopicIn/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    int fps = 30;
    int length = 5;

    const int SCREEN_TICKS_PER_FRAME = 1000 / fps;
    Uint32 frame_start_time = SDL_GetTicks();
    bool quit = 0;

    for(int i = 1; i < num_frames; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    SDL_Rect startGame = {740, 187, 501, 110};
    SDL_Rect howToPlay = {740, 338, 418, 110};
    SDL_Rect quitButton = {740, 492, 399, 110};
    SDL_Rect backButton = {25, 29, 230, 96};
    SDL_Rect freeButton = {173, 508, 441, 141};
    quit = 0;
    SDL_Event event;
    int retVal;
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT) quit = 1;
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                Mix_PlayChannel(-1, clickEff, 0);
                int x = event.button.x;
                int y = event.button.y;
                if(x >= startGame.x && x <= startGame.x + startGame.w && y >= startGame.y && y <= startGame.y + startGame.h){
                    retVal = 1;
                    quit = 1;
                }
                else if(x >= howToPlay.x && x <= howToPlay.x + howToPlay.w && y >= howToPlay.y && y <= howToPlay.y + howToPlay.h){
                    retVal = 2;
                    quit = 1;
                }
                else if(x >= quitButton.x && x <= quitButton.x + quitButton.w && y >= quitButton.y && y <= quitButton.y + quitButton.h){
                    retVal = 3;
                    quit = 1;
                }
                else if(x >= backButton.x && x <= backButton.x + backButton.w && y >= backButton.y && y <= backButton.y + backButton.h){
                    retVal = 4;
                    quit = 1;
                }
                else if(x >= freeButton.x && x <= freeButton.x + freeButton.w && y >= freeButton.y && y <= freeButton.y + freeButton.h){
                    retVal = 5;
                    quit = 1;
                }
            }
        }
    }

    for(int i = 1; i < 21; i++) {
        std::string filename = "Videos/ChooseTopicOut/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    quit = 0;
    for(int i = 1; i < 21; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    return retVal;
}

void renderHowToPlay(){
    int num_frames = 31;
    SDL_Texture* textures[num_frames];
    for(int i = 1; i < num_frames; i++) {
        std::string filename = "Videos/HowToPlayIn/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    int fps = 30;
    int length = 5;

    const int SCREEN_TICKS_PER_FRAME = 1000 / fps;
    Uint32 frame_start_time = SDL_GetTicks();
    bool quit = 0;

    for(int i = 1; i < num_frames; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    SDL_Rect backButton = {25, 29, 230, 96};
    quit = 0;
    SDL_Event event;

    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT) quit = 1;
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                Mix_PlayChannel(-1, clickEff, 0);
                int x = event.button.x;
                int y = event.button.y;
                if(x >= backButton.x && x <= backButton.x + backButton.w && y >= backButton.y && y <= backButton.y + backButton.h){
                    quit = 1;
                }
            }
        }
    }

    for(int i = 1; i < num_frames; i++) {
        std::string filename = "Videos/HowToPlayOut/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    quit = 0;
    for(int i = 1; i < num_frames; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }
    return;
}

void renderGameIn(){
    int num_frames = 46;
    SDL_Texture* textures[num_frames];
    for(int i = 1; i < num_frames; i++) {
        std::string filename = "Videos/GameIn/scene" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        textures[i] = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_FreeSurface(surface);
    }
    int fps = 30;
    int length = 5;

    const int SCREEN_TICKS_PER_FRAME = 1000 / fps;
    Uint32 frame_start_time = SDL_GetTicks();
    bool quit = 0;

    for(int i = 1; i < num_frames; i++) {
        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if(quit)
            break;

        // render the frame
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, textures[i], nullptr, nullptr);
        SDL_RenderPresent(gRenderer);

        // delay to control the frame rate
        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_time);
        }

        frame_start_time = SDL_GetTicks();
    }

    return;
}

//Draw 5x6 box grid
void drawGrid(){
    for(int j = 0; j < 6; j++){
        int startX = (WindowSizeW - (numBlocks * (blockSize + blockSpacing))) / 2;
        int startY = WindowSizeH / 5 + j * (blockSize + blockSpacing);
        for (int i = 0; i < numBlocks; i++) {
            int x = startX + i * (blockSize + blockSpacing);
            int y = startY;
            SDL_Rect blockRect = { x, y, blockSize, blockSize };
            SDL_SetRenderDrawColor(gRenderer, 248, 248, 248, 255);
            SDL_RenderFillRect(gRenderer, &blockRect);
            SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(gRenderer, &blockRect);
        }
    }
}

//Clear white screen
void reset(){
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
    SDL_RenderClear(gRenderer);
    backdrop.render(0, 0, NULL);
}

//Draw gray, yellow or green boxes
void drawCheckBox(int x, int y, int color){
    if(color == 1)
        SDL_SetRenderDrawColor(gRenderer, GREY.r, GREY.g, GREY.b, 255);
    else if(color == 2)
        SDL_SetRenderDrawColor(gRenderer, YELLOW.r, YELLOW.g, YELLOW.b, 255);
    else if(color == 3)
        SDL_SetRenderDrawColor(gRenderer, GREEN.r, GREEN.g, GREEN.b, 255);
    SDL_Rect box = {x, y, blockSize, blockSize};
    SDL_RenderFillRect(gRenderer, &box);
}

//Check the current word and update the keyboard color
int check(const string inputString, int row, string secretWord, vector<string> allGuesses, map<char, int> &res, bool update){
    int count = 0;
    string currentWord = "";
    for(int i = row * 5; i < row * 5 + 5; i++)
        currentWord += inputString[i];
    if(find(allGuesses.begin(), allGuesses.end(), currentWord) == allGuesses.end())
        return 3;
    int color[5] = {0};
    for(int i = 0; i < 5; i++)
        if(currentWord[i] == secretWord[i]){
            color[i] = 3;
            if(update)
                res[currentWord[i]] = 3;
            secretWord[i] = '*';
            count++;
        }
    for(int i = 0; i < 5; i++){
        if(color[i] != 0) continue;
        else{
            size_t found = secretWord.find(currentWord[i]);
            if(found != string::npos){
                if(update){
                    if(res[currentWord[i]] != 3) res[currentWord[i]] = 2;
                }
                color[i] = 2;
                secretWord[found] = '*';
            }
            else{
                color[i] = 1;
                if(update){
                    if(res[currentWord[i]] == 0)
                        res[currentWord[i]] = 1;
                }
            }
        }
    }

    int startX = (WindowSizeW - (numBlocks * (blockSize + blockSpacing))) / 2;
    int y = WindowSizeH / 5 + row * (blockSize + blockSpacing);
    for(int i = 0; i < 5; i++){
        int x = startX + i * (blockSize + blockSpacing);
        if(color[i] == 3)
            drawCheckBox(x, y, 3);
        else if(color[i] == 2)
            drawCheckBox(x, y, 2);
        else if(color[i] == 1)
            drawCheckBox(x, y, 1);
    }

    int col = 0;
    for(int i = 0; i < (int)currentWord.length(); i++){
        int x = (int)currentWord[i] - int('A') - 32;
        int charWidth, charHeight;
        TTF_SizeText(gFont, string(1, currentWord[i]).c_str(), &charWidth, &charHeight);
        int currentX = startX + col * (blockSize + blockSpacing) + (blockSize - charWidth) / 2 - 4;

        lettersWhite[x].render(currentX, y + 6, NULL);
        col++;
    }
    if(count == 5) return 1;
    else{
        if(row >= 5) return 2;
        return 0;
    }
}

//Render the current keyboard
void renderKeyboard(SDL_Renderer* renderer, map<char, int> res) {

    const int keyWidth = 40;
    const int keyHeight = 60;
    const int paddingX = 5;
    const int paddingY = 10;
    const int startX = (WindowSizeW - 10 * keyWidth - 9 * paddingX) / 2;
    const int startY = WindowSizeH - 3 * keyHeight - 5 * paddingY;

    SDL_Rect keyRect = { startX, startY, keyWidth, keyHeight };
    for (int i = 0; i < 26; i++) {
        bool color = 1;
        char letter = keyboardLetter[i];
        if (res[letter] == 3) {
            //Render the correct letter in green
            SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, 255);
        }
        else if (res[letter] == 2) {
            //Render the incorrect letter in yellow
            SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, 255);
        }
        else if(res[letter] == 1){
            //Render a wrong button in gray
            SDL_SetRenderDrawColor(renderer, GREY.r, GREY.g, GREY.b, 255);
        }
        else{
            //Render a blank button in gray-white
            SDL_SetRenderDrawColor(renderer, 211, 214, 218, 255);
            color = 0;
        }

        SDL_RenderFillRect(renderer, &keyRect);
        if(initKeyboard){
            keyboardBox[i].pos = keyRect;
            keyboardBox[i].letter = letter;
        }
        // Draw the letter inside the button

        string letterString = convertToUpper(string() + letter);
        SDL_Surface* textSurface = TTF_RenderText_Blended(gFont, letterString.c_str(), (color == 1)? WHITE:BLACK);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = { keyRect.x + keyWidth / 2 - textSurface->w / 2, keyRect.y + keyHeight / 2 - textSurface->h / 2, textSurface->w, textSurface->h };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        keyRect.x += keyWidth + paddingX;

        //Endline keyboard
        if (letter == 'p') {
            keyRect.x = startX + keyWidth / 2;
            keyRect.y += keyHeight + paddingY;
        }
        else if(letter == 'l'){
            keyRect.x = startX + keyWidth / 2;
            keyRect.y += keyHeight + paddingY;
            if(initKeyboard) Enter = keyRect;
            keyRect.x = (WindowSizeW - 7 * keyWidth - 6 * paddingX) / 2;

        }
    }
    if(initKeyboard) BackSpace = keyRect;
    SDL_Texture* ent = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("Images/enter.png"));
    SDL_Texture* bsp = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("Images/bspace.png"));
    SDL_RenderCopy(gRenderer, ent, NULL, &Enter);
    SDL_RenderCopy(gRenderer, bsp, NULL, &BackSpace);
    initKeyboard = 0;
}

void renderHint(string hints){
    lTexture h;
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Bold.ttf", 40);
    h.loadFromText(hints, RED);
    h.render(1045, 302, NULL);
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Medium.ttf", 28);
}

//Render win or lose and reveal the secret word
bool renderResult(int win, const string &secretWords, int numGuess){
    Mix_PauseMusic();
    int charWidth, charHeight;
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Bold.ttf", 64);
    string uWin = "YOU WIN!", uLose = "YOU LOSE!", playAgain = "DO YOU WANT TO PLAY AGAIN? (Y/N)";
    //Darken the screen
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);
    SDL_Rect rect = {0, 0, WindowSizeW, WindowSizeH};
    SDL_RenderFillRect(gRenderer, &rect);

    //If win
    if(win == 1){
        TTF_SizeText(gFont, uWin.c_str(), &charWidth, &charHeight);
        WIN.render(154, 86, NULL);
        youWin.render((WindowSizeW - charWidth) / 2, (WindowSizeH - 400) / 2, NULL);
        char x = char(numGuess) + '0';
        string numberGuesses = "NUMBER OF GUESSES: ";
        numberGuesses += string(1, x);
        numberOfGuesses.loadFromText(numberGuesses, BLACK);
        TTF_SizeText(gFont, numberGuesses.c_str(), &charWidth, &charHeight);
        numberOfGuesses.render((WindowSizeW - charWidth) / 2, (WindowSizeH - 400) / 2 + 2.4 * charHeight, NULL);
        Mix_PlayChannel(-1, winEff, 0);
    }
    //If lose
    else if(win == 2){
        TTF_SizeText(gFont, uLose.c_str(), &charWidth, &charHeight);
        LOSE.render(154, 86, NULL);
        youLose.render((WindowSizeW - charWidth) / 2, (WindowSizeH - 400) / 2, NULL);
        Mix_PlayChannel(-1, loseEff, 0);
    }
    //Show the secret letter
    theLetterWas.loadFromText("'" + convertToUpper(secretWords) + "'", RED);
    TTF_SizeText(gFont, ("'" + convertToUpper(secretWords) + "'").c_str(), &charWidth, &charHeight);
    theLetterWas.render((WindowSizeW - charWidth) / 2, (WindowSizeH - 400) / 2 + 1.2 * charHeight, NULL);

    //Ask to play again
    SDL_RenderPresent(gRenderer);
    bool quit = false;
    SDL_Event event;

    SDL_Rect Yes = {401, 565, 332, 178};
    SDL_Rect No = {808, 565, 332, 178};
    bool replay;
    while(quit == false){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT) quit = true;
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                Mix_PlayChannel(-1, clickEff, 0);
                int x = event.button.x;
                int y = event.button.y;
                if(x >= Yes.x && x <= Yes.x + Yes.w && y >= Yes.y && y <= Yes.y + Yes.y + Yes.h){
                    replay = 1;
                    quit = 1;
                }
                else if(x >= No.x && x <= No.x + No.w && y >= No.y && y <= No.y + No.y + No.h){
                    replay = 0;
                    quit = 1;
                }
            }
        }
    }
    Mix_ResumeMusic();
    gFont = TTF_OpenFont("TrueTypeFonts/ClearSans-Medium.ttf", 28);
    return replay;
}

//Main function
int main(int argc, char* args[]){
    if(!init()){
        cout << "Failed in initialization step!\n";
    }
    else{
        if(!loadMedia()){
            cout << "Failed in load-media step!\n";
        }
        else{
            Mix_PlayMusic(backgroundMusic, -1);
            MAINMENU:
            int x = renderMainMenu();

            int y;

            if(x == 1){
                TOPIC:
                y = renderChooseTopic();
                if(y == 4)
                    goto MAINMENU;
            }
            if(x == 2){
                renderHowToPlay();
                goto MAINMENU;
            }
            if(x == 3){
                close();
                return 0;
            }
            renderGameIn();
            backdrop.render(0, 0, NULL);
            SDL_RenderPresent(gRenderer);

            //Store words
            vector<string> allQuestions;
            vector<string> allGuesses;
            vector<string> allHints;
            questions(allQuestions, y);
            possibleGuess(allGuesses);
            makeHint(allHints, y);

            //Random seed and randomize secret word
            srand(time(NULL));
            int index = rand() % allQuestions.size();
            string secretWords = allQuestions[index];
            string hints;
            if(y == 5)
                hints = "Good luck! I dont know:<";
            else hints = allHints[index];
//            cout << secretWords;

            //Initialize the keyboard for used letters
            map<char, int> res;
            for(char x = 'a'; x <= 'z'; x++)
                res[x] = 0;

            //Win or lose flag
            int win = 0;

            //Game control flag and event
            bool quit = 0;
            bool enter = 0;
            bool requestHint = 0;
            SDL_Event event;

            //Start receiving text input
            string doneText = "";
            string inputText = "";
            SDL_StartTextInput();
            int fixedMinLength = 0;
            int fixedMaxLength = 5;

            //Create moving background
            SDL_Texture* backgroundTextures[81];

            for (int i = 1; i < 81; i++)
            {
                std::string filename = "Videos/GameLoop/scene" + std::to_string(i) + ".png";
                SDL_Surface* surf = IMG_Load(filename.c_str());
                backgroundTextures[i] = SDL_CreateTextureFromSurface(gRenderer, surf);
                SDL_FreeSurface(surf);
            }

            int currentImage = 1;
            const int fps = 30; // frames per second
            const int frameDelay = 1000 / fps; // milliseconds per frame
            Uint32 frameStart;
            int frameTime;

            SDL_Rect backButton = {25, 29, 230, 96};
            SDL_Rect giveUp = {1271, 31, 190, 90};
            SDL_Rect hintButton = {1271, 150, 190, 90};

            //Main loop
            while(!quit){

                //Flag to decide whether to render the text or not
                reset();
                bool renderText = 0;



                //Waiting for events
                while(SDL_PollEvent(&event) != 0){

                    //The game is played over or the user clicks 'X' button
                    if(event.type == SDL_QUIT){
                        close();
                        return 0;
                    }

                    //Whenever the user press a button
                    else if(event.type == SDL_KEYDOWN){

                            Mix_PlayChannel(-1, clickEff, 0);
                            //Clear screen
                            reset();

                            //Give up
                            if(event.key.keysym.sym == SDLK_ESCAPE){
                                win = 2;
                                quit = 1;
                            }

                            //Trigger enter flag
                            if(event.key.keysym.sym == SDLK_RETURN){
                                enter = 1;
                                renderText = 1;
                            }

                            //Trigger backspace to delete last character
                            if(event.key.keysym.sym == SDLK_BACKSPACE && (int)inputText.length() > fixedMinLength){
                                inputText.pop_back();
                                renderText = 1;
                            }
                        }

                    //Receive text input
                    else if(event.type == SDL_TEXTINPUT){
                            if(enter && win != 3) doneText = inputText;
                            if((int)inputText.length() < fixedMaxLength && isLetterAndLowerCase(event.text.text[0])){
                                inputText += event.text.text;
                                enter = 0;
                                renderText = 1;
                            }
                        }

                    //Receive mouse input
                    else if(event.type == SDL_MOUSEBUTTONDOWN){
                        Mix_PlayChannel(-1, clickEff, 0);
                        int x = event.button.x;
                        int y = event.button.y;

                        //Check click enter

                        if(x >= backButton.x && x <= backButton.x + backButton.w && y >= backButton.y && y <= backButton.y + backButton.h){
                            goto TOPIC;
                        }
                        else if(x >= Enter.x && x < Enter.x + Enter.w && y >= Enter.y && y < Enter.y + Enter.h){
                            enter = 1;
                            renderText = 1;
                        }
                        else if(x >= giveUp.x && x <= giveUp.x + giveUp.w && y >= giveUp.y && y <= giveUp.y + giveUp.h){
                            win = 2;
                            quit = 1;
                        }
                        else if(x >= hintButton.x && x <= hintButton.x + hintButton.w && y >= hintButton.y && y <= hintButton.y + hintButton.h){
                            requestHint = 1;
                        }

                        //Check click backspace
                        else if(x >= BackSpace.x && x < BackSpace.x + BackSpace.w && y >= BackSpace.y && y < BackSpace.y + BackSpace.h){
                            if((int)inputText.length() > fixedMinLength){
                                inputText.pop_back();
                                reset();
                                renderText = 1;
                            }
                        }

                        // Check if the mouse click is within the bounds of any letter on the virtual keyboard
                        else{
                            if(x >= WindowSizeW - 130 && x < WindowSizeW - 9 && y >= 100 && y <= 100 + 45){
                                win = 2;
                                quit = 1;
                            }
                            else for (int i = 0; i < 26; i++) {
                                if (x >= keyboardBox[i].pos.x && x < keyboardBox[i].pos.x + keyboardBox[i].pos.w &&
                                    y >= keyboardBox[i].pos.y && y < keyboardBox[i].pos.y + keyboardBox[i].pos.h){

                                    // Render the pressed letter to the screen
                                    if(enter && win != 3) doneText = inputText;
                                    if((int)inputText.length() < fixedMaxLength){
                                        inputText += keyboardBox[i].letter;
                                        enter = 0;
                                        renderText = 1;
                                    }
                                    break; // exit the loop once a key has been pressed
                                }
                            }
                        }
                    }
                }

                // Start the frame timer
                frameStart = SDL_GetTicks();

                // Render the current background image
                SDL_RenderCopy(gRenderer, backgroundTextures[currentImage], NULL, NULL);
                drawGrid();
                if(requestHint)
                    renderHint(hints);

                // Advance to the next background image
                currentImage++;
                if(currentImage == 81)
                    currentImage = 1;

                // Control the frame rate
                frameTime = SDL_GetTicks() - frameStart;
                if (frameDelay > frameTime)
                {
                    SDL_Delay(frameDelay - frameTime);
                }

                //Render all of the game's features

                //Variables to track current rendering position
                int row = 0, col = 0;
                int startX = (WindowSizeW - (numBlocks * (blockSize + blockSpacing))) / 2;
                int startY = WindowSizeH / 5 + 6;

                //Render the whole word
                for(int i = 0; i < (int)inputText.length(); i++){
                    int x = (int)inputText[i] - int('A') - 32;
                    int charWidth, charHeight;
                    TTF_SizeText(gFont, string(1, inputText[i]).c_str(), &charWidth, &charHeight);
                    int currentX = startX + col * (blockSize + blockSpacing) + (blockSize - charWidth) / 2 - 4;
                    int currentY = startY + row * (blockSize + blockSpacing);

                    lettersBlack[x].render(currentX, currentY, NULL);
                    col++;

                    //Render check-color box for the substring that has been checked
                    check(doneText, row, secretWords, allGuesses, res, 0);

                    //Endline
                    if (col == numBlocks){
                        col = 0;

                        //Render check-color box for the newly input word only if pressing ENTER
                        if(enter && inputText.length() % 5 == 0 && row == (int)inputText.length() / 5 - 1){
                            win = check(inputText, row, secretWords, allGuesses, res, 1);

                            //If win or lose
                            if(win == 1 || win == 2)
                                quit = 1;

                            //If guess wrong and still have chances, update the min and max length of input text
                            if(win == 0){
                                fixedMinLength = inputText.length();
                                fixedMaxLength = fixedMinLength + 5;
                            }
                        }
                        row++;
                    }
                }

                //Render the keyboard and update the screen
                renderKeyboard(gRenderer, res);
                SDL_RenderPresent(gRenderer);
            }
            SDL_StopTextInput();

            //Ask to replay
            int numGuess = (int)inputText.length() / 5;
            bool replay = renderResult(win, secretWords, numGuess);
            if(replay == 1) goto TOPIC;
            else{
                quit = 1;
                goto MAINMENU;
            }
        }
    }
    close();
    return 0;
}



