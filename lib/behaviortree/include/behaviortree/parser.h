#ifndef BEHAVIORTREE_PARSER_H
#define BEHAVIORTREE_PARSER_H

#include <filesystem>

#include "behaviortree/blackboard.h"
#include "behaviortree/factory.h"

namespace behaviortree {
/**
 * @brief The BehaviorTreeParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class Parser {
 public:
    Parser() = default;

    virtual ~Parser() = default;

    Parser(const Parser &refOther) = delete;
    Parser &operator=(const Parser &refOther) = delete;

    Parser(Parser &&refOther) = default;
    Parser &operator=(Parser &&refOther) = default;

    virtual void LoadFromFile(
            const std::filesystem::path &refFileName, bool addIncludes = true
    ) = 0;

    virtual void LoadFromText(
            const std::string &refXmlText, bool addIncludes = true
    ) = 0;

    virtual std::vector<std::string> RegisteredBehaviorTrees() const = 0;

    virtual Tree InstantiateTree(
            const Blackboard::Ptr &refRootBlackboard,
            std::string refTreeName = {}
    ) = 0;

    virtual void ClearInternalState() {};
};
}// namespace behaviortree

#endif// BEHAVIORTREE_PARSER_H
