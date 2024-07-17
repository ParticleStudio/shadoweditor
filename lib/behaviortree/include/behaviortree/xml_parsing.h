#ifndef BEHAVIORTREE_XML_PARSING_H
#define BEHAVIORTREE_XML_PARSING_H

#include <filesystem>
#include <unordered_map>

#include "behaviortree/parser.h"

namespace behaviortree {
/**
 * @brief The XMLParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class XMLParser: public Parser {
 public:
    XMLParser(const BehaviorTreeFactory &refFactory);

    ~XMLParser() override;

    XMLParser(const XMLParser &refOther) = delete;
    XMLParser &operator=(const XMLParser &refOther) = delete;

    XMLParser(XMLParser &&refOther) noexcept;
    XMLParser &operator=(XMLParser &&refOther) noexcept;

    void LoadFromFile(
            const std::filesystem::path &refFileName, bool addIncludes = true
    ) override;

    void LoadFromText(const std::string &refXmlText, bool addIncludes = true)
            override;

    [[nodiscard]] std::vector<std::string> RegisteredBehaviorTrees(
    ) const override;

    [[nodiscard]] Tree InstantiateTree(
            const Blackboard::Ptr &refRootBlackboard,
            std::string maintreeToExecute = {}
    ) override;

    void ClearInternalState() override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_P;
};

void VerifyXML(
        const std::string &refXmlText,
        const std::unordered_map<std::string, NodeType> &refRegisteredNodes
);

/**
 * @brief WriteTreeNodesModelXML generates an XMl that contains the GetManifest in the
 * <TreeNodesModel>
 *
 * @param refFactory          the factory with the registered types
 * @param includeBuiltin  if true, include the builtin Nodes
 *
 * @return  string containing the XML.
 */
[[nodiscard]] std::string WriteTreeNodesModelXML(
        const BehaviorTreeFactory &refFactory, bool includeBuiltin = false
);

/**
 * @brief WriteTreeXSD generates an XSD for the nodes defined in the factory
 *
 * @param rFactory          the factory with the registered types
 *
 * @return  string containing the XML.
 */
[[nodiscard]] std::string WriteTreeXSD(const BehaviorTreeFactory &rFactory);

/**
 * @brief WriteTreeToXML create a string that contains the XML that corresponds to a given tree.
 * When using this function with a logger, you should probably set both add_metadata and
 * add_builtin_models to true.
 *
 * @param refTree               the input tree
 * @param addMetadata       if true, the attributes "_uid" and "_fullPath" will be added to the nodes
 * @param addBuiltinModels if true, include the builtin Nodes into the <TreeNodesModel>
 *
 * @return string containing the XML.
 */
[[nodiscard]] std::string WriteTreeToXML(
        const Tree &refTree, bool addMetadata, bool addBuiltinModels
);

}// namespace behaviortree

#endif// BEHAVIORTREE_XML_PARSING_H
