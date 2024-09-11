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

    Parser(const Parser &rOther) = delete;
    Parser &operator=(const Parser &rOther) = delete;

    Parser(Parser &&rOther) = default;
    Parser &operator=(Parser &&rOther) = default;

    virtual void LoadFromFile(const std::filesystem::path &rFileName, bool addIncludes = true) = 0;

    virtual void LoadFromText(const std::string &rText, bool addIncludes = true) = 0;

    virtual std::vector<std::string> RegisteredBehaviorTrees() const = 0;

    virtual Tree InstantiateTree(const Blackboard::Ptr &rRootBlackboard, std::string rTreeName = {}) = 0;

    virtual void ClearInternalState() {};
};
}// namespace behaviortree

#endif// BEHAVIORTREE_PARSER_H
