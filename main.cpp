#include <crtdbg.h>
#include "Renderer.h"

int CALLBACK main(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Check for memory leaks

	Renderer* renderer = Renderer::getInstance();

	while (1) { //TODO: exit while loop in a good way
		//Update

		//Begin frame

		//Record

		//Execute list(s)

		//Present
	}

	printf("Exiting main...\n");
	return 0;
}