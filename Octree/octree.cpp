#include "octree.h"

#include <iostream>

bool Octree::intersects(const Sphere* sphere, const Box& box) {
    // each dimension is divided by 3: left-outside, inside, right-outside
    // so the box subdivides the whole space by 27 pieces
    // sign = -1, 0, 1
    // sign = -1 -> coordinate to compare is mins
    // sign = 0 -> coordinate to compare is itself
    // sign = 1 -> maxs
    // compare the distance to radius

    std::array<float, 3> closestPoint;
    for (int i = 0; i < 3; i++) {
        if (sphere->center()[i] < box.mins[i])
            closestPoint[i] = box.mins[i];
        else if (sphere->center()[i] > box.maxs[i])
            closestPoint[i] = box.maxs[i];
        else
            closestPoint[i] = sphere->center()[i];
    }
    constexpr float EPS = 0.001;
    glm::vec3 diff;
    for (int i = 0; i < 3; i++)
        diff[i] = closestPoint[i] - sphere->center()[i];
    float dist = glm::length(diff);
    return dist + EPS < sphere->radius();
}

OctreeNode::OctreeNode(const std::array<float, 3>& center, const Box& boundary, int nodeID, int vIndex, std::vector<OctreeNode*>& nodeList)
    : center(center), boundary(boundary), nodeID(nodeID), vIndex(vIndex), subBoxes(makeSubBoxes(center, boundary)) {
    nodeList.push_back(this);
}

std::array<Box, 1 << 3> OctreeNode::makeSubBoxes(const std::array<float, 3>& center, const Box& boundary) {
    std::array<Box, 1 << 3> subBoxes;
    for (int mask = 0; mask < 1 << 3; mask++) {
        for (int pos = 0; pos < 3; pos++) {
            if (mask & (1 << pos)) {
                subBoxes[mask].mins[pos] = boundary.mins[pos];
                subBoxes[mask].maxs[pos] = center[pos];
            }
            else {
                subBoxes[mask].mins[pos] = center[pos];
                subBoxes[mask].maxs[pos] = boundary.maxs[pos];
            }
        }
    }
    return subBoxes;
}

std::array<float, 3> Box::getCenter() const {
    std::array<float, 3> ret;
    for (int i = 0; i < 3; i++)
        ret[i] = (mins[i] + maxs[i]) / 2;
    return ret;
}

OctreeNode* Octree::makeNode(const std::array<float, 3>& center, const Box& boundary) {
    const auto& mins = boundary.mins;
    const auto& maxs = boundary.maxs;
    bool isModifying;
    int vIndex;
    if (deletedVIndex.empty()) {
        isModifying = false;
        vIndex = newVIndex;
        newVIndex += 18 * 3;
    }
    else {
        isModifying = true;
        vIndex = deletedVIndex.back();
        deletedVIndex.pop_back();
    }
    int nodeID = nodeList.size();

    // when appending the vertices to the end
    // vIndex = 8 * 3 + nodeID * 18 * 3; // 6 + 4*3 = 18 new vertices
    int offset = 0;
    auto updateVertexBuffer = [&](const std::array<float, 3>& vertex) {
        if (isModifying) {
            insertVertex(vIndex + offset, vertex);
            offset += 3;
        }
        else
            addVertex(vertex);
    };

    // center lines
    for (int i = 0; i < 3; i++) {
        std::array<float, 3> vertexMin, vertexMax;
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                vertexMin[j] = mins[j];
                vertexMax[j] = maxs[j];
            }
            else {
                vertexMin[j] = center[j];
                vertexMax[j] = center[j];
            }
        }
        updateVertexBuffer(vertexMin);
        updateVertexBuffer(vertexMax);
    }

    // boundary lines
    std::array<int, 4> masks{ 0, 1, 3, 2 };
    for (int i = 0; i < 3; i++) {
        std::array<float, 3> vertex;
        for(int mask: masks){
            int pos = 0;
            for (int j = 0; j < 3; j++) {
                if(i == j){
                    vertex[j] = center[i];
                }
                else {
                    if (mask & (1 << pos++))
                        vertex[j] = mins[j];
                    else
                        vertex[j] = maxs[j];
                }
            }
            updateVertexBuffer(vertex);
        }
    }

    // here, we care about # of vertices, not # of vertice coordinates
    vIndex /= 3;
    for (int i = 0; i < 6; i+=2) {
        addIndex(vIndex + i);
        addIndex(vIndex + i + 1);
    }
    for (int i = 6; i < 18; i += 4) {
        for (int j = i; j < i + 3; j++) {
            addIndex(vIndex + j);
            addIndex(vIndex + j + 1);
        }
        addIndex(vIndex + i + 3);
        addIndex(vIndex + i);
    }

    return new OctreeNode(center, boundary, nodeID, vIndex, nodeList);
}

