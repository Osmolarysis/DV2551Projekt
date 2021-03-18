#include <crtdbg.h>
#include "Renderer/Renderer.h"
#include "States\StateStack.h"
#include "Utility\Timer.h"
#include "Utility\Input.h"

//unsigned int wang_hash(unsigned int seed) //Random generator TODO remove
//{
//	seed = (seed ^ 61) ^ (seed >> 16);
//	seed *= 9;
//	seed = seed ^ (seed >> 4);
//	seed *= 0x27d4eb2d;
//	seed = seed ^ (seed >> 15);
//	return seed;
//}

int CALLBACK main(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Check for memory leaks



	//Timer
	Timer* timer = Timer::getInstance();

	//Renderer
	Renderer* renderer = Renderer::getInstance();

	//Input
	Input::initialise(renderer->getWindowHandle());
	Input* input = Input::getInstance();

	//StateStack
	StateStack* stateStack = StateStack::getInstance();
	stateStack->push(State::StateType::cubeState);

	//Windows handler
	MSG msg = { 0 };
	bool mainLoop = true;

	timer->reset();

	// Closes and executes commandlists
	Renderer::getInstance()->closeCommandLists();

	/*for (size_t i = 0; i < 16; i++) //TODO: Remove
	{
		unsigned int thread = i;
		float randomx = wang_hash(thread);
		float randomy = wang_hash(randomx);
		float randomz = wang_hash(randomy);

		randomx = randomx * (1.0f / 4294967296.0f);
		randomy = randomy * (1.0f / 4294967296.0f);
		randomz = randomz * (1.0f / 4294967296.0f);

		printf("x: %f, y: %f, z: %f\n", randomx, randomy, randomz);
	}*/

	while (mainLoop) { //TODO: exit while loop in a good way - statestacks if we feelin fancy
		//Update
		timer->update();
		input->update();
		stateStack->update();

		//Begin frame
		renderer->beginFrame();

		//Record (TODO: multithread)
		stateStack->record();

		//Execute list(s)
		//stateStack->executeList();
		renderer->executeList();

		//Present
		renderer->present();

		//Message handling
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (msg.message) {
			case WM_QUIT: {
				mainLoop = false;
				renderer->waitForGPU();
				break;
			}
			}
		}
	}

	printf("Exiting main...\n");
	return 0;
}
