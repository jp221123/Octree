#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "object.h"

#include <array>
#include <vector>
#include <unordered_set>

class OctreeNode {
private:
	static constexpr int CAPACITY = 10;
	std::unordered_set<SolidBody*> objects;
	int count{0};
	const std::array<float, 3> center;
	Box boundary;
	const std::array<Box, 1<<3> subBoxes;
	std::array<OctreeNode*, 8> children;
	int nodeID; // just the position in nodeList
	int vIndex;

	const bool isEmpty() const { return count == 0; }
	const bool isLeaf() const { return count <= CAPACITY; }
	std::array<Box, 1 << 3> makeSubBoxes(const std::array<float, 3>& center, const Box& boundary);
public:
	OctreeNode(const std::array<float, 3>& center, const Box& boundary, int nodeID, int vIndex, std::vector<OctreeNode*>& nodeList);

	friend class Octree;
};

class Octree {
public:
	Octree(float max) : boundary(-max, -max, -max, max, max, max), root(nullptr) {}
	~Octree();
	void init();

	void draw(const glm::mat4& projMat, const glm::mat4& viewMat);
	bool insert(SolidBody* object, bool isSafe = false);
	bool update(SolidBody* object); // assumes object is in the octree
	void remove(SolidBody* object); // assumes object is in the octree
	bool intersects(SolidBody* object);

	void dump();
private:
	const Box boundary;
	OctreeNode* root;
	std::unordered_set<SolidBody*> objects;

	const glm::vec3 lineColor{ 0.7f, 0.7f, 0.7f };
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	GLuint vertexArrayID, vertexBufferID, elementBufferID;
	LineShader shader;
	void bind();
	void generateBoundary();

	void insertVertex(int index, const std::array<float, 3>& vertex);
	void addVertex(const std::array<float, 3>& vertex);
	void addIndex(int index);

	bool isDirty{ false };
	void updateBuffer();

	int newVIndex{0};
	std::vector<int> deletedVIndex;
	std::vector<OctreeNode*> nodeList; // index buffer is configured in the order in node list
	OctreeNode* makeNode(const std::array<float, 3>& center, const Box& boundary);
	void insert(OctreeNode* node, SolidBody* object);
	bool remove(OctreeNode* node, SolidBody* object);
	std::unordered_set<SolidBody*> clean(OctreeNode* node);
	bool intersects(OctreeNode* node, SolidBody* object);
	void dump(OctreeNode* node);
};