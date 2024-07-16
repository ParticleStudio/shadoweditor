#include "behaviortree/json_parsing.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <typeindex>

#if defined(__linux) || defined(__linux__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wattributes"
#endif

#include <filesystem>
#include <map>

#include "behaviortree/blackboard.h"
#include "behaviortree/tree_node.h"
#include "behaviortree/util/demangle_util.h"
#include "tinyxml2.h"

namespace {
std::string xsdAttributeType(const behaviortree::PortInfo &refPortInfo) {
    if(refPortInfo.Direction() == behaviortree::PortDirection::OUTPUT) {
        return "blackboardType";
    }
    const auto &refTypeInfo = refPortInfo.Type();
    if((refTypeInfo == typeid(int)) or (refTypeInfo == typeid(unsigned int))) {
        return "integerOrBlackboardType";
    } else if(refTypeInfo == typeid(double)) {
        return "decimalOrBlackboardType";
    } else if(refTypeInfo == typeid(bool)) {
        return "booleanOrBlackboardType";
    } else if(refTypeInfo == typeid(std::string)) {
        return "stringOrBlackboardType";
    }

    return std::string();
}

}// namespace

namespace behaviortree {
using namespace tinyxml2;

auto StrEqual = [](const char *str1, const char *str2) -> bool {
    return strcmp(str1, str2) == 0;
};

struct SubtreeModel {
    std::unordered_map<std::string, behaviortree::PortInfo> ports;
};

struct JsonParser::PImpl {
    TreeNode::Ptr CreateNodeFromJson(const XMLElement *element, const Blackboard::Ptr &blackboard, const TreeNode::Ptr &node_parent, const std::string &prefix_path, Tree &output_tree);

    void RecursivelyCreateSubtree(const std::string &tree_ID, const std::string &tree_path, const std::string &prefix_path, Tree &output_tree, Blackboard::Ptr blackboard, const TreeNode::Ptr &root_node);

    void GetPortsRecursively(const XMLElement *element, std::vector<std::string> &output_ports);

    void LoadDocImpl(XMLDocument *doc, bool add_includes);

    std::list<std::unique_ptr<XMLDocument> > openedDocumentList;
    std::map<std::string, const XMLElement *> treeRootMap;

    const BehaviorTreeFactory &refFactory;

    std::filesystem::path currentPath;
    std::map<std::string, SubtreeModel> subtreeModelMap;

    int32_t suffixCount;

    explicit PImpl(const BehaviorTreeFactory &fact): refFactory(fact),
                                                     currentPath(std::filesystem::current_path()),
                                                     suffixCount(0) {}

    void Clear() {
        suffixCount = 0;
        currentPath = std::filesystem::current_path();
        openedDocumentList.clear();
        treeRootMap.clear();
    }

