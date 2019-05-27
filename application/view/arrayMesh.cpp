#include "arrayMesh.h"
#include "../util/Log.h"

ArrayMesh::ArrayMesh(const float* vertices, const float* uv, const unsigned int vertexCount, const unsigned int dimensions) : AbstractMesh(), vertexCount(vertexCount) {
	vertexBuffer = new VertexBuffer(vertices, vertexCount * dimensions);
	uvBuffer = new VertexBuffer(uv, vertexCount * 2);

	vertexBufferLayout.push<float>(dimensions);
	uvBufferLayout.push<float>(2);

	vertexArray->addBuffer(*vertexBuffer, vertexBufferLayout);
	vertexArray->addBuffer(*uvBuffer, uvBufferLayout);
}

ArrayMesh::ArrayMesh(const float* vertices, const unsigned int vertexCount, const unsigned int dimensions, RenderMode renderMode) : AbstractMesh(renderMode), vertexCount(vertexCount) {
	vertexBuffer = new VertexBuffer(vertices, vertexCount * dimensions);

	vertexBufferLayout.push<float>(dimensions);
	vertexArray->addBuffer(*vertexBuffer, vertexBufferLayout);
}

void ArrayMesh::render() {
	vertexArray->bind();
	glDrawArrays((int) renderMode, 0, vertexCount);
}

void ArrayMesh::close() {
	vertexBuffer->close();
	vertexArray->close();
}