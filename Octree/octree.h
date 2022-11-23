#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "sphere.h"

#include <array>
#include <vector>
#include <unordered_set>

struct Box {
	std::array<float, 3> mins;
	std::array<float, 3> maxs;
	Box() {}
	Box(float x1, float y1, float z1, float x2, float y2, float z2)
		: mins{ x1, y1, z1 }, maxs{ x2, y2, z2 }{}
	Box(const std::array<float, 3>& mins, const std::array<float, 3>& maxs)
		: mins(mins), maxs(maxs) {}
	Box(const Box& box)
		: mins(box.mins), maxs(box.maxs) {}
	std::array<float, 3> getCenter() const;
};

class OctreeNode {
private:
	static constexpr int CAPACITY = 30;
	std::unordered_set<Sphere*> spheres;
	int count{0};
	const std::array<float, 3> center;
	Box boundary;
	const std::array<Box, 1<<3> subBoxes;
	std::array<OctreeNode*, 8> children;
	int nodeID; // just the position in nodeList
	int vIndex;

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
	bool insert(Sphere* sphere);
	void remove(Sphere* sphere); // assumes sphere is in the octree
	bool intersects(Sphere* sphere);
	bool isInBoundary(Sphere* sphere, const float MARGIN = 0.01);
private:
	const Box boundary;
	OctreeNode* root;
	std::unordered_set<Sphere*> spheres;

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
	void insert(OctreeNode* node, Sphere* sphere);
	bool remove(OctreeNode* node, Sphere* sphere);
	void clean(OctreeNode* node);
	bool intersects(OctreeNode* node, Sphere* sphere);
	bool intersects(const Sphere* sphere, const Box& box);
};