 private:
    void LoadSubtreeModel(const XMLElement *xml_root);
};

#if defined(__linux) || defined(__linux__)
#    pragma GCC diagnostic pop
#endif

JsonParser::JsonParser(const BehaviorTreeFactory &factory): m_P(new PImpl(factory)) {}

JsonParser::JsonParser(JsonParser &&other) noexcept {
    this->m_P = std::move(other.m_P);
}

JsonParser &JsonParser::operator=(JsonParser &&other) noexcept {
    this->m_P = std::move(other.m_P);
    return *this;
}

JsonParser::~JsonParser() = default;

void JsonParser::LoadFromFile(const std::filesystem::path &filepath, bool add_includes) {
    m_P->openedDocumentList.emplace_back(new XMLDocument());

    XMLDocument *doc = m_P->openedDocumentList.back().get();
    doc->LoadFile(filepath.string().c_str());

    m_P->currentPath = std::filesystem::absolute(filepath.parent_path());

    m_P->LoadDocImpl(doc, add_includes);
}

void JsonParser::LoadFromText(const std::string &xml_text, bool add_includes) {
    m_P->openedDocumentList.emplace_back(new XMLDocument());

    XMLDocument *doc = m_P->openedDocumentList.back().get();
    doc->Parse(xml_text.c_str(), xml_text.size());

    m_P->LoadDocImpl(doc, add_includes);
}

std::vector<std::string> JsonParser::RegisteredBehaviorTrees() const {
    std::vector<std::string> out;
    for(const auto &it: m_P->treeRootMap) {
        out.push_back(it.first);
    }
    return out;
}

void behaviortree::JsonParser::PImpl::LoadSubtreeModel(const XMLElement *xml_root) {
    for(auto models_node = xml_root->FirstChildElement("TreeNodesModel");
        models_node != nullptr;
        models_node = models_node->NextSiblingElement("TreeNodesModel")) {
        for(auto sub_node = models_node->FirstChildElement("SubTree");
            sub_node != nullptr;
            sub_node = sub_node->NextSiblingElement("SubTree")) {
            auto subtree_id = sub_node->Attribute("ID");
            auto &subtree_model = subtreeModelMap[subtree_id];

            std::pair<const char *, behaviortree::PortDirection> port_types[3] =
                    {{"input_port", behaviortree::PortDirection::INPUT},
                     {"output_port", behaviortree::PortDirection::OUTPUT},
                     {"inout_port", behaviortree::PortDirection::INOUT}};

            for(const auto &[name, direction]: port_types) {
                for(auto port_node = sub_node->FirstChildElement(name);
                    port_node != nullptr;
                    port_node = port_node->NextSiblingElement(name)) {
                    behaviortree::PortInfo port(direction);
                    auto name = port_node->Attribute("name");
                    if(!name) {
                        throw RuntimeError(
                                "Missing attribute [name] in port (SubTree "
                                "model)"
                        );
                    }
                    if(auto default_value = port_node->Attribute("default")) {
                        port.SetDefaultValue(default_value);
                    }
                    if(auto description = port_node->Attribute("description")) {
                        port.SetDescription(description);
                    }
                    subtree_model.ports[name] = std::move(port);
                }
            }
        }
    }
}

void JsonParser::PImpl::LoadDocImpl(XMLDocument *doc, bool add_includes) {
    if(doc->Error()) {
        char buffer[512];
        snprintf(
                buffer, sizeof buffer, "Error parsing the XML: %s",
                doc->ErrorStr()
        );
        throw RuntimeError(buffer);
    }

    const XMLElement *xml_root = doc->RootElement();

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
        if(!add_includes) {
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
                throw RuntimeError(
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

        openedDocumentList.emplace_back(new XMLDocument());
        XMLDocument *next_doc = openedDocumentList.back().get();

        // change current path to the included file for handling additional relative paths
        const auto previous_path = currentPath;
        currentPath = std::filesystem::absolute(file_path.parent_path());

        next_doc->LoadFile(file_path.string().c_str());
        LoadDocImpl(next_doc, add_includes);

        // reset current path to the previous value
        currentPath = previous_path;
    }

    // Collect the names of all nodes registered with the behavior tree factory
    std::unordered_map<std::string, behaviortree::NodeType> registered_nodes;
    for(const auto &it: refFactory.Manifests()) {
        registered_nodes.insert({it.first, it.second.type});
    }

    XMLPrinter printer;
    doc->Print(&printer);
    auto xml_text = std::string(printer.CStr(), size_t(printer.CStrSize()));

    // Verify the validity of the XML before adding any behavior trees to the parser's list of registered trees
    VerifyJson(xml_text, registered_nodes);

    LoadSubtreeModel(xml_root);

    // Register each BehaviorTree within the XML
    for(auto bt_node = xml_root->FirstChildElement("BehaviorTree");
        bt_node != nullptr;
        bt_node = bt_node->NextSiblingElement("BehaviorTree")) {
        std::string tree_name;
        if(bt_node->Attribute("ID")) {
            tree_name = bt_node->Attribute("ID");
        } else {
            tree_name = "BehaviorTree_" + std::to_string(suffixCount++);
        }

        treeRootMap[tree_name] = bt_node;
    }
}

void VerifyJson(const std::string &xml_text, const std::unordered_map<std::string, behaviortree::NodeType> &registered_nodes) {
    XMLDocument doc;
    auto xml_error = doc.Parse(xml_text.c_str(), xml_text.size());
    if(xml_error) {
        char buffer[512];
        snprintf(
                buffer, sizeof buffer, "Error parsing the XML: %s",
                doc.ErrorName()
        );
        throw RuntimeError(buffer);
    }

    //-------- Helper functions (lambdas) -----------------
    auto ThrowError = [&](int line_num, const std::string &text) {
        char buffer[512];
        snprintf(
                buffer, sizeof buffer, "Error at line %d: -> %s", line_num,
                text.c_str()
        );
        throw RuntimeError(buffer);
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
        throw RuntimeError("The XML must have a root node called <root>");
    }
    //-------------------------------------------------
    auto models_root = xml_root->FirstChildElement("TreeNodesModel");
    auto meta_sibling =
            models_root ? models_root->NextSiblingElement("TreeNodesModel")
                        : nullptr;

    if(meta_sibling) {
        ThrowError(
                meta_sibling->GetLineNum(),
                " Only a single node <TreeNodesModel> is "
                "supported"
        );
    }
    if(models_root) {
        // not having a MetaModel is not an error. But consider that the
        // Graphical editor needs it.
        for(auto node = xml_root->FirstChildElement(); node != nullptr;
            node = node->NextSiblingElement()) {
            const std::string name = node->Name();
            if(name == "Action" || name == "Decorator" || name == "SubTree" ||
               name == "Condition" || name == "Control") {
                const char *ID = node->Attribute("ID");
                if(!ID) {
                    ThrowError(
                            node->GetLineNum(),
                            "Error at line %d: -> The attribute "
                            "[ID] is mandatory"
                    );
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
    std::function<void(const XMLElement *)> recursiveStep;

    recursiveStep = [&](const XMLElement *node) {
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
                        "GetChild"
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
                        "GetChild"
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
                        "GetChild"
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
                        "GetChild"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <Control> must have the "
                        "attribute [ID]"
                );
            }
        } else if(name == "SubTree") {
            if(children_count != 0) {
                ThrowError(
                        line_number, "<SubTree> should not have any GetChild"
                );
            }
            if(ID.empty()) {
                ThrowError(
                        line_number,
                        "The tag <SubTree> must have the "
                        "attribute [ID]"
                );
            }
            if(registered_nodes.count(ID) != 0) {
                ThrowError(
                        line_number,
                        "The attribute [ID] of tag <SubTree> must "
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
                        "GetChild"
                );
            }
            if(registered_nodes.count(ID) != 0) {
                ThrowError(
                        line_number,
                        "The attribute [ID] of tag <BehaviorTree> "
                        "must not use the name of a registered Node"
                );
            }
        } else {
            // search in the factory and the list of subtrees
            const auto search = registered_nodes.find(name);
            bool found = (search != registered_nodes.end());
            if(!found) {
                ThrowError(
                        line_number, std::string("Node not recognized: ") + name
                );
            }

            if(search->second == NodeType::DECORATOR) {
                if(children_count != 1) {
                    ThrowError(
                            line_number,
                            std::string("The node <") + name +
                                    "> must have exactly 1 GetChild"
                    );
                }
            } else if(search->second == NodeType::CONTROL) {
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
            recursiveStep(child);
        }
    };

    for(auto bt_root = xml_root->FirstChildElement("BehaviorTree");
        bt_root != nullptr;
        bt_root = bt_root->NextSiblingElement("BehaviorTree")) {
        recursiveStep(bt_root);
    }
}

Tree JsonParser::InstantiateTree(const Blackboard::Ptr &root_blackboard, std::string main_tree_ID) {
    Tree output_tree;

    // use the main_tree_to_execute argument if it was provided by the user
    // or the one in the FIRST document opened
    if(main_tree_ID.empty()) {
        XMLElement *first_xml_root = m_P->openedDocumentList.front()->RootElement();

        if(auto main_tree_attribute = first_xml_root->Attribute("main_tree_to_execute")) {
            main_tree_ID = main_tree_attribute;
        } else if(m_P->treeRootMap.size() == 1) {
            // special case: there is only one registered BT.
            main_tree_ID = m_P->treeRootMap.begin()->first;
        } else {
            throw RuntimeError("[main_tree_to_execute] was not specified correctly");
        }
    }

    //--------------------------------------
    if(!root_blackboard) {
        throw RuntimeError("XMLParser::InstantiateTree needs a non-Empty root_blackboard");
    }

    m_P->RecursivelyCreateSubtree(main_tree_ID, {}, {}, output_tree, root_blackboard, TreeNode::Ptr());
    output_tree.Initialize();
    return output_tree;
}

void JsonParser::ClearInternalState() {
    m_P->Clear();
}

TreeNode::Ptr JsonParser::PImpl::CreateNodeFromJson(const XMLElement *element, const Blackboard::Ptr &blackboard, const TreeNode::Ptr &node_parent, const std::string &prefix_path, Tree &output_tree) {
    const auto element_name = element->Name();
    const auto element_ID = element->Attribute("ID");

    auto node_type = ConvertFromString<NodeType>(element_name);
    // name used by the factory
    std::string type_ID;

    if(node_type == NodeType::UNDEFINED) {
        // This is the case of nodes like <MyCustomAction>
        // check if the factory has this name
        if(refFactory.Builders().count(element_name) == 0) {
            throw RuntimeError(element_name, " is not a registered node");
        }
        type_ID = element_name;

        if(element_ID) {
            throw RuntimeError(
                    "Attribute [ID] is not allowed in <", type_ID, ">"
            );
        }
    } else {
        // in this case, it is mandatory to have a field "ID"
        if(!element_ID) {
            throw RuntimeError(
                    "Attribute [ID] is mandatory in <", type_ID, ">"
            );
        }
        type_ID = element_ID;
    }

    // By default, the instance name is equal to ID, unless the
    // attribute [name] is present.
    const char *attr_name = element->Attribute("name");
    const std::string instance_name =
            (attr_name != nullptr) ? attr_name : type_ID;

    const TreeNodeManifest *manifest = nullptr;

    auto manifest_it = refFactory.Manifests().find(type_ID);
    if(manifest_it != refFactory.Manifests().end()) {
        manifest = &manifest_it->second;
    }

    PortsRemapping port_remap;
    for(const XMLAttribute *att = element->FirstAttribute(); att;
        att = att->Next()) {
        if(IsAllowedPortName(att->Name())) {
            const std::string port_name = att->Name();
            const std::string port_value = att->Value();

            if(manifest) {
                auto port_model_it = manifest->ports.find(port_name);
                if(port_model_it == manifest->ports.end()) {
                    throw RuntimeError(
                            StrCat("a port with name [", port_name,
                                   "] is found in the XML, but not in the "
                                   "ProvidedPorts()")
                    );
                } else {
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
                                    StrCat("The port with name \"", port_name,
                                           "\" and value \"", port_value,
                                           "\" can not be converted to ",
                                           port_model.TypeName());
                            throw LogicError(msg);
                        }
                    }
                }
            }

            port_remap[port_name] = port_value;
        }
    }

    NodeConfig config;
    config.ptrBlackboard = blackboard;
    config.path = prefix_path + instance_name;
    config.uid = output_tree.GetUID();
    config.ptrManifest = manifest;

    if(type_ID == instance_name) {
        config.path += std::string("::") + std::to_string(config.uid);
    }

    auto AddCondition = [&](auto &conditions, const char *attr_name, auto ID) {
        if(auto script = element->Attribute(attr_name)) {
            conditions.insert({ID, std::string(script)});
        }
    };

    for(int i = 0; i < int(PreCond::COUNT); i++) {
        auto pre = static_cast<PreCond>(i);
        AddCondition(config.preConditions, ToStr(pre).c_str(), pre);
    }
    for(int i = 0; i < int(PostCond::COUNT); i++) {
        auto post = static_cast<PostCond>(i);
        AddCondition(config.postConditions, ToStr(post).c_str(), post);
    }

    //---------------------------------------------
    TreeNode::Ptr new_node;

    if(node_type == NodeType::SUBTREE) {
        config.inputPortsMap = port_remap;
        new_node = refFactory.InstantiateTreeNode(instance_name, ToStr(NodeType::SUBTREE), config);
        auto subtree_node = dynamic_cast<SubTreeNode *>(new_node.get());
        subtree_node->SetSubtreeId(type_ID);
    } else {
        if(!manifest) {
            auto msg =
                    StrCat("Missing manifest for element_ID: ", element_ID,
                           ". It shouldn't happen. Please report this issue.");
            throw RuntimeError(msg);
        }

        //Check that name in remapping can be found in the manifest
        for(const auto &[name_in_subtree, _]: port_remap) {
            if(manifest->ports.count(name_in_subtree) == 0) {
                throw RuntimeError(
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
        for(const auto &[port_name, port_info]: manifest->ports) {
            auto remap_it = port_remap.find(port_name);
            if(remap_it == port_remap.end()) {
                continue;
            }
            StringView remapped_port = remap_it->second;

            if(auto param_res =
                       TreeNode::GetRemappedKey(port_name, remapped_port)) {
                // port_key will contain the key to find the entry in the blackboard
                const auto port_key =
                        static_cast<std::string>(param_res.value());

                // if the entry already exists, check that the type is the same
                if(auto prev_info = blackboard->GetEntryInfo(port_key)) {
                    // Check consistency of types.
                    bool const port_type_mismatch = (prev_info->IsStronglyTyped() && port_info.IsStronglyTyped() && prev_info->Type() != port_info.Type());

                    // special case related to convertFromString
                    bool const string_input = (prev_info->Type() == typeid(std::string));

                    if(port_type_mismatch && !string_input) {
                        blackboard->DebugMessage();

                        throw RuntimeError(
                                "The creation of the tree failed because the "
                                "port [",
                                port_key, "] was initially created with Type [", Demangle(prev_info->Type()), "] and, later Type [", Demangle(port_info.Type()), "] was used somewhere else."
                        );
                    }
                } else {
                    // not found, insert for the first time.
                    blackboard->CreateEntry(port_key, port_info);
                }
            }
        }

        // Set the port direction in config
        for(const auto &remap_it: port_remap) {
            const auto &port_name = remap_it.first;
            auto port_it = manifest->ports.find(port_name);
            if(port_it != manifest->ports.end()) {
                auto direction = port_it->second.Direction();
                if(direction != PortDirection::OUTPUT) {
                    config.inputPortsMap.insert(remap_it);
                }
                if(direction != PortDirection::INPUT) {
                    config.outputPortsMap.insert(remap_it);
                }
            }
        }

        // use default value if available for empty ports. Only inputs
        for(const auto &port_it: manifest->ports) {
            const std::string &port_name = port_it.first;
            const PortInfo &port_info = port_it.second;

            const auto direction = port_info.Direction();
            const auto &default_string = port_info.DefaultValueString();
            if(!default_string.empty()) {
                if(direction != PortDirection::OUTPUT &&
                   config.inputPortsMap.count(port_name) == 0) {
                    config.inputPortsMap.insert({port_name, default_string});
                }

                if(direction != PortDirection::INPUT &&
                   config.outputPortsMap.count(port_name) == 0 &&
                   TreeNode::IsBlackboardPointer(default_string)) {
                    config.outputPortsMap.insert({port_name, default_string});
                }
            }
        }

        new_node = refFactory.InstantiateTreeNode(instance_name, type_ID, config);
    }

    // add the pointer of this node to the parent
    if(node_parent != nullptr) {
        if(auto control_parent =
                   dynamic_cast<ControlNode *>(node_parent.get())) {
            control_parent->AddChild(new_node.get());
        } else if(auto decorator_parent =
                          dynamic_cast<DecoratorNode *>(node_parent.get())) {
            decorator_parent->SetChild(new_node.get());
        }
    }

    return new_node;
}

void behaviortree::JsonParser::PImpl::RecursivelyCreateSubtree(
        const std::string &tree_ID, const std::string &tree_path,
        const std::string &prefix_path, Tree &output_tree,
        Blackboard::Ptr blackboard, const TreeNode::Ptr &root_node
) {
    std::function<
            void(const TreeNode::Ptr &, Tree::Subtree::Ptr, std::string,
                 const XMLElement *)>
            recursiveStep;

    recursiveStep = [&](TreeNode::Ptr parent_node, Tree::Subtree::Ptr subtree,
                        std::string prefix, const XMLElement *element) {
        // create the node
        auto node = CreateNodeFromJson(element, blackboard, parent_node, prefix, output_tree);
        subtree->ptrNodes.push_back(node);

        // common case: iterate through all children
        if(node->Type() != NodeType::SUBTREE) {
            for(auto child_element = element->FirstChildElement();
                child_element;
                child_element = child_element->NextSiblingElement()) {
                recursiveStep(node, subtree, prefix, child_element);
            }
        } else// special case: SubTreeNode
        {
            auto new_bb = Blackboard::Create(blackboard);
            const std::string subtree_ID = element->Attribute("ID");
            std::unordered_map<std::string, std::string> subtree_remapping;
            bool do_autoremap = false;

            for(auto attr = element->FirstAttribute(); attr != nullptr;
                attr = attr->Next()) {
                std::string attr_name = attr->Name();
                std::string attr_value = attr->Value();
                if(attr_value == "{=}") {
                    attr_value = StrCat("{", attr_name, "}");
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
            auto subtree_model_it = subtreeModelMap.find(subtree_ID);
            if(subtree_model_it != subtreeModelMap.end()) {
                const auto &subtree_model_ports =
                        subtree_model_it->second.ports;
                // check if:
                // - remapping contains mondatory ports
                // - if any of these has default value
                for(const auto &[port_name, port_info]: subtree_model_ports) {
                    auto it = subtree_remapping.find(port_name);
                    // don't override existing remapping
                    if(it == subtree_remapping.end() && !do_autoremap) {
                        // remapping is not explicitly defined in the XML: use the model
                        if(port_info.DefaultValueString().empty()) {
                            auto msg = StrCat(
                                    "In the <TreeNodesModel> the <Subtree "
                                    "ID=\"",
                                    subtree_ID,
                                    "\"> is defining a mandatory port called [",
                                    port_name, "], but you are not remapping it"
                            );
                            throw RuntimeError(msg);
                        } else {
                            subtree_remapping.insert(
                                    {port_name, port_info.DefaultValueString()}
                            );
                        }
                    }
                }
            }

            for(const auto &[attr_name, attr_value]: subtree_remapping) {
                if(TreeNode::IsBlackboardPointer(attr_value)) {
                    // do remapping
                    StringView port_name =
                            TreeNode::StripBlackboardPointer(attr_value);
                    new_bb->AddSubtreeRemapping(attr_name, port_name);
                } else {
                    // constant string: just set that constant value into the BB
                    // IMPORTANT: this must not be autoremapped!!!
                    new_bb->EnableAutoRemapping(false);
                    new_bb->Set(
                            attr_name, static_cast<std::string>(attr_value)
                    );
                    new_bb->EnableAutoRemapping(do_autoremap);
                }
            }

            std::string subtree_path = subtree->instanceName;
            if(!subtree_path.empty()) {
                subtree_path += "/";
            }
            if(auto name = element->Attribute("name")) {
                subtree_path += name;
            } else {
                subtree_path +=
                        subtree_ID + "::" + std::to_string(node->GetUID());
            }

            RecursivelyCreateSubtree(
                    subtree_ID,
                    subtree_path,      // name
                    subtree_path + "/",//prefix
                    output_tree, new_bb, node
            );
        }
    };

    auto it = treeRootMap.find(tree_ID);
    if(it == treeRootMap.end()) {
        throw std::runtime_error(
                std::string("Can't find a tree with name: ") + tree_ID
        );
    }

    auto root_element = it->second->FirstChildElement();

    //-------- start recursion -----------

    // Append a new subtree to the list
    auto new_tree = std::make_shared<Tree::Subtree>();
    new_tree->ptrBlackboard = blackboard;
    new_tree->instanceName = tree_path;
    new_tree->treeId = tree_ID;
    output_tree.ptrSubtrees.push_back(new_tree);

    recursiveStep(root_node, new_tree, prefix_path, root_element);
}

void JsonParser::PImpl::GetPortsRecursively(
        const XMLElement *element, std::vector<std::string> &output_ports
) {
    for(const XMLAttribute *attr = element->FirstAttribute(); attr != nullptr;
        attr = attr->Next()) {
        const char *attr_name = attr->Name();
        const char *attr_value = attr->Value();
        if(IsAllowedPortName(attr_name) &&
           TreeNode::IsBlackboardPointer(attr_value)) {
            auto port_name = TreeNode::StripBlackboardPointer(attr_value);
            output_ports.push_back(static_cast<std::string>(port_name));
        }
    }

    for(auto child_element = element->FirstChildElement(); child_element;
        child_element = child_element->NextSiblingElement()) {
        GetPortsRecursively(child_element, output_ports);
    }
}

void AddNodeModelToJson(const TreeNodeManifest &model, XMLDocument &doc, XMLElement *model_root) {
    XMLElement *element = doc.NewElement(ToStr(model.type).c_str());
    element->SetAttribute("ID", model.registrationId.c_str());

    for(const auto &[port_name, port_info]: model.ports) {
        XMLElement *port_element = nullptr;
        switch(port_info.Direction()) {
            case PortDirection::INPUT:
                port_element = doc.NewElement("input_port");
                break;
            case PortDirection::OUTPUT:
                port_element = doc.NewElement("output_port");
                break;
            case PortDirection::INOUT:
                port_element = doc.NewElement("inout_port");
                break;
        }

        port_element->SetAttribute("name", port_name.c_str());
        if(port_info.Type() != typeid(void)) {
            port_element->SetAttribute(
                    "Type", behaviortree::Demangle(port_info.Type()).c_str()
            );
        }
        if(!port_info.DefaultValue().Empty()) {
            port_element->SetAttribute(
                    "default", port_info.DefaultValueString().c_str()
            );
        }

        if(!port_info.Description().empty()) {
            port_element->SetText(port_info.Description().c_str());
        }
        element->InsertEndChild(port_element);
    }

    if(!model.metadata.empty()) {
        auto metadata_root = doc.NewElement("MetadataFields");

        for(const auto &[name, value]: model.metadata) {
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

        if(auto subtree = dynamic_cast<const SubTreeNode *>(&node)) {
            elem = doc.NewElement(node.GetRegistrAtionName().c_str());
            elem->SetAttribute("ID", subtree->GetSubtreeId().c_str());
            if(add_metadata) {
                elem->SetAttribute(
                        "_fullpath", subtree->GetConfig().path.c_str()
                );
            }
        } else {
            elem = doc.NewElement(node.GetRegistrAtionName().c_str());
            elem->SetAttribute("name", node.GetNodeName().c_str());
        }

        if(add_metadata) {
            elem->SetAttribute("_uid", node.GetUID());
        }

        for(const auto &[name, value]: node.GetConfig().inputPortsMap) {
            elem->SetAttribute(name.c_str(), value.c_str());
        }
        for(const auto &[name, value]: node.GetConfig().outputPortsMap) {
            // avoid duplicates, in the case of INOUT ports
            if(node.GetConfig().inputPortsMap.count(name) == 0) {
                elem->SetAttribute(name.c_str(), value.c_str());
            }
        }

        for(const auto &[pre, script]: node.GetConfig().preConditions) {
            elem->SetAttribute(ToStr(pre).c_str(), script.c_str());
        }
        for(const auto &[post, script]: node.GetConfig().postConditions) {
            elem->SetAttribute(ToStr(post).c_str(), script.c_str());
        }

        parent_elem->InsertEndChild(elem);

        if(auto control = dynamic_cast<const ControlNode *>(&node)) {
            for(const auto &child: control->Children()) {
                addNode(*child, elem);
            }
        } else if(auto decorator = dynamic_cast<const DecoratorNode *>(&node)) {
            if(decorator->Type() != NodeType::SUBTREE) {
                addNode(*decorator->GetChild(), elem);
            }
        }
    };

    for(const auto &subtree: tree.ptrSubtrees) {
        XMLElement *subtree_elem = doc.NewElement("BehaviorTree");
        subtree_elem->SetAttribute("ID", subtree->treeId.c_str());
        subtree_elem->SetAttribute("_fullpath", subtree->instanceName.c_str());
        rootXML->InsertEndChild(subtree_elem);
        addNode(*subtree->ptrNodes.front(), subtree_elem);
    }

    XMLElement *model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    static const BehaviorTreeFactory temp_factory;

    std::map<std::string, const TreeNodeManifest *> ordered_models;
    for(const auto &[registration_ID, model]: tree.m_ManifestsMap) {
        if(add_builtin_models ||
           !temp_factory.BuiltinNodes().count(registration_ID)) {
            ordered_models.insert({registration_ID, &model});
        }
    }

    for(const auto &[registration_ID, model]: ordered_models) {
        AddNodeModelToJson(*model, doc, model_root);
    }
}

std::string WriteTreeNodesModelJson(
        const BehaviorTreeFactory &factory, bool include_builtin
) {
    XMLDocument doc;

    XMLElement *rootXML = doc.NewElement("root");
    rootXML->SetAttribute("BTCPP_format", "4");
    doc.InsertFirstChild(rootXML);

    XMLElement *model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    std::map<std::string, const TreeNodeManifest *> ordered_models;

    for(const auto &[registration_ID, model]: factory.Manifests()) {
        if(include_builtin ||
           factory.BuiltinNodes().count(registration_ID) == 0) {
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

std::string WriteTreeXSD(const BehaviorTreeFactory &factory) {
    // There are 2 forms of representation for a node:
    // compact: <Sequence .../>  and explicit: <Control ID="Sequence" ... />
    // Only the compact form is supported because the explicit form doesn't
    // make sense with XSD since we would need to allow any attribute.
    // Prepare the data

    std::map<std::string, const TreeNodeManifest *> ordered_models;
    for(const auto &[registration_id, model]: factory.Manifests()) {
        ordered_models.insert({registration_id, &model});
    }

    XMLDocument doc;

    // Add the XML declaration
    XMLDeclaration *declaration = doc.NewDeclaration(
            "xml version=\"1.0\" "
            "encoding=\"UTF-8\""
    );
    doc.InsertFirstChild(declaration);

    // Create the root element with namespace and attributes
    // To validate a BT XML file with `schema.xsd` in the same directory:
    // <root BTCPP_format="4" main_tree_to_execute="MainTree"
    //   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    //   xsi:noNamespaceSchemaLocation="schema.xsd">
    XMLElement *schema_element = doc.NewElement("xs:schema");
    schema_element->SetAttribute(
            "xmlns:xs", "http://www.w3.org/2001/XMLSchema"
    );
    schema_element->SetAttribute("elementFormDefault", "qualified");
    doc.InsertEndChild(schema_element);

    auto parse_and_insert = [&doc](XMLElement *parent_elem, const char *str) {
        XMLDocument tmp_doc;
        tmp_doc.Parse(str);
        if(tmp_doc.Error()) {
            std::cerr << "Internal error parsing existing XML: "
                      << tmp_doc.ErrorStr() << std::endl;
            return;
        }
        for(auto child = tmp_doc.FirstChildElement(); child != nullptr;
            child = child->NextSiblingElement()) {
            parent_elem->InsertEndChild(child->DeepClone(&doc));
        }
    };

    // Common elements.
    XMLComment *comment = doc.NewComment("Define the common elements");
    schema_element->InsertEndChild(comment);

    // TODO: add <xs:whiteSpace value="preserve"/> for `inputPortType` and `outputPortType`.
    parse_and_insert(schema_element, R"(
    <xs:simpleType name="blackboardType">
        <xs:restriction base="xs:string">
            <xs:pattern value="\{.*\}"/>
        </xs:restriction>
    </xs:simpleType>
    <xs:simpleType name="booleanOrBlackboardType">
      <xs:union memberTypes="xs:boolean blackboardType"/>
    </xs:simpleType>
    <xs:simpleType name="integerOrBlackboardType">
      <xs:union memberTypes="xs:integer blackboardType"/>
    </xs:simpleType>
    <xs:simpleType name="decimalOrBlackboardType">
      <xs:union memberTypes="xs:decimal blackboardType"/>
    </xs:simpleType>
    <xs:simpleType name="stringOrBlackboardType">
      <xs:union memberTypes="xs:string blackboardType"/>
    </xs:simpleType>
    <xs:simpleType name="descriptionType">
        <xs:restriction base="xs:string">
          <xs:whiteSpace value="preserve"/>
        </xs:restriction>
    </xs:simpleType>
    <xs:complexType name="inputPortType">
      <xs:simpleContent>
        <xs:extension base="xs:string">
          <xs:attribute name="name" type="xs:string" use="required"/>
          <xs:attribute name="type" type="xs:string" use="optional"/>
          <xs:attribute name="default" type="xs:string" use="optional"/>
        </xs:extension>
      </xs:simpleContent>
    </xs:complexType>
    <xs:complexType name="outputPortType">
      <xs:simpleContent>
        <xs:extension base="xs:string">
          <xs:attribute name="name" type="xs:string" use="required"/>
          <xs:attribute name="type" type="xs:string" use="optional"/>
        </xs:extension>
      </xs:simpleContent>
    </xs:complexType>
    <xs:attributeGroup name="preconditionAttributeGroup">
      <xs:attribute name="_failureIf" type="xs:string" use="optional"/>
      <xs:attribute name="_skipIf" Type="xs:string" use="optional"/>
      <xs:attribute name="_successIf" Type="xs:string" use="optional"/>
      <xs:attribute name="_while" type="xs:string" use="optional"/>
    </xs:attributeGroup>
    <xs:attributeGroup name="postconditionAttributeGroup">
      <xs:attribute name="_onSuccess" type="xs:string" use="optional"/>
      <xs:attribute name="_onFailure" type="xs:string" use="optional"/>
      <xs:attribute name="_post" type="xs:string" use="optional"/>
      <xs:attribute name="_onHalted" type="xs:string" use="optional"/>
    </xs:attributeGroup>)");

    // Common attributes
    // Note that we do not add the `ID` attribute because we do not
    // support the explicit notation (e.g. <Action ID="Saysomething">).
    // Cf. https://www.behaviortree.dev/docs/learn-the-basics/xml_format/#compact-vs-explicit-representation
    // There is no way to check attribute validity with the explicit notation with XSD.
    // The `ID` attribute for `<SubTree>` is handled separately.
    parse_and_insert(schema_element, R"(
    <xs:attributeGroup name="commonAttributeGroup">
      <xs:attribute name="name" Type="xs:string" use="optional"/>
      <xs:attributeGroup ref="preconditionAttributeGroup"/>
      <xs:attributeGroup ref="postconditionAttributeGroup"/>
    </xs:attributeGroup>)");

    // Basic node types
    parse_and_insert(schema_element, R"(
    <xs:complexType name="treeNodesModelNodeType">
      <xs:sequence>
        <xs:choice minOccurs="0" maxOccurs="unbounded">
          <xs:element name="input_port" type="inputPortType"/>
          <xs:element name="output_port" Type="outputPortType"/>
        </xs:choice>
        <xs:element name="description" type="descriptionType" minOccurs="0" maxOccurs="1"/>
      </xs:sequence>
      <xs:attribute name="ID" type="xs:string" use="required"/>
    </xs:complexType>
    <xs:group name="treeNodesModelNodeGroup">
      <xs:choice>
        <xs:element name="Action" Type="treeNodesModelNodeType"/>
        <xs:element name="Condition" type="treeNodesModelNodeType"/>
        <xs:element name="Control" type="treeNodesModelNodeType"/>
        <xs:element name="Decorator" type="treeNodesModelNodeType"/>
      </xs:choice>
    </xs:group>
    )");

    // `root` element
    const auto root_element_xsd = R"(
    <xs:element name="root">
      <xs:complexType>
        <xs:sequence>
          <xs:choice minOccurs="0" maxOccurs="unbounded">
            <xs:element ref="include"/>
            <xs:element ref="BehaviorTree"/>
          </xs:choice>
          <xs:element ref="TreeNodesModel" minOccurs="0" maxOccurs="1"/>
        </xs:sequence>
        <xs:attribute name="BTCPP_format" Type="xs:string" use="required"/>
        <xs:attribute name="main_tree_to_execute" Type="xs:string" use="optional"/>
      </xs:complexType>
    </xs:element>
  )";
    parse_and_insert(schema_element, root_element_xsd);

    // Group definition for a single node of any of the existing node types.
    XMLElement *one_node_group = doc.NewElement("xs:group");
    {
        one_node_group->SetAttribute("name", "oneNodeGroup");
        std::ostringstream xsd;
        xsd << "<xs:choice>";
        for(const auto &[registration_id, model]: ordered_models) {
            xsd << "<xs:element name=\"" << registration_id << "\" Type=\""
                << registration_id << "Type\"/>";
        }
        xsd << "</xs:choice>";
        parse_and_insert(one_node_group, xsd.str().c_str());
        schema_element->InsertEndChild(one_node_group);
    }

    // `include` element
    parse_and_insert(schema_element, R"(
    <xs:element name="include">
      <xs:complexType>
        <xs:attribute name="path" Type="xs:string" use="required"/>
        <xs:attribute name="ros_pkg" Type="xs:string" use="optional"/>
      </xs:complexType>
    </xs:element>
  )");

    // `BehaviorTree` element
    parse_and_insert(schema_element, R"(
  <xs:element name="BehaviorTree">
    <xs:complexType>
      <xs:group ref="oneNodeGroup"/>
      <xs:attribute name="ID" Type="xs:string" use="required"/>
    </xs:complexType>
  </xs:element>
  )");

    // `TreeNodesModel` element
    parse_and_insert(schema_element, R"(
    <xs:element name="TreeNodesModel">
      <xs:complexType>
          <xs:group ref="treeNodesModelNodeGroup" minOccurs="0" maxOccurs="unbounded"/>
      </xs:complexType>
    </xs:element>
  )");

    // Definitions for all node types.
    for(const auto &[registration_id, model]: ordered_models) {
        XMLElement *type = doc.NewElement("xs:complexType");
        type->SetAttribute("name", (model->registrationId + "Type").c_str());
        if((model->type == NodeType::ACTION) or
           (model->type == NodeType::CONDITION) or
           (model->type == NodeType::SUBTREE)) {
            /* No children, nothing to add. */
        } else if(model->type == NodeType::DECORATOR) {
            /* One child. */
            // <xs:group ref="oneNodeGroup" minOccurs="1" maxOccurs="1"/>
            XMLElement *group = doc.NewElement("xs:group");
            group->SetAttribute("ref", "oneNodeGroup");
            group->SetAttribute("minOccurs", "1");
            group->SetAttribute("maxOccurs", "1");
            type->InsertEndChild(group);
        } else {
            /* NodeType::CONTROL. */
            // TODO: check the code, the doc says 1..N but why not 0..N?
            // <xs:group ref="oneNodeGroup" minOccurs="0" maxOccurs="unbounded"/>
            XMLElement *group = doc.NewElement("xs:group");
            group->SetAttribute("ref", "oneNodeGroup");
            group->SetAttribute("minOccurs", "0");
            group->SetAttribute("maxOccurs", "unbounded");
            type->InsertEndChild(group);
        }
        XMLElement *common_attr_group = doc.NewElement("xs:attributeGroup");
        common_attr_group->SetAttribute("ref", "commonAttributeGroup");
        type->InsertEndChild(common_attr_group);
        for(const auto &[port_name, port_info]: model->ports) {
            XMLElement *attr = doc.NewElement("xs:attribute");
            attr->SetAttribute("name", port_name.c_str());
            const auto xsd_attribute_type = xsdAttributeType(port_info);
            if(not xsd_attribute_type.empty()) {
                attr->SetAttribute("Type", xsd_attribute_type.c_str());
            }
            if(not port_info.DefaultValue().Empty()) {
                attr->SetAttribute(
                        "default", port_info.DefaultValueString().c_str()
                );
            } else {
                attr->SetAttribute("use", "required");
            }
            type->InsertEndChild(attr);
        }
        if(model->registrationId == "SubTree") {
            parse_and_insert(type, R"(
        <xs:attribute name="ID" Type="xs:string" use="required"/>
        <xs:anyAttribute processContents="skip"/>
      )");
        }
        schema_element->InsertEndChild(type);
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

std::string WriteTreeToJson(const Tree &tree, bool add_metadata, bool add_builtin_models) {
    XMLDocument doc;

    XMLElement *rootXML = doc.NewElement("root");
    rootXML->SetAttribute("BTCPP_format", 4);
    doc.InsertFirstChild(rootXML);

    AddTreeToJson(tree, doc, rootXML, add_metadata, add_builtin_models);

    XMLPrinter printer;
    doc.Print(&printer);
    return std::string(printer.CStr(), size_t(printer.CStrSize() - 1));
}

}// namespace behaviortree
