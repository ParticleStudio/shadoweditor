/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree/behaviortree.h"

#include <cstring>

namespace behaviortree {
void ApplyRecursiveVisitor(const TreeNode* ptrNode,
                           const std::function<void(const TreeNode*)>& refVisitor) {
    if(!ptrNode) {
        throw LogicError("One of the Children of a DecoratorNode or ControlNode is nullptr");
    }

    refVisitor(ptrNode);

    if(auto control = dynamic_cast<const behaviortree::ControlNode*>(ptrNode)) {
        for(const auto& refChild: control->Children()) {
            ApplyRecursiveVisitor(static_cast<const TreeNode*>(refChild), refVisitor);
        }
    } else if(auto ptrDecorator = dynamic_cast<const behaviortree::DecoratorNode*>(ptrNode)) {
        ApplyRecursiveVisitor(ptrDecorator->Child(), refVisitor);
    }
}

void ApplyRecursiveVisitor(TreeNode* ptrNode, const std::function<void(TreeNode*)>& refVisitor) {
    if(ptrNode == nullptr) {
        throw LogicError("One of the Children of a DecoratorNode or ControlNode is nullptr");
    }

    refVisitor(ptrNode);

    if(auto ptrControl = dynamic_cast<behaviortree::ControlNode*>(ptrNode)) {
        for(const auto& refChild: ptrControl->Children()) {
            ApplyRecursiveVisitor(refChild, refVisitor);
        }
    } else if(auto ptrDecorator = dynamic_cast<behaviortree::DecoratorNode*>(ptrNode)) {
        if(ptrDecorator->Child()) {
            ApplyRecursiveVisitor(ptrDecorator->Child(), refVisitor);
        }
    }
}

void PrintTreeRecursively(const TreeNode* ptrRootNode, std::ostream& refStream) {
    std::function<void(unsigned, const behaviortree::TreeNode*)> recursivePrint;

    recursivePrint = [&recursivePrint, &refStream](unsigned indent, const behaviortree::TreeNode* ptrNode) {
        for(uint32_t i = 0; i < indent; i++) {
            refStream << "   ";
        }
        if(ptrNode == nullptr) {
            refStream << "!nullptr!" << std::endl;
            return;
        }
        refStream << ptrNode->Name() << std::endl;
        indent++;

        if(auto ptrControl = dynamic_cast<const behaviortree::ControlNode*>(ptrNode)) {
            for(const auto& refChild: ptrControl->Children()) {
                recursivePrint(indent, refChild);
            }
        } else if(auto ptrDecorator = dynamic_cast<const behaviortree::DecoratorNode*>(ptrNode)) {
            recursivePrint(indent, ptrDecorator->Child());
        }
    };

    refStream << "----------------" << std::endl;
    recursivePrint(0, ptrRootNode);
    refStream << "----------------" << std::endl;
}

void BuildSerializedStatusSnapshot(TreeNode* ptrRootNode,
                                   SerializedTreeStatus& refSerializedBuffer) {
    refSerializedBuffer.clear();

    auto visitor = [&refSerializedBuffer](const TreeNode* ptrNode) {
        refSerializedBuffer.push_back(
                std::make_pair(ptrNode->GetUID(), static_cast<uint8_t>(ptrNode->NodeStatus())));
    };

    ApplyRecursiveVisitor(ptrRootNode, visitor);
}

int LibraryVersionNumber() {
    static int number = -1;
    if(number == -1) {
        auto const parts = splitString(BEHAVIORTREE_LIBRARY_VERSION, '.');
        number = std::stoi(std::string(parts[0])) * 10000 +
                 std::stoi(std::string(parts[1])) * 100 + std::stoi(std::string(parts[2]));
    }
    return number;
}

const char* LibraryVersionString() {
    return BEHAVIORTREE_LIBRARY_VERSION;
}

}// namespace behaviortree
