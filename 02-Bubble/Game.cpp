#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
   glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   intro.init();
	credits.init();
 instructions.init();
	currentState = STATE_INTRO;
}

bool Game::update(int deltaTime)
{
  switch(currentState)
	{
	case STATE_INTRO:
		intro.update(deltaTime);
		if(intro.isFinished())
            changeState(STATE_CREDITS);
		break;
	case STATE_MENU:
		menu.update(deltaTime);
		break;
	case STATE_PLAYING:
		scene.update(deltaTime);
		break;
 case STATE_INSTRUCTIONS:
		instructions.update(deltaTime);
		break;
	case STATE_CREDITS:
		credits.update(deltaTime);
		if(credits.isFinished())
			changeState(STATE_MENU);
		break;
	default:
		break;
	}

	return bPlay;
}

void Game::render()
{
	GLFWwindow* window;
	int framebufferWidth, framebufferHeight;

	window = glfwGetCurrentContext();
	if (window != NULL)
	{
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		glViewport(0, 0, framebufferWidth, framebufferHeight);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  switch(currentState)
	{
	case STATE_INTRO:
		intro.render();
		break;
	case STATE_MENU:
		menu.render();
		break;
	case STATE_PLAYING:
		scene.render();
		break;
 case STATE_INSTRUCTIONS:
		instructions.render();
		break;
	case STATE_CREDITS:
		credits.render();
		break;
	default:
		break;
	}
}

void Game::keyPressed(int key)
{
   keys[key] = true;

	if(key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;

  if(currentState == STATE_INTRO)
		intro.skip();
	else if(currentState == STATE_CREDITS)
		credits.skip();
	else if(currentState == STATE_MENU)
		menu.keyPressed(key);
  else if(currentState == STATE_INSTRUCTIONS)
	{
		if(key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_BACKSPACE)
			changeState(STATE_MENU);
	}
	else if(currentState == STATE_PLAYING)
	{
		if(key == GLFW_KEY_G)
			scene.toggleGodMode();
		else if(key == GLFW_KEY_K)
			scene.giveAllKeys();
        else if(key == GLFW_KEY_R)
			scene.loadLevel(0);
     else if(key >= GLFW_KEY_1 && key <= GLFW_KEY_9)
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
       scene.loadLevel(1);
	if(newState == STATE_INTRO)
		intro.init();
	if(newState == STATE_CREDITS)
		credits.init();
  if(newState == STATE_INSTRUCTIONS)
		instructions.init();
	if(newState == STATE_MENU)
		menu.init();

	currentState = newState;
}



