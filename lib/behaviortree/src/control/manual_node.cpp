module behaviortree.manual_node;

//
//#include <ncurses.h>
//
//#include "behaviortree/action_node.h"
//
//namespace behaviortree {
//ManualSelectorNode::ManualSelectorNode(const std::string& refName, const NodeConfig& refConfig)
//    : ControlNode::ControlNode(refName, refConfig), m_RunningChildIdx(-1), m_PreviouslyExecutedIdx(-1) {
//    SetRegistrationId("ManualSelector");
//}
//
//void ManualSelectorNode::Halt() {
//    if(m_RunningChildIdx >= 0) {
//        HaltChild(size_t(m_RunningChildIdx));
//    }
//    m_RunningChildIdx = -1;
//    ControlNode::Halt();
//}
//
//NodeStatus ManualSelectorNode::Tick() {
//    const size_t childrenCount = m_ChildrenNodesVec.size();
//
//    if(childrenCount == 0) {
//        return SelectNodeStatus();
//    }
//
//    bool repeatLast{false};
//    GetInput(REPEAT_LAST_SELECTION, repeatLast);
//
//    int idx = 0;
//
//    if(repeatLast && m_PreviouslyExecutedIdx >= 0) {
//        idx = m_PreviouslyExecutedIdx;
//    } else {
//        SetNodeStatus(NodeStatus::RUNNING);
//        idx = SelectChild();
//        m_PreviouslyExecutedIdx = idx;
//
//        if(idx == NUM_SUCCESS) {
//            return NodeStatus::SUCCESS;
//        }
//        if(idx == NUM_FAILURE) {
//            return NodeStatus::FAILURE;
//        }
//        if(idx == NUM_RUNNING) {
//            return NodeStatus::RUNNING;
//        }
//    }
//
//    NodeStatus ret = m_ChildrenNodesVec[idx]->ExecuteTick();
//    if(ret == NodeStatus::RUNNING) {
//        m_RunningChildIdx = idx;
//    }
//    return ret;
//}
//
//NodeStatus ManualSelectorNode::SelectNodeStatus() const {
//    WINDOW* win;
//    initscr();
//    cbreak();
//
//    win = newwin(6, 70, 1, 1);// create a new window
//
//    mvwprintw(win, 0, 0, "No children.");
//    mvwprintw(win, 1, 0, "Press: S to return SUCCESSFUL,");
//    mvwprintw(win, 2, 0, "       F to return FAILURE, or");
//    mvwprintw(win, 3, 0, "       R to return RUNNING.");
//
//    wrefresh(win);    // update the terminal screen
//    noecho();         // disable echoing of characters on the screen
//    keypad(win, TRUE);// enable keyboard input for the window.
//    curs_set(0);      // hide the default screen cursor.
//
//    int ch = 0;
//    NodeStatus ret;
//    while(1) {
//        if(ch == 's' || ch == 'S') {
//            ret = NodeStatus::SUCCESS;
//            break;
//        } else if(ch == 'f' || ch == 'F') {
//            ret = NodeStatus::FAILURE;
//            break;
//        } else if(ch == 'r' || ch == 'R') {
//            ret = NodeStatus::RUNNING;
//            break;
//        }
//        ch = wgetch(win);
//    }
//    werase(win);
//    wrefresh(win);
//    delwin(win);
//    endwin();
//    return ret;
//}
//
//uint8_t ManualSelectorNode::SelectChild() const {
//    const size_t childrenCount = m_ChildrenNodesVec.size();
//
//    std::vector<std::string> list;
//    list.reserve(childrenCount);
//    for(const auto& refChild: m_ChildrenNodesVec) {
//        list.push_back(refChild->GetNodeName());
//    }
//
//    size_t width = 10;
//    for(const auto& refStr: list) {
//        width = std::max(width, refStr.size() + 2);
//    }
//
//    WINDOW* win;
//    initscr();
//    cbreak();
//
//    win = newwin(childrenCount + 6, 70, 1, 1);// create a new window
//
//    mvwprintw(win, 0, 0, "Use UP/DOWN arrow to select the child, Enter to confirm.");
//    mvwprintw(win, 1, 0, "Press: S to skip and return SUCCESSFUL,");
//    mvwprintw(win, 2, 0, "       F to skip and return FAILURE, or");
//    mvwprintw(win, 3, 0, "       R to skip and return RUNNING.");
//
//    // now print all the menu items and highlight the first one
//    for(size_t i = 0; i < list.size(); i++) {
//        mvwprintw(win, i + 5, 0, "%2ld. %s", i + 1, list[i].c_str());
//    }
//
//    wrefresh(win);    // update the terminal screen
//    noecho();         // disable echoing of characters on the screen
//    keypad(win, TRUE);// enable keyboard input for the window.
//    curs_set(0);      // hide the default screen cursor.
//
//    uint8_t row = 0;
//    int ch = 0;
//    while(1) {
//        // right pad with spaces to make the items appear with even width.
//        wattroff(win, A_STANDOUT);
//        mvwprintw(win, row + 5, 4, "%s", list[row].c_str());
//        // use a variable to increment or decrement the value based on the input.
//        if(ch == KEY_DOWN) {
//            row = (row == childrenCount - 1) ? 0 : row + 1;
//        } else if(ch == KEY_UP) {
//            row = (row == 0) ? (childrenCount - 1) : row - 1;
//        } else if(ch == KEY_ENTER || ch == 10) {
//            break;
//        } else if(ch == 's' || ch == 'S') {
//            row = NUM_SUCCESS;
//            break;
//        } else if(ch == 'f' || ch == 'F') {
//            row = NUM_FAILURE;
//            break;
//        } else if(ch == 'r' || ch == 'R') {
//            row = NUM_RUNNING;
//            break;
//        }
//
//        // now highlight the next item in the list.
//        wattron(win, A_STANDOUT);
//        mvwprintw(win, row + 5, 4, "%s", list[row].c_str());
//        ch = wgetch(win);
//    }
//
//    werase(win);
//    wrefresh(win);
//    delwin(win);
//    endwin();
//    return row;
//}
//
//}// namespace behaviortree

// module behaviortree.manual_node;
