#pragma once
#include <DirectXMath.h>
#include <thread>
#include <mutex>
#include "../State.h"
#include "../../Camera/Camera.h"
#include "../../Mesh/MeshGroup.h"

const int NUM_BOXES = 1;

class CubeState : public State {
private:
	//Scene
	std::vector<std::unique_ptr<MeshGroup>> m_scene;
	std::unique_ptr<Camera> m_camera;

	//Transformation
	struct Transformation {
		XMFLOAT4 rotation;
		XMFLOAT4 translation;
	} m_transformationMatrix[NUM_BOXES];

	ComPtr<ID3D12Resource2> m_ACHeap[2];
	ComPtr<ID3D12Resource2> m_ACBuffer[2]; //One for consuming and one for appending

	ComPtr<ID3D12PipelineState> m_computeStateObject;

	struct recordingThread {
		std::thread* m_thread = nullptr;
		std::mutex m_mutex;
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