import <cstdio>;
import <cstring>;
import <filesystem>;
import <fstream>;
import <functional>;
import <iostream>;
import <list>;
import <string>;
import <typeindex>;

#if defined(__linux) || defined(__linux__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wattributes"
#endif

import <filesystem>;
import <map>;

import common.exception;

#include "behaviortree/json_parsing.h"

#include "behaviortree/blackboard.h"
#include "behaviortree/tree_node.h"
#include "behaviortree/util/demangle_util.h"
#include "common/string.hpp"
#include "nlohmann/json.hpp"
#include "tinyxml2.h"

namespace behaviortree {
using namespace tinyxml2;

auto StrEqual = [](const char *pStr1, const char *pStr2) -> bool {
    return strcmp(pStr1, pStr2) == 0;
};

struct SubtreeModel {
    std::unordered_map<std::string, behaviortree::PortInfo> portMap;
};

struct JsonParser::PImpl {
    TreeNode::Ptr CreateNodeFromJson(const XMLElement *pElement, const Blackboard::Ptr &rBlackboard, const TreeNode::Ptr &rNodeParent, const std::string &rPrefixPath, Tree &rOutputTree);

    void RecursivelyCreateSubtree(const std::string &pParentNode, const std::string &pSubtree, const std::string &rPrefixPath, Tree &rOutputTree, Blackboard::Ptr pBlackboard, const TreeNode::Ptr &pRootNode);

    void GetPortsRecursively(const XMLElement *pElement, std::vector<std::string> &rOutputPortVec);

    void LoadJsonImpl(nlohmann::json *pJson, bool addInclude);

    std::list<std::unique_ptr<nlohmann::json>> openedJsonList;
    std::map<std::string, const nlohmann::json *> treeRootMap;

    const BehaviorTreeFactory &rFactory;

    std::filesystem::path currentPath;
    std::map<std::string, SubtreeModel> subtreeModelMap;

    int32_t suffixCount;

    explicit PImpl(const BehaviorTreeFactory &rFact): rFactory(rFact), currentPath(std::filesystem::current_path()), suffixCount(0) {}

    void Clear() {
        suffixCount = 0;
        currentPath = std::filesystem::current_path();
        openedJsonList.clear();
        treeRootMap.clear();
    }