int dbgcnt = 0;
// assumption: node's boundary intersects with sphere
void Octree::insert(OctreeNode* node, Sphere* sphere) {
    dbgcnt++;
    // go down and make node if necessary
    auto explore = [&](Sphere* sphere) {
        for (int i = 0; i < 1 << 3; i++) {
            auto& box = node->subBoxes[i];
            if (intersects(sphere, box)) {
                if (node->children[i] == nullptr)
                    node->children[i] = makeNode(box.getCenter(), box);
                insert(node->children[i], sphere);
            }
        }
    };

    if (node->isLeaf()) {
        node->count++;
        node->spheres.insert(sphere);
        if (node->count <= node->CAPACITY)
            return;

        // node->count > node->CAPACITY, have to push down all spheres
        for (auto sphere : node->spheres)
            explore(sphere);
        node->spheres.clear();
    }
    else {
        node->count++;
        explore(sphere);
    }
}

bool Octree::insert(Sphere* sphere){
    dbgcnt = 0;
    if (spheres.count(sphere)) {
        std::cerr << "the sphere had already been added" << std::endl;
        return false;
    }
    if (!isInBoundary(sphere))
        return false;
    if (intersects(sphere))
        return false;

    if (root == nullptr)
        root = makeNode({ 0.0f, 0.0f, 0.0f }, boundary);
    insert(root, sphere);

    spheres.insert(sphere);
    isDirty = true;

    return true;
}

bool Octree::isInBoundary(Sphere* sphere, const float MARGIN) {
    for (int i = 0; i < 3; i++) {
        if (sphere->center()[i] + sphere->radius() > boundary.maxs[i] - MARGIN)
            return false;
        if(sphere->center()[i] - sphere->radius() < boundary.mins[i] - MARGIN)
            return false;
    }
    return true;
}

bool Octree::intersects(OctreeNode* node, Sphere* sphere) {
    dbgcnt++;
    if (!intersects(sphere, node->boundary))
        return false;
    if (node->isLeaf()) {
        for (auto sphere2 : node->spheres) {
            if (sphere2->intersects(sphere))
                return true;
        }
        return false;
    }
    else {
        for (int i = 0; i < 1 << 3; i++) {
            if (node->children[i] == nullptr)
                continue;
            if (intersects(node->children[i], sphere))
                return true;
        }
        return false;
    }
}

bool Octree::intersects(Sphere* sphere) {
    dbgcnt = 0;
    if (root == nullptr)
        return false;
    return intersects(root, sphere);
}

// remove all nodes under node
// overwrite the vbo and ibo
// -- move the last vertices and indices
void Octree::clean(OctreeNode* node) {
    dbgcnt++;
    int iIndex = node->nodeID * 15 * 2 + 12 * 2;
    deletedVIndex.push_back(indices[iIndex] * 3);
    if (nodeList.back() == node) {
        assert(iIndex + 15 * 2 == indices.size());
        nodeList.pop_back();
        for (int i = 0; i < 15 * 2; i++)
            indices.pop_back();
    }
    else {
        auto node2 = nodeList.back();
        nodeList.pop_back();
        nodeList[node->nodeID] = node2;
        node2->nodeID = node->nodeID;
        // 3 + 4*3 = 15 lines per center
        for (int i = 15 * 2 - 1; i >= 0; i--) {
            indices[iIndex + i] = indices.back();
            indices.pop_back();
        }
    }
    for (int i = 0; i < 1 << 3; i++) {
        if (node->children[i] == nullptr)
            continue;
        clean(node->children[i]);
    }
    delete node;
}

