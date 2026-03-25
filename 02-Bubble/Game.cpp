#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
   currentState = STATE_MENU;
	menu.init();
}

bool Game::update(int deltaTime)
{
    if(currentState == STATE_MENU)
		menu.update(deltaTime);
	else if(currentState == STATE_PLAYING)
		scene.update(deltaTime);

	return bPlay;
}

void Game::render()
{
 GLFWwindow *window;
	int framebufferWidth, framebufferHeight;

	window = glfwGetCurrentContext();
	if(window != NULL)
	{
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		glViewport(0, 0, framebufferWidth, framebufferHeight);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(currentState == STATE_MENU)
		menu.render();
	else if(currentState == STATE_PLAYING)
		scene.render();
}

void Game::keyPressed(int key)
{
   keys[key] = true;

	if(key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;

	if(currentState == STATE_MENU)
		menu.keyPressed(key);
	else if(currentState == STATE_PLAYING)
	{
		if(key == GLFW_KEY_G)
			scene.toggleGodMode();
		else if(key == GLFW_KEY_K)
			scene.giveAllKeys();
		else if(key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
			scene.loadLevel(key - GLFW_KEY_1 + 1);
	}
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}

void Game::changeState(GameState newState)
{
	if(newState == STATE_PLAYING)
		scene.init();

	currentState = newState;
}



