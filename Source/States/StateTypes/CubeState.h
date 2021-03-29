#pragma once
#include <DirectXMath.h>
#include <thread>
#include <mutex>
#include "../State.h"
#include "../../Camera/Camera.h"
#include "../../Mesh/MeshGroup.h"

//const int NUM_BOXES = 64*64*64;

class CubeState : public State {
private:
	//Scene
	std::vector<std::unique_ptr<MeshGroup>> m_scene;
	std::unique_ptr<Camera> m_camera;

	XMMATRIX m_transformationMatrix[NUM_INSTANCE_CUBES] = { XMMatrixIdentity() };

	ComPtr<ID3D12Resource2> m_ComputeGameLogicUpdateHeap;
	ComPtr<ID3D12Resource2> m_ComputeGameLogicUpdateBuffer;
	ComPtr<ID3D12Resource2> m_ComputeGameLogicReadBuffer[2];

	ComPtr<ID3D12PipelineState> m_computeStateObject;

	int m_backBufferIndex = 0;

	struct recordingThread {
		std::thread* m_thread = nullptr;
		std::mutex m_mutex;
		bool isActive = true; //Should we NOT destroy it?
	};
	recordingThread m_copyThread;
	recordingThread m_computeThread;
	recordingThread m_directThread;

	ComPtr<ID3D12Fence1> m_threadFence[2];
	UINT64 m_fenceValue[2] = { 1,1 };

	void copyRecord();
	void computeRecord();
	void directRecord();

	std::shared_ptr<VertexBuffer> createBox(float width, float hight, float depth);
	void initiateTransformMatrices();
public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};