// if true, the node had been deleted
bool Octree::remove(OctreeNode* node, Sphere* sphere) {
    dbgcnt++;
    if (!intersects(sphere, node->boundary))
        return false;

    // now assume that sphere had been added to node
    if (node->isLeaf()) {
        // still leaf
        node->count--;
        node->spheres.erase(sphere);
        if (node->count == 0) {
            clean(node);
            return true;
        }
    }
    else {
        // non-leaf to leaf
        if (node->count == node->CAPACITY) {
            for (int i = 0; i < 1 << 3; i++) {
                if (node->children[i] == nullptr)
                    continue;
                node->spheres.insert(node->children[i]->spheres.begin(), node->children[i]->spheres.end());
                clean(node->children[i]);
                node->children[i] = nullptr;
            }
        }
        else { // node->count > node->CAPACITY
            for (int i = 0; i < 1 << 3; i++) {
                if (node->children[i] == nullptr)
                    continue;
                if (remove(node->children[i], sphere))
                    node->children[i] = nullptr;
            }
        }
    }
    return false;
}

void Octree::remove(Sphere* sphere) {
    dbgcnt = 0;
    if (root == nullptr || !spheres.count(sphere)) {
        std::cerr << "can't find the sphere to remove" << std::endl;
        return;
    }
    if (remove(root, sphere))
        root = nullptr;

    spheres.erase(sphere);
    isDirty = true;
}

void Octree::init(){
	shader.init();

    generateBoundary();

    bind();
}

void Octree::generateBoundary() {
    for (int mask = 0; mask < 1 << 3; mask++) {
        std::array<float, 3> vertex;
        for (int pos = 0; pos < 3; pos++) {
            if (mask & (1 << pos))
                vertex[pos] = boundary.mins[pos];
            else
                vertex[pos] = boundary.maxs[pos];
        }
        addVertex(vertex);
    }

    for (int mask = 0; mask < 1 << 3; mask++) {
        for (int pos = 0; pos < 3; pos++) {
            if (mask & (1 << pos))
                continue;
            int mask2 = mask | (1 << pos);
            indices.push_back(mask);
            indices.push_back(mask2);
        }
    }

    newVIndex = 8 * 3;
}

void Octree::draw(const glm::mat4& projMat, const glm::mat4& viewMat) {
    if (isDirty) {
        updateBuffer();
        isDirty = false;
    }

    glUseProgram(shader.programID);

    // model matrix is simply identity
    glm::mat4 vpMat = projMat * viewMat;
    glUniformMatrix4fv(shader.vpMatID, 1, GL_FALSE, &vpMat[0][0]);
    glUniform3fv(shader.lineColorID, 1, &lineColor[0]);

    glBindVertexArray(vertexArrayID);

    glDrawElements(
        GL_LINES,          // mode
        indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    glUseProgram(0);
    glBindVertexArray(0);
}

void Octree::updateBuffer(){
    glBindVertexArray(vertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Octree::bind(){
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute #
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    glGenBuffers(1, &elementBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
}

Octree::~Octree() {
    if (vertexArrayID != 0)
        glDeleteVertexArrays(1, &vertexArrayID);
    if (vertexBufferID != 0)
        glDeleteBuffers(1, &vertexBufferID);
    if (elementBufferID != 0)
        glDeleteBuffers(1, &elementBufferID);
}

void Octree::addVertex(const std::array<float, 3>& vertex) {
    for (int i = 0; i < 3; i++)
        vertices.push_back(vertex[i]);
}
void Octree::insertVertex(int index, const std::array<float, 3>& vertex) {
    for (int i = 0; i < 3; i++)
        vertices[index+i] = vertex[i];
}
void Octree::addIndex(int index) {
    indices.push_back(index);
}