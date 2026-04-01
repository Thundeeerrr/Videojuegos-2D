#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "Game.h"


#define TARGET_FRAMERATE 60.0f

namespace
{
	ma_engine* gAudioEngine = nullptr;
	bool gIsAudioMuted = false;
	const float kMutedVolume = 0.f;
	const float kUnmutedVolume = 1.f;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_M && gAudioEngine != nullptr)
		{
			if(gIsAudioMuted)
				ma_engine_set_volume(gAudioEngine, kUnmutedVolume);
			else
				ma_engine_set_volume(gAudioEngine, kMutedVolume);

			gIsAudioMuted = !gIsAudioMuted;
		}

		Game::instance().keyPressed(key);
    }
	else if (action == GLFW_RELEASE)
		Game::instance().keyReleased(key);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game::instance().mouseMove(int(xpos), int(ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().mousePress(button);
	else if (action == GLFW_RELEASE)
		Game::instance().mouseRelease(button);
}


int main(void)
{
	GLFWwindow* window;
	ma_engine engine;
   ma_sound menuMusic;
	ma_sound gameplayMusic;
	double timePerFrame = 1.f / TARGET_FRAMERATE, timePreviousFrame, currentTime;
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		// Handle audio init failure
		return -1;
	}
 if(ma_sound_init_from_file(&engine, "bgm/bgm.mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &menuMusic) != MA_SUCCESS)
	{
		ma_engine_uninit(&engine);
		return -1;
	}
	if(ma_sound_init_from_file(&engine, "bgm/Bugs Bunny Crazy Castle 3 - Garden Level - Gerudochu.mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &gameplayMusic) != MA_SUCCESS)
	{
		ma_sound_uninit(&menuMusic);
		ma_engine_uninit(&engine);
		return -1;
	}
	ma_sound_set_looping(&menuMusic, MA_TRUE);
	ma_sound_set_looping(&gameplayMusic, MA_TRUE);
    gAudioEngine = &engine;
	gIsAudioMuted = false;
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Set window initial position */
	glfwSetWindowPos(window, 100, 100);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Set callbacks */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	/* Init glew to have access to GL extensions */
	glewExperimental = GL_TRUE;
	glewInit();

	/* Init step of the game loop */
	Game::instance().init();
	timePreviousFrame = glfwGetTime();
 ma_sound_start(&menuMusic);
	bool wasPlaying = false;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		if (currentTime - timePreviousFrame >= timePerFrame)
		{
			/* Update & render steps of the game loop */
			if(!Game::instance().update(int(1000.0f * (currentTime - timePreviousFrame))))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
          const bool isPlaying = (Game::instance().getCurrentState() == STATE_PLAYING);
			if(isPlaying != wasPlaying)
			{
				if(isPlaying)
				{
					ma_sound_stop(&menuMusic);
					ma_sound_seek_to_pcm_frame(&menuMusic, 0);
					ma_sound_start(&gameplayMusic);
				}
				else
				{
					ma_sound_stop(&gameplayMusic);
					ma_sound_seek_to_pcm_frame(&gameplayMusic, 0);
					ma_sound_start(&menuMusic);
				}
				wasPlaying = isPlaying;
			}

			if(isPlaying && !ma_sound_is_playing(&gameplayMusic))
			{
				ma_sound_seek_to_pcm_frame(&gameplayMusic, 0);
				ma_sound_start(&gameplayMusic);
			}
			else if(!isPlaying && !ma_sound_is_playing(&menuMusic))
			{
				ma_sound_seek_to_pcm_frame(&menuMusic, 0);
				ma_sound_start(&menuMusic);
			}
			Game::instance().render();
			timePreviousFrame = currentTime;

			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		}

		/* Poll for and process events */
		glfwPollEvents();
	}

  ma_sound_uninit(&gameplayMusic);
	ma_sound_uninit(&menuMusic);
    ma_engine_uninit(&engine);
	glfwTerminate();
	return 0;
}

