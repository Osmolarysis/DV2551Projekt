#pragma once
#include <DirectXMath.h>
#include <thread>
#include <mutex>
#include "../State.h"
#include "../../Camera/Camera.h"
#include "../../Mesh/MeshGroup.h"

class CubeState : public State {
private:
	//Scene
	std::vector<std::unique_ptr<MeshGroup>> m_scene;
	std::unique_ptr<Camera> m_camera;

	struct recordingThread {
		std::thread* m_thread = nullptr;
		std::mutex m_mutex;
		bool isRunning = false; //Is it actually working? or idling?
		bool isActive = true; //Should we NOT destroy it?
	};
	recordingThread m_copyThread;
	recordingThread m_computeThread;
	recordingThread m_directThread;

	void copyRecord();
	void computeRecord();
	void directRecord();

	std::shared_ptr<VertexBuffer> createBox(float width, float hight, float depth);
public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};