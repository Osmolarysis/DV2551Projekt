#pragma once
class VertexBuffer
{
private:
	unsigned int refs = 0;


public:
	VertexBuffer();
	~VertexBuffer();

	// just taken from assignment 1
	void setData(const void* data, size_t size, size_t offset);
	void bind(size_t offset, size_t size, unsigned int location);
	void unbind();
	size_t getSize();
	void incRef() { refs++; };
	void decRef() { if (refs > 0) refs--; };
	inline unsigned int refCount() { return refs; };
};

