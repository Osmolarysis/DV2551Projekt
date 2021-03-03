#include <crtdbg.h>
#include "Renderer/Renderer.h"

int CALLBACK main(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Check for memory leaks

	//Renderer
	Renderer* renderer = Renderer::getInstance();

	//Windows handler
	MSG msg = { 0 };
	bool mainLoop = true;

	while (mainLoop) { //TODO: exit while loop in a good way - statestacks if we feelin fancy
		//Update

		//Begin frame

		//Record

		//Execute list(s)

		//Present

		//Message handling
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (msg.message) {
			case WM_QUIT: {
				mainLoop = false;
				break;
			}
			}
		}
	}

	printf("Exiting main...\n");
	return 0;
}