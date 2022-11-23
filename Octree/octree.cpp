#include "octree.h"

#include <iostream>

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
// assumption: node's boundary intersects with object
void Octree::insert(OctreeNode* node, SolidBody* object) {
    dbgcnt++;
    // go down and make node if necessary
    auto explore = [&](SolidBody* object) {
        for (int i = 0; i < 1 << 3; i++) {
            auto& box = node->subBoxes[i];
            if (object->intersects(box)) {
                if (node->children[i] == nullptr)
                    node->children[i] = makeNode(box.getCenter(), box);
                insert(node->children[i], object);
            }
        }
    };

    if (node->isLeaf()) {
        node->count++;
        node->objects.insert(object);
        if (node->count <= node->CAPACITY)
            return;

        // node->count > node->CAPACITY, have to push down all objects
        for (auto object : node->objects)
            explore(object);
        node->objects.clear();
    }
    else {
        node->count++;
        explore(object);
    }
}

bool Octree::insert(SolidBody* object, bool isSafe){
    dbgcnt = 0;
    if (objects.count(object)) {
        std::cerr << "the object had already been added" << std::endl;
        return false;
    }
    if (!isSafe) {
        if (!object->containedInBoundary(boundary))
            return false;
        if (intersects(object))
            return false;
    }

    if (root == nullptr)
        root = makeNode({ 0.0f, 0.0f, 0.0f }, boundary);
    insert(root, object);

    objects.insert(object);
    isDirty = true;

    return true;
}

bool Octree::intersects(OctreeNode* node, SolidBody* object) {
    dbgcnt++;
    if (!object->intersects(node->boundary))
        return false;
    if (node->isLeaf()) {
        for (auto object2 : node->objects) {
            if(object2 != object && object2->intersects(object))
                return true;
        }
        return false;
    }
    else {
        for (int i = 0; i < 1 << 3; i++) {
            if (node->children[i] == nullptr)
                continue;
            if (intersects(node->children[i], object))
                return true;
        }
        return false;
    }
}

bool Octree::intersects(SolidBody* object) {
    dbgcnt = 0;
    if (root == nullptr)
        return false;
    return intersects(root, object);
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
bool Octree::remove(OctreeNode* node, SolidBody* object) {
    dbgcnt++;
    if (!object->intersects(node->boundary))
        return false;

    // now assume that object had been added to node
    if (node->isLeaf()) {
        // still leaf
        node->count--;
        node->objects.erase(object);
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
                node->objects.insert(node->children[i]->objects.begin(), node->children[i]->objects.end());
                clean(node->children[i]);
                node->children[i] = nullptr;
            }
        }
        else { // node->count > node->CAPACITY
            for (int i = 0; i < 1 << 3; i++) {
                if (node->children[i] == nullptr)
                    continue;
                if (remove(node->children[i], object))
                    node->children[i] = nullptr;
            }
        }
    }
    return false;
}

void Octree::remove(SolidBody* object) {
    dbgcnt = 0;
    if (root == nullptr || !objects.count(object)) {
        std::cerr << "can't find the object to remove" << std::endl;
        return;
    }
    if (remove(root, object))
        root = nullptr;

    objects.erase(object);
    isDirty = true;
}

bool Octree::update(SolidBody* object)
{
    if (!object->containedInBoundary(boundary))
        return false;
    if (intersects(object))
        return false;

    remove(object);
    bool res = insert(object, true);
    assert(res);
    return res;
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