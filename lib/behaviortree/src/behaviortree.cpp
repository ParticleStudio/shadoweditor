#include "behaviortree/behaviortree.h"

#include <cstring>

#include "behaviortree.config.h"

namespace behaviortree {
void ApplyRecursiveVisitor(const TreeNode *ptrNode, const std::function<void(const TreeNode *)> &refVisitor) {
    if(ptrNode == nullptr) {
        throw LogicError("One of the Children of a DecoratorNode or ControlNode is nullptr");
    }

    refVisitor(ptrNode);

    if(auto control = dynamic_cast<const behaviortree::ControlNode *>(ptrNode)) {
        for(const auto &refChild: control->Children()) {
            ApplyRecursiveVisitor(static_cast<const TreeNode *>(refChild), refVisitor);
        }
    } else if(auto ptrDecorator = dynamic_cast<const behaviortree::DecoratorNode *>(ptrNode)) {
        ApplyRecursiveVisitor(ptrDecorator->GetChild(), refVisitor);
    }
}

void ApplyRecursiveVisitor(TreeNode *ptrNode, const std::function<void(TreeNode *)> &refVisitor) {
    if(ptrNode == nullptr) {
        throw LogicError("One of the Children of a DecoratorNode or ControlNode is nullptr");
    }

    refVisitor(ptrNode);

    if(auto ptrControl = dynamic_cast<behaviortree::ControlNode *>(ptrNode)) {
        for(const auto &refChild: ptrControl->Children()) {
            ApplyRecursiveVisitor(refChild, refVisitor);
        }
    } else if(auto ptrDecorator = dynamic_cast<behaviortree::DecoratorNode *>(ptrNode)) {
        if(ptrDecorator->GetChild()) {
            ApplyRecursiveVisitor(ptrDecorator->GetChild(), refVisitor);
        }
    }
}

void PrintTreeRecursively(const TreeNode *ptrRootNode, std::ostream &refStream) {
    std::function<void(unsigned, const behaviortree::TreeNode *)> recursivePrint;

    recursivePrint = [&recursivePrint, &refStream](unsigned indent, const behaviortree::TreeNode *ptrNode) {
        for(uint32_t i = 0; i < indent; i++) {
            refStream << "   ";
        }
        if(ptrNode == nullptr) {
            refStream << "!nullptr!" << std::endl;
            return;
        }
        refStream << ptrNode->GetNodeName() << std::endl;
        indent++;

        if(auto ptrControl = dynamic_cast<const behaviortree::ControlNode *>(ptrNode)) {
            for(const auto &refChild: ptrControl->Children()) {
                recursivePrint(indent, refChild);
            }
        } else if(auto ptrDecorator = dynamic_cast<const behaviortree::DecoratorNode *>(ptrNode)) {
            recursivePrint(indent, ptrDecorator->GetChild());
        }
    };

    refStream << "----------------" << std::endl;
    recursivePrint(0, ptrRootNode);
    refStream << "----------------" << std::endl;
}

void BuildSerializedStatusSnapshot(TreeNode *ptrRootNode, SerializedTreeStatus &refSerializedBuffer) {
    refSerializedBuffer.clear();

    auto visitor = [&refSerializedBuffer](const TreeNode *ptrNode) {
        refSerializedBuffer.push_back(std::make_pair(ptrNode->GetUID(), static_cast<uint8_t>(ptrNode->GetNodeStatus())));
    };

    ApplyRecursiveVisitor(ptrRootNode, visitor);
}

int GetLibraryVersionNumber() {
    static int number = -1;
    if(number == -1) {
        auto const parts = SplitString(BEHAVIORTREE_VERSION, '.');
        number = std::stoi(std::string(parts[0])) * 10000 +
                 std::stoi(std::string(parts[1])) * 100 +
                 std::stoi(std::string(parts[2]));
    }
    return number;
}

const char *GetLibraryVersionString() {
    return BEHAVIORTREE_VERSION;
}

}// namespace behaviortree
