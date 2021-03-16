#include <crtdbg.h>
#include "Renderer/Renderer.h"
#include "States\StateStack.h"
#include "Utility\Timer.h"
#include "Utility\Input.h"

int CALLBACK main(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Check for memory leaks
	
	Input::initialise();

	//Timer
	Timer* timer = Timer::getInstance();

	//Input
	Input* input = Input::getInstance();

	//Renderer
	Renderer* renderer = Renderer::getInstance();

	//StateStack
	StateStack* stateStack = StateStack::getInstance();
	stateStack->push(State::StateType::cubeState);

	//Windows handler
	MSG msg = { 0 };
	bool mainLoop = true;

	timer->reset();
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