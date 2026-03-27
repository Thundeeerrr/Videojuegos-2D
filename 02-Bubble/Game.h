#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Menu.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


enum GameState { STATE_MENU, STATE_PLAYING, STATE_INSTRUCTIONS, STATE_CREDITS };


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() {}
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);
	void changeState(GameState newState);

	bool getKey(int key) const;

private:
	bool bPlay; // Continue to play game?
    bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
								// we can have access at any time
	GameState currentState;
	Menu menu;
	Scene scene;

	bool insideRoom;
};


#endif // _GAME_INCLUDE