 private:
    void LoadSubtreeModel(const XMLElement *xml_root);
};

#if defined(__linux) || defined(__linux__)
#    pragma GCC diagnostic pop
#endif

JsonParser::JsonParser(const BehaviorTreeFactory &rFactory): m_pPImpl(new PImpl(rFactory)) {}

JsonParser::JsonParser(JsonParser &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
}

JsonParser &JsonParser::operator=(JsonParser &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
    return *this;
}

JsonParser::~JsonParser() = default;

void JsonParser::LoadFromFile(const std::filesystem::path &rFilepath, bool addInclude) {
    if(!std::filesystem::exists(rFilepath) || !std::filesystem::is_regular_file(rFilepath)) {
        throw util::RuntimeError("file is not exists: ", rFilepath.string());
    }

    m_pPImpl->openedJsonList.emplace_back(std::move(std::make_unique<nlohmann::json>()));

    nlohmann::json *pJson = m_pPImpl->openedJsonList.back().get();
    try {
        std::ifstream jsonFile(rFilepath);
        jsonFile >> *pJson;
    } catch(std::exception &ex) {
        throw util::RuntimeError("read json file fail: ", ex.what());
    }

    m_pPImpl->currentPath = std::filesystem::absolute(rFilepath.parent_path());

    m_pPImpl->LoadJsonImpl(pJson, addInclude);
}

void JsonParser::LoadFromText(const std::string &rText, bool addInclude) {
    m_pPImpl->openedJsonList.emplace_back(std::move(std::make_unique<nlohmann::json>()));

    nlohmann::json *pJson = m_pPImpl->openedJsonList.back().get();
    pJson->parse(rText);

    m_pPImpl->LoadJsonImpl(pJson, addInclude);
}

std::vector<std::string> JsonParser::RegisteredBehaviorTrees() const {
    std::vector<std::string> out;
    for(const auto &rIter: m_pPImpl->treeRootMap) {
        out.push_back(rIter.first);
    }
    return out;
}

void behaviortree::JsonParser::PImpl::LoadSubtreeModel(const XMLElement *xml_root) {
    for(auto models_node = xml_root->FirstChildElement("TreeNodesModel");
        models_node != nullptr;
        models_node = models_node->NextSiblingElement("TreeNodesModel")) {
        for(auto sub_node = models_node->FirstChildElement("Subtree");
            sub_node != nullptr;
            sub_node = sub_node->NextSiblingElement("Subtree")) {
            auto subtree_id = sub_node->Attribute("Id");
            auto &subtree_model = subtreeModelMap[subtree_id];

            std::pair<const char *, behaviortree::PortDirection> port_types[3] = {
                    {"input_port", behaviortree::PortDirection::Input},
                    {"output_port", behaviortree::PortDirection::Output},
                    {"inout_port", behaviortree::PortDirection::InOut}
            };

            for(const auto &[name, direction]: port_types) {
                for(auto port_node = sub_node->FirstChildElement(name);
                    port_node != nullptr;
                    port_node = port_node->NextSiblingElement(name)) {
                    behaviortree::PortInfo port(direction);
                    auto name = port_node->Attribute("name");
                    if(!name) {
                        throw util::RuntimeError("Missing attribute [name] in port (Subtree model)");
                    }
                    if(auto default_value = port_node->Attribute("default")) {
                        port.SetDefaultValue(default_value);
                    }
                    if(auto description = port_node->Attribute("description")) {
                        port.SetDescription(description);
                    }
                    subtree_model.portMap[name] = std::move(port);
                }
            }
        }
    }
}

void JsonParser::PImpl::LoadJsonImpl(nlohmann::json *pJson, bool addInclude) {
    if(pJson->Error()) {
        char buffer[512];
        snprintf(buffer, sizeof buffer, "Error parsing the XML: %s", pJson->ErrorStr());
        throw util::RuntimeError(buffer);
    }

    const XMLElement *xml_root = pJson->RootElement();

    auto format = xml_root->Attribute("BTCPP_format");
    if(!format) {
        std::cout << "Warnings: The first tag of the XML (<root>) should "
                     "contain the "
                     "attribute [BTCPP_format=\"4\"]\n"
                  << "Please check if your XML is compatible with version 4.x "
                     "of BT.CPP"
                  << std::endl;
    }

    // recursively include other files
    for(auto incl_node = xml_root->FirstChildElement("include");
        incl_node != nullptr;
        incl_node = incl_node->NextSiblingElement("include")) {
        if(!addInclude) {
            break;
        }

        std::filesystem::path file_path(incl_node->Attribute("path"));
        const char *ros_pkg_relative_path = incl_node->Attribute("ros_pkg");

        if(ros_pkg_relative_path) {
            if(file_path.is_absolute()) {
                std::cout << "WARNING: <include path=\"...\"> contains an "
                             "absolute path.\n"
                          << "Attribute [ros_pkg] will be ignored."
                          << std::endl;
            } else {
                std::string ros_pkg_path;
                throw util::RuntimeError(
                        "Using attribute [ros_pkg] in <include>, but this "
                        "library was "
                        "compiled without ROS support. Recompile the "
                        "BehaviorTree.CPP "
                        "using catkin"
                );
            }
        }

        if(!file_path.is_absolute()) {
            file_path = currentPath / file_path;
        }

        openedJsonList.emplace_back(new XMLDocument());
        XMLDocument *next_doc = openedJsonList.back().get();

        // change current path to the included file for handling additional relative paths
        const auto previous_path = currentPath;
        currentPath = std::filesystem::absolute(file_path.parent_path());

        next_doc->LoadFile(file_path.string().c_str());
        LoadJsonImpl(next_doc, addInclude);

        // reset current path to the previous value
        currentPath = previous_path;
    }

    // Collect the names of all nodes registered with the behavior tree factory
    std::unordered_map<std::string, behaviortree::NodeType> registeredNodeMap;
    for(const auto &it: rFactory.GetManifest()) {
        registeredNodeMap.insert({it.first, it.second.type});
    }

    XMLPrinter printer;
    pJson->Print(&printer);
    auto xml_text = std::string(printer.CStr(), size_t(printer.CStrSize()));

    // Verify the validity of the XML before adding any behavior trees to the parser's list of registered trees
    VerifyJson(xml_text, registeredNodeMap);

    LoadSubtreeModel(xml_root);

    // Register each BehaviorTree within the XML
    for(auto bt_node = xml_root->FirstChildElement("BehaviorTree"); bt_node != nullptr; bt_node = bt_node->NextSiblingElement("BehaviorTree")) {
        std::string tree_name;
        if(bt_node->Attribute("Id")) {
            tree_name = bt_node->Attribute("Id");
        } else {
            tree_name = "behaviortree_" + std::to_string(suffixCount++);
        }

        treeRootMap[tree_name] = bt_node;
    }
}

void VerifyJson(const std::string &rJsonText, const std::unordered_map<std::string, behaviortree::NodeType> &rRegisteredNodes) {
    XMLDocument doc;
    auto xml_error = doc.Parse(rJsonText.c_str(), rJsonText.size());
    if(xml_error) {
        char buffer[512];
        snprintf(buffer, sizeof buffer, "Error parsing the XML: %s", doc.ErrorName());
        throw util::RuntimeError(buffer);
    }

    //-------- Helper functions (lambdas) -----------------
    auto ThrowError = [&](int line_num, const std::string &text) {
        char buffer[512];
        snprintf(buffer, sizeof buffer, "Error at line %d: -> %s", line_num, text.c_str());
        throw util::RuntimeError(buffer);
    };

    auto ChildrenCount = [](const XMLElement *parent_node) {
        int count = 0;
        for(auto node = parent_node->FirstChildElement(); node != nullptr;
            node = node->NextSiblingElement()) {
            count++;
        }
        return count;
    };
    //-----------------------------

    const XMLElement *xml_root = doc.RootElement();

    if(!xml_root || !StrEqual(xml_root->Name(), "root")) {
        throw util::RuntimeError("The XML must have a root node called <root>");
    }
    //-------------------------------------------------
    auto models_root = xml_root->FirstChildElement("TreeNodesModel");
    auto meta_sibling = models_root ? models_root->NextSiblingElement("TreeNodesModel") : nullptr;

    if(meta_sibling) {
        ThrowError(meta_sibling->GetLineNum(), " Only a single node <TreeNodesModel> is supported");
    }
    if(models_root) {
        // not having a MetaModel is not an error. But consider that the
        // Graphical editor needs it.
        for(auto node = xml_root->FirstChildElement(); node != nullptr;
            node = node->NextSiblingElement()) {
            const std::string name = node->Name();
            if(name == "Action" || name == "Decorator" || name == "Subtree" || name == "Condition" || name == "Control") {
                const char *pId = node->Attribute("Id");
                if(!pId) {
                    ThrowError(node->GetLineNum(), "Error at line %d: -> The attribute [ID] is mandatory");
                }
            }
        }
    }
    //-------------------------------------------------

    int behavior_tree_count = 0;
    for(auto child = xml_root->FirstChildElement(); child != nullptr;
        child = child->NextSiblingElement()) {
        behavior_tree_count++;
    }

    // function to be called recursively
    std::function<void(const XMLElement *)> RecursiveStep = [&](const XMLElement *node) {
        const int children_count = ChildrenCount(node);
        const std::string name = node->Name();
        const std::string ID =
                node->Attribute("ID") ? node->Attribute("ID") : "";
        const int line_number = node->GetLineNum();

        if(name == "Decorator") {
            if(children_count != 1) {
                ThrowError(
                        line_number,
                        "The tag <Decorator> must have exactly 1 "
                        "GetChildNode"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Decorator> must have the "
                        "attribute [ID]"
                );
            }
        } else if(name == "Action") {
            if(children_count != 0) {
                ThrowError(
                        line_number,
                        "The tag <Action> must not have any "
                        "GetChildNode"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Action> must have the "
                        "attribute [ID]"
                );
            }
        } else if(name == "Condition") {
            if(children_count != 0) {
                ThrowError(
                        line_number,
                        "The tag <Condition> must not have any "
                        "GetChildNode"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Condition> must have the "
                        "attribute [ID]"
                );
            }
        } else if(name == "Control") {
            if(children_count == 0) {
                ThrowError(
                        line_number,
                        "The tag <Control> must have at least 1 "
                        "GetChildNode"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Control> must have the "
                        "attribute [ID]"
                );
            }
        } else if(name == "Subtree") {
            if(children_count != 0) {
                ThrowError(
                        line_number, "<Subtree> should not have any GetChildNode"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Subtree> must have the "
                        "attribute [ID]"
                );
            }
            if(rRegisteredNodes.count(ID) != 0) {
                ThrowError(
                        line_number,
                        "The attribute [ID] of tag <Subtree> must "
                        "not use the name of a registered Node"
                );
            }
        } else if(name == "BehaviorTree") {
            if(ID.empty() && behavior_tree_count > 1) {
                ThrowError(
                        line_number,
                        "The tag <BehaviorTree> must have the "
                        "attribute [ID]"
                );
            }
            if(children_count != 1) {
                ThrowError(
                        line_number,
                        "The tag <BehaviorTree> must have exactly 1 "
                        "GetChildNode"
                );
            }
            if(rRegisteredNodes.count(ID) != 0) {
                ThrowError(
                        line_number,
                        "The attribute [ID] of tag <BehaviorTree> "
                        "must not use the name of a registered Node"
                );
            }
        } else {
            // search in the factory and the list of subtrees
            const auto search = rRegisteredNodes.find(name);
            bool found = (search != rRegisteredNodes.end());
            if(!found) {
                ThrowError(
                        line_number, std::string("Node not recognized: ") + name
                );
            }

            if(search->second == NodeType::Decorator) {
                if(children_count != 1) {
                    ThrowError(
                            line_number,
                            std::string("The node <") + name +
                                    "> must have exactly 1 GetChildNode"
                    );
                }
            } else if(search->second == NodeType::Control) {
                if(children_count == 0) {
                    ThrowError(
                            line_number,
                            std::string("The node <") + name +
                                    "> must have 1 or more Children"
                    );
                }
            }
        }
        //recursion
        for(auto child = node->FirstChildElement(); child != nullptr;
            child = child->NextSiblingElement()) {
            RecursiveStep(child);
        }
    };

    for(auto bt_root = xml_root->FirstChildElement("BehaviorTree");
        bt_root != nullptr;
        bt_root = bt_root->NextSiblingElement("BehaviorTree")) {
        RecursiveStep(bt_root);
    }
}

Tree JsonParser::InstantiateTree(const Blackboard::Ptr &rRootBlackboard, std::string main_tree_ID) {
    Tree output_tree;

    // use the main_tree_to_execute argument if it was provided by the user
    // or the one in the FIRST document opened
    if(main_tree_ID.empty()) {
        XMLElement *first_xml_root = m_pPImpl->openedJsonList.front()->RootElement();

        if(auto main_tree_attribute = first_xml_root->Attribute("main_tree_to_execute")) {
            main_tree_ID = main_tree_attribute;
        } else if(m_pPImpl->treeRootMap.size() == 1) {
            // special case: there is only one registered BT.
            main_tree_ID = m_pPImpl->treeRootMap.begin()->first;
        } else {
            throw util::RuntimeError("[main_tree_to_execute] was not specified correctly");
        }
    }

    //--------------------------------------
    if(!rRootBlackboard) {
        throw util::RuntimeError("XMLParser::InstantiateTree needs a non-Empty root_blackboard");
    }

    m_pPImpl->RecursivelyCreateSubtree(main_tree_ID, {}, {}, output_tree, rRootBlackboard, TreeNode::Ptr());
    output_tree.Initialize();
    return output_tree;
}

void JsonParser::ClearInternalState() {
    m_pPImpl->Clear();
}

TreeNode::Ptr JsonParser::PImpl::CreateNodeFromJson(const XMLElement *pElement, const Blackboard::Ptr &rBlackboard, const TreeNode::Ptr &rNodeParent, const std::string &rPrefixPath, Tree &rOutputTree) {
    const auto element_name = pElement->Name();
    const auto element_ID = pElement->Attribute("Id");

    auto node_type = ConvertFromString<NodeType>(element_name);
    // name used by the factory
    std::string type_ID;

    if(node_type == NodeType::Undefined) {
        // This is the case of nodes like <MyCustomAction>
        // check if the factory has this name
        if(rFactory.GetBuilder().count(element_name) == 0) {
            throw util::RuntimeError(element_name, " is not a registered node");
        }
        type_ID = element_name;

        if(element_ID) {
            throw util::RuntimeError("Attribute [ID] is not allowed in <", type_ID, ">");
        }
    } else {
        // in this case, it is mandatory to have a field "ID"
        if(!element_ID) {
            throw util::RuntimeError("Attribute [ID] is mandatory in <", type_ID, ">");
        }
        type_ID = element_ID;
    }

    // By default, the instance name is equal to ID, unless the
    // attribute [name] is present.
    const char *attr_name = pElement->Attribute("name");
    const std::string instance_name = (attr_name != nullptr) ? attr_name : type_ID;

    const TreeNodeManifest *manifest = nullptr;

    auto manifest_it = rFactory.GetManifest().find(type_ID);
    if(manifest_it != rFactory.GetManifest().end()) {
        manifest = &manifest_it->second;
    }

    PortsRemapping port_remap;
    for(const XMLAttribute *att = pElement->FirstAttribute(); att;
        att = att->Next()) {
        if(IsAllowedPortName(att->Name())) {
            const std::string port_name = att->Name();
            const std::string port_value = att->Value();

            if(manifest) {
                auto port_model_it = manifest->portMap.find(port_name);
                if(port_model_it == manifest->portMap.end()) {
                    throw util::RuntimeError(
                            util::StrCat("a port with name [", port_name,
                                         "] is found in the XML, but not in the "
                                         "ProvidedPorts()")
                    );
                }

                const auto &port_model = port_model_it->second;
                bool is_blacbkboard = port_value.size() >= 3 &&
                                      port_value.front() == '{' &&
                                      port_value.back() == '}';
                // let's test already if conversion is possible
                if(!is_blacbkboard && port_model.Converter() &&
                   port_model.IsStronglyTyped()) {
                    // This may throw
                    try {
                        port_model.Converter()(port_value);
                    } catch(std::exception &ex) {
                        auto msg =
                                util::StrCat("The port with name \"", port_name, "\" and value \"", port_value, "\" can not be converted to ", port_model.TypeName());
                        throw util::LogicError(msg);
                    }
                }
            }

            port_remap[port_name] = port_value;
        }
    }

    NodeConfig config;
    config.pBlackboard = rBlackboard;
    config.path = rPrefixPath + instance_name;
    config.uid = rOutputTree.GetUid();
    config.pManifest = manifest;

    if(type_ID == instance_name) {
        config.path += std::string("::") + std::to_string(config.uid);
    }

    auto AddCondition = [&](auto &conditions, const char *attr_name, auto ID) {
        if(auto script = pElement->Attribute(attr_name)) {
            conditions.insert({ID, std::string(script)});
        }
    };

    for(int i = 0; i < int(PreCond::Count); i++) {
        auto pre = static_cast<PreCond>(i);
        AddCondition(config.preConditionMap, ToStr(pre).c_str(), pre);
    }
    for(int i = 0; i < int(PostCond::Count); i++) {
        auto post = static_cast<PostCond>(i);
        AddCondition(config.postConditionMap, ToStr(post).c_str(), post);
    }

    //---------------------------------------------
    TreeNode::Ptr new_node;

    if(node_type == NodeType::Subtree) {
        config.inputPortMap = port_remap;
        new_node = rFactory.InstantiateTreeNode(instance_name, ToStr(NodeType::Subtree), config);
        auto subtree_node = dynamic_cast<SubtreeNode *>(new_node.get());
        subtree_node->SetSubtreeId(type_ID);
    } else {
        if(!manifest) {
            auto msg =
                    util::StrCat("Missing manifest for element_ID: ", element_ID, ". It shouldn't happen. Please report this issue.");
            throw util::RuntimeError(msg);
        }

        //Check that name in remapping can be found in the manifest
        for(const auto &[name_in_subtree, _]: port_remap) {
            if(manifest->portMap.count(name_in_subtree) == 0) {
                throw util::RuntimeError(
                        "Possible typo? In the XML, you tried to remap port \"",
                        name_in_subtree, "\" in node [", config.path, "(Type ",
                        type_ID,
                        ")], but the manifest/model of this node does not "
                        "contain a "
                        "port "
                        "with this name."
                );
            }
        }

        // Initialize the ports in the BB to set the type
        for(const auto &[port_name, port_info]: manifest->portMap) {
            auto remap_it = port_remap.find(port_name);
            if(remap_it == port_remap.end()) {
                continue;
            }
            std::string_view remapped_port = remap_it->second;

            if(auto param_res =
                       TreeNode::GetRemappedKey(port_name, remapped_port)) {
                // port_key will contain the key to find the entry in the blackboard
                const auto port_key =
                        static_cast<std::string>(param_res.value());

                // if the entry already exists, check that the type is the same
                if(auto prev_info = rBlackboard->GetEntryInfo(port_key)) {
                    // Check consistency of types.
                    bool const port_type_mismatch = (prev_info->IsStronglyTyped() && port_info.IsStronglyTyped() && prev_info->Type() != port_info.Type());

                    // special case related to convertFromString
                    bool const string_input = (prev_info->Type() == typeid(std::string));

                    if(port_type_mismatch && !string_input) {
                        rBlackboard->DebugMessage();

                        throw util::RuntimeError(
                                "The creation of the tree failed because the "
                                "port [",
                                port_key, "] was initially created with Type [", Demangle(prev_info->Type()), "] and, later Type [", Demangle(port_info.Type()), "] was used somewhere else."
                        );
                    }
                } else {
                    // not found, insert for the first time.
                    rBlackboard->CreateEntry(port_key, port_info);
                }
            }
        }

        // Set the port direction in config
        for(const auto &remap_it: port_remap) {
            const auto &port_name = remap_it.first;
            auto port_it = manifest->portMap.find(port_name);
            if(port_it != manifest->portMap.end()) {
                auto direction = port_it->second.Direction();
                if(direction != PortDirection::Output) {
                    config.inputPortMap.insert(remap_it);
                }
                if(direction != PortDirection::Input) {
                    config.outputPortMap.insert(remap_it);
                }
            }
        }

        // use default value if available for empty ports. Only inputs
        for(const auto &port_it: manifest->portMap) {
            const std::string &port_name = port_it.first;
            const PortInfo &port_info = port_it.second;

            const auto direction = port_info.Direction();
            const auto &default_string = port_info.DefaultValueString();
            if(!default_string.empty()) {
                if(direction != PortDirection::Output &&
                   config.inputPortMap.count(port_name) == 0) {
                    config.inputPortMap.insert({port_name, default_string});
                }

                if(direction != PortDirection::Input &&
                   config.outputPortMap.count(port_name) == 0 &&
                   TreeNode::IsBlackboardPointer(default_string)) {
                    config.outputPortMap.insert({port_name, default_string});
                }
            }
        }

        new_node = rFactory.InstantiateTreeNode(instance_name, type_ID, config);
    }

    // add the pointer of this node to the parent
    if(rNodeParent != nullptr) {
        if(auto control_parent =
                   dynamic_cast<ControlNode *>(rNodeParent.get())) {
            control_parent->AddChildNode(new_node.get());
        } else if(auto decorator_parent =
                          dynamic_cast<DecoratorNode *>(rNodeParent.get())) {
            decorator_parent->SetChildNode(new_node.get());
        }
    }

    return new_node;
}

void behaviortree::JsonParser::PImpl::RecursivelyCreateSubtree(const std::string &rTreeId, const std::string &rTreePath, const std::string &rPrefixPath, Tree &rOutputTree, Blackboard::Ptr pBlackboard, const TreeNode::Ptr &pRootNode) {
    std::function<void(const TreeNode::Ptr &, Tree::Subtree::Ptr, std::string, const XMLElement *)> recursiveStep;
    recursiveStep = [&](TreeNode::Ptr pParentNode, Tree::Subtree::Ptr pSubtree, std::string prefix, const XMLElement *element) {
        // create the node
        auto pTreeNode = CreateNodeFromJson(element, pBlackboard, pParentNode, prefix, rOutputTree);
        pSubtree->nodeVec.push_back(pTreeNode);

        // common case: iterate through all children
        if(pTreeNode->Type() != NodeType::Subtree) {
            for(auto child_element = element->FirstChildElement(); child_element; child_element = child_element->NextSiblingElement()) {
                recursiveStep(pTreeNode, pSubtree, prefix, child_element);
            }
        } else {// special case: SubtreeNode
            auto new_bb = Blackboard::Create(pBlackboard);
            const std::string subtreeId = element->Attribute("Id");
            std::unordered_map<std::string, std::string> subtree_remapping;
            bool do_autoremap = false;

            for(auto attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
                std::string attr_name = attr->Name();
                std::string attr_value = attr->Value();
                if(attr_value == "{=}") {
                    attr_value = util::StrCat("{", attr_name, "}");
                }

                if(attr_name == "_autoremap") {
                    do_autoremap = ConvertFromString<bool>(attr_value);
                    new_bb->EnableAutoRemapping(do_autoremap);
                    continue;
                }
                if(!IsAllowedPortName(attr->Name())) {
                    continue;
                }
                subtree_remapping.insert({attr_name, attr_value});
            }
            // check if this subtree has a model. If it does,
            // we want to check if all the mandatory ports were remapped and
            // add default ones, if necessary
            auto subtree_model_it = subtreeModelMap.find(subtreeId);
            if(subtree_model_it != subtreeModelMap.end()) {
                const auto &subtree_model_ports = subtree_model_it->second.portMap;
                // check if:
                // - remapping contains mondatory ports
                // - if any of these has default value
                for(const auto &[port_name, port_info]: subtree_model_ports) {
                    auto it = subtree_remapping.find(port_name);
                    // don't override existing remapping
                    if(it == subtree_remapping.end() && !do_autoremap) {
                        // remapping is not explicitly defined in the XML: use the model
                        if(port_info.DefaultValueString().empty()) {
                            auto msg = util::StrCat("In the <TreeNodesModel> the <Subtree ID=\"", subtreeId, "\"> is defining a mandatory port called [", port_name, "], but you are not remapping it");
                            throw util::RuntimeError(msg);
                        } else {
                            subtree_remapping.insert({port_name, port_info.DefaultValueString()});
                        }
                    }
                }
            }

            for(const auto &[attr_name, attr_value]: subtree_remapping) {
                if(TreeNode::IsBlackboardPointer(attr_value)) {
                    // do remapping
                    std::string_view port_name = TreeNode::StripBlackboardPointer(attr_value);
                    new_bb->AddSubtreeRemapping(attr_name, port_name);
                } else {
                    // constant string: just set that constant value into the BB
                    // IMPORTANT: this must not be autoremapped!!!
                    new_bb->EnableAutoRemapping(false);
                    new_bb->Set(attr_name, static_cast<std::string>(attr_value));
                    new_bb->EnableAutoRemapping(do_autoremap);
                }
            }

            std::string subtreePath = pSubtree->instanceName;
            if(!subtreePath.empty()) {
                subtreePath += "/";
            }
            if(auto name = element->Attribute("name")) {
                subtreePath += name;
            } else {
                subtreePath += subtreeId + "::" + std::to_string(pTreeNode->GetUid());
            }

            RecursivelyCreateSubtree(
                    subtreeId,
                    subtreePath,      // name
                    subtreePath + "/",//prefix
                    rOutputTree, new_bb, pTreeNode
            );
        }
    };

    auto iter = treeRootMap.find(rTreeId);
    if(iter == treeRootMap.end()) {
        throw std::runtime_error(std::string("Can't find a tree with name: ") + rTreeId);
    }

    auto root_element = iter->second->FirstChildElement();

    //-------- start recursion -----------

    // Append a new subtree to the list
    auto new_tree = std::make_shared<Tree::Subtree>();
    new_tree->pBlackboard = pBlackboard;
    new_tree->instanceName = rTreePath;
    new_tree->treeId = rTreeId;
    rOutputTree.m_subtreeVec.push_back(new_tree);

    recursiveStep(pRootNode, new_tree, rPrefixPath, root_element);
}

void JsonParser::PImpl::GetPortsRecursively(const XMLElement *element, std::vector<std::string> &rOutputPortVec) {
    for(const XMLAttribute *attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
        const char *attr_name = attr->Name();
        const char *attr_value = attr->Value();
        if(IsAllowedPortName(attr_name) && TreeNode::IsBlackboardPointer(attr_value)) {
            auto port_name = TreeNode::StripBlackboardPointer(attr_value);
            rOutputPortVec.push_back(static_cast<std::string>(port_name));
        }
    }

    for(auto child_element = element->FirstChildElement(); child_element;
        child_element = child_element->NextSiblingElement()) {
        GetPortsRecursively(child_element, rOutputPortVec);
    }
}

void AddNodeModelToJson(const TreeNodeManifest &model, XMLDocument &doc, XMLElement *model_root) {
    XMLElement *element = doc.NewElement(ToStr(model.type).c_str());
    element->SetAttribute("ID", model.registrationId.c_str());

    for(const auto &[port_name, port_info]: model.portMap) {
        XMLElement *port_element = nullptr;
        switch(port_info.Direction()) {
            case PortDirection::Input:
                port_element = doc.NewElement("input_port");
                break;
            case PortDirection::Output:
                port_element = doc.NewElement("output_port");
                break;
            case PortDirection::InOut:
                port_element = doc.NewElement("inout_port");
                break;
        }

        port_element->SetAttribute("name", port_name.c_str());
        if(port_info.Type() != typeid(void)) {
            port_element->SetAttribute("Type", behaviortree::Demangle(port_info.Type()).c_str());
        }
        if(!port_info.DefaultValue().Empty()) {
            port_element->SetAttribute("default", port_info.DefaultValueString().c_str());
        }

        if(!port_info.Description().empty()) {
            port_element->SetText(port_info.Description().c_str());
        }
        element->InsertEndChild(port_element);
    }

    if(!model.metadataVec.empty()) {
        auto metadata_root = doc.NewElement("MetadataFields");

        for(const auto &[name, value]: model.metadataVec) {
            auto metadata_element = doc.NewElement("Metadata");
            metadata_element->SetAttribute(name.c_str(), value.c_str());
            metadata_root->InsertEndChild(metadata_element);
        }

        element->InsertEndChild(metadata_root);
    }

    model_root->InsertEndChild(element);
}

void AddTreeToJson(const Tree &tree, XMLDocument &doc, XMLElement *rootXML, bool add_metadata, bool add_builtin_models) {
    std::function<void(const TreeNode &, XMLElement *)> addNode;
    addNode = [&](const TreeNode &node, XMLElement *parent_elem) {
        XMLElement *elem = nullptr;

        if(auto subtree = dynamic_cast<const SubtreeNode *>(&node)) {
            elem = doc.NewElement(node.GetRegistrAtionName().c_str());
            elem->SetAttribute("ID", subtree->GetSubtreeId().c_str());
            if(add_metadata) {
                elem->SetAttribute("_fullpath", subtree->GetConfig().path.c_str());
            }
        } else {
            elem = doc.NewElement(node.GetRegistrAtionName().c_str());
            elem->SetAttribute("name", node.GetNodeName().c_str());
        }

        if(add_metadata) {
            elem->SetAttribute("_uid", node.GetUid());
        }

        for(const auto &[name, value]: node.GetConfig().inputPortMap) {
            elem->SetAttribute(name.c_str(), value.c_str());
        }
        for(const auto &[name, value]: node.GetConfig().outputPortMap) {
            // avoid duplicates, in the case of INOUT ports
            if(node.GetConfig().inputPortMap.count(name) == 0) {
                elem->SetAttribute(name.c_str(), value.c_str());
            }
        }

        for(const auto &[pre, script]: node.GetConfig().preConditionMap) {
            elem->SetAttribute(ToStr(pre).c_str(), script.c_str());
        }
        for(const auto &[post, script]: node.GetConfig().postConditionMap) {
            elem->SetAttribute(ToStr(post).c_str(), script.c_str());
        }

        parent_elem->InsertEndChild(elem);

        if(auto control = dynamic_cast<const ControlNode *>(&node)) {
            for(const auto &child: control->GetChildrenNode()) {
                addNode(*child, elem);
            }
        } else if(auto decorator = dynamic_cast<const DecoratorNode *>(&node)) {
            if(decorator->Type() != NodeType::Subtree) {
                addNode(*decorator->GetChildNode(), elem);
            }
        }
    };

    for(const auto &subtree: tree.m_subtreeVec) {
        XMLElement *subtree_elem = doc.NewElement("BehaviorTree");
        subtree_elem->SetAttribute("ID", subtree->treeId.c_str());
        subtree_elem->SetAttribute("_fullpath", subtree->instanceName.c_str());
        rootXML->InsertEndChild(subtree_elem);
        addNode(*subtree->nodeVec.front(), subtree_elem);
    }

    XMLElement *model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    static const BehaviorTreeFactory temp_factory;

    std::map<std::string, const TreeNodeManifest *> ordered_models;
    for(const auto &[registration_ID, model]: tree.m_manifestsMap) {
        if(add_builtin_models ||
           !temp_factory.GetBuiltinNodes().count(registration_ID)) {
            ordered_models.insert({registration_ID, &model});
        }
    }

    for(const auto &[registration_ID, model]: ordered_models) {
        AddNodeModelToJson(*model, doc, model_root);
    }
}

std::string WriteTreeNodesModelJson(const BehaviorTreeFactory &rFactory, bool include_builtin) {
    XMLDocument doc;

    XMLElement *rootXML = doc.NewElement("root");
    rootXML->SetAttribute("BTCPP_format", "4");
    doc.InsertFirstChild(rootXML);

    XMLElement *model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    std::map<std::string, const TreeNodeManifest *> ordered_models;

    for(const auto &[registration_ID, model]: rFactory.GetManifest()) {
        if(include_builtin ||
           rFactory.GetBuiltinNodes().count(registration_ID) == 0) {
            ordered_models.insert({registration_ID, &model});
        }
    }

    for(const auto &[registration_ID, model]: ordered_models) {
        AddNodeModelToJson(*model, doc, model_root);
    }

    XMLPrinter printer;
    doc.Print(&printer);
    return std::string(printer.CStr(), size_t(printer.CStrSize() - 1));
}

Tree BuildTreeFromText(const BehaviorTreeFactory &factory, const std::string &text, const Blackboard::Ptr &blackboard) {
    JsonParser jsonParser(factory);
    jsonParser.LoadFromText(text);
    return jsonParser.InstantiateTree(blackboard);
}

Tree BuildTreeFromFile(const BehaviorTreeFactory &factory, const std::string &filename, const Blackboard::Ptr &blackboard) {
    JsonParser jsonParser(factory);
    jsonParser.LoadFromFile(filename);
    return jsonParser.InstantiateTree(blackboard);
}

std::string WriteTreeToJson(const Tree &rTree, bool add_metadata, bool add_builtin_models) {
    XMLDocument doc;

    XMLElement *rootXML = doc.NewElement("root");
    rootXML->SetAttribute("BTCPP_format", 4);
    doc.InsertFirstChild(rootXML);

    AddTreeToJson(rTree, doc, rootXML, add_metadata, add_builtin_models);

    XMLPrinter printer;
    doc.Print(&printer);
    return std::string(printer.CStr(), size_t(printer.CStrSize() - 1));
}

}// namespace behaviortree
