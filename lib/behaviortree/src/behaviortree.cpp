#include "behaviortree/behaviortree.h"

#include <cstring>

#include "behaviortree.config.h"

namespace behaviortree {
void ApplyRecursiveVisitor(const TreeNode *pTreeNode, const std::function<void(const TreeNode *)> &rVisitor) {
    if(pTreeNode == nullptr) {
        throw util::LogicError("One of the GetChildrenNode of a DecoratorNode or ControlNode is nullptr");
    }

    rVisitor(pTreeNode);

    if(auto control = dynamic_cast<const behaviortree::ControlNode *>(pTreeNode)) {
        for(const auto &rChildNode: control->GetChildrenNode()) {
            ApplyRecursiveVisitor(static_cast<const TreeNode *>(rChildNode), rVisitor);
        }
    } else if(auto *pDecorator = dynamic_cast<const behaviortree::DecoratorNode *>(pTreeNode)) {
        ApplyRecursiveVisitor(pDecorator->GetChildNode(), rVisitor);
    }
}

void ApplyRecursiveVisitor(TreeNode *pTreeNode, const std::function<void(TreeNode *)> &rVisitor) {
    if(pTreeNode == nullptr) {
        throw util::LogicError("One of the GetChildrenNode of a DecoratorNode or ControlNode is nullptr");
    }

    rVisitor(pTreeNode);

    if(auto *pControl = dynamic_cast<behaviortree::ControlNode *>(pTreeNode)) {
        for(const auto &rChildNode: pControl->GetChildrenNode()) {
            ApplyRecursiveVisitor(rChildNode, rVisitor);
        }
    } else if(auto *pDecorator = dynamic_cast<behaviortree::DecoratorNode *>(pTreeNode)) {
        if(pDecorator->GetChildNode() != nullptr) {
            ApplyRecursiveVisitor(pDecorator->GetChildNode(), rVisitor);
        }
    }
}

void PrintTreeRecursively(const TreeNode *pRootNode, std::ostream &rStream) {
    std::function<void(unsigned, const behaviortree::TreeNode *)> recursivePrint;

    recursivePrint = [&recursivePrint, &rStream](unsigned indent, const behaviortree::TreeNode *pNode) {
        for(uint32_t i = 0; i < indent; i++) {
            rStream << "   ";
        }
        if(pNode == nullptr) {
            rStream << "!nullptr!" << std::endl;
            return;
        }
        rStream << pNode->GetNodeName() << std::endl;
        indent++;

        if(auto pControl = dynamic_cast<const behaviortree::ControlNode *>(pNode)) {
            for(const auto &rChild: pControl->GetChildrenNode()) {
                recursivePrint(indent, rChild);
            }
        } else if(auto pDecorator = dynamic_cast<const behaviortree::DecoratorNode *>(pNode)) {
            recursivePrint(indent, pDecorator->GetChildNode());
        }
    };

    rStream << "----------------" << std::endl;
    recursivePrint(0, pRootNode);
    rStream << "----------------" << std::endl;
}

void BuildSerializedStatusSnapshot(TreeNode *pRootNode, SerializedTreeStatus &rSerializedBuffer) {
    rSerializedBuffer.clear();

    auto visitor = [&rSerializedBuffer](const TreeNode *pNode) {
        rSerializedBuffer.emplace_back(pNode->GetUid(), static_cast<uint8_t>(pNode->GetNodeStatus()));
    };

    ApplyRecursiveVisitor(pRootNode, visitor);
}

int GetLibraryVersionNumber() {
    static int number = -1;
    if(number == -1) {
        auto const partVec = SplitString(BEHAVIORTREE_VERSION, '.');
        number = std::stoi(std::string(partVec[0])) * 10000 +
                 std::stoi(std::string(partVec[1])) * 100 +
                 std::stoi(std::string(partVec[2]));
    }
    return number;
}

const char *GetLibraryVersionString() {
    return BEHAVIORTREE_VERSION;
}

}// namespace behaviortree
