#ifndef BEHAVIORTREE_JSON_PARSING_H
#define BEHAVIORTREE_JSON_PARSING_H

#include <filesystem>
#include <unordered_map>

#include "behaviortree/parser.h"

namespace behaviortree {
/**
 * @brief The JsonParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class JsonParser: public Parser {
 public:
    JsonParser(const BehaviorTreeFactory &refFactory);

    ~JsonParser() override;

    JsonParser(const JsonParser &refOther) = delete;
    JsonParser &operator=(const JsonParser &refOther) = delete;

    JsonParser(JsonParser &&refOther) noexcept;
    JsonParser &operator=(JsonParser &&refOther) noexcept;

    void LoadFromFile(const std::filesystem::path &refFileName, bool addIncludes = true) override;

    void LoadFromText(const std::string &refText, bool addIncludes = true) override;

    [[nodiscard]] std::vector<std::string> RegisteredBehaviorTrees() const override;

    [[nodiscard]] Tree InstantiateTree(const Blackboard::Ptr &refRootBlackboard, std::string maintreeToExecute = {}) override;

    void ClearInternalState() override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_P;
};

void VerifyJson(const std::string &refJsonText, const std::unordered_map<std::string, NodeType> &refRegisteredNodes);

/**
 * @brief WriteTreeNodesModelJson generates an Json that contains the Manifests in the
 * <TreeNodesModel>
 *
 * @param refFactory          the factory with the registered types
 * @param includeBuiltin  if true, include the builtin Nodes
 *
 * @return  string containing the Json.
 */
[[nodiscard]] std::string WriteTreeNodesModelJson(const BehaviorTreeFactory &refFactory, bool includeBuiltin = false);

/**
 * @brief WriteTreeXSD generates an XSD for the nodes defined in the factory
 *
 * @param refFactory          the factory with the registered types
 *
 * @return  string containing the Json.
 */
[[nodiscard]] std::string WriteTreeXSD(const BehaviorTreeFactory &refFactory);

/**
 * @brief WriteTreeToJson create a string that contains the Json that corresponds to a given tree.
 * When using this function with a logger, you should probably set both add_metadata and
 * add_builtin_models to true.
 *
 * @param refTree               the input tree
 * @param addMetadata       if true, the attributes "_uid" and "_fullPath" will be added to the nodes
 * @param addBuiltinModels if true, include the builtin Nodes into the <TreeNodesModel>
 *
 * @return string containing the Json.
 */
[[nodiscard]] std::string WriteTreeToJson(const Tree &refTree, bool addMetadata, bool addBuiltinModels);

}// namespace behaviortree

#endif// BEHAVIORTREE_JSON_PARSING_H
