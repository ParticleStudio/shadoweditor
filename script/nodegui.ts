import {ItemDataRole, QAbstractTableModel, QModelIndex, QTableView, QVariant} from "@nodegui/nodegui";

class MyModel extends QAbstractTableModel {
    rowCount(parent = new QModelIndex()): number {
        return 2;
    }

    columnCount(parent = new QModelIndex()): number {
        return 2;
    }

    data(index: QModelIndex, role = ItemDataRole.DisplayRole): QVariant {
        if (role === ItemDataRole.DisplayRole) {
            return new QVariant(`Row${index.row() + 1}, Column${index.column() + 1}`);
        }
        return new QVariant();
    }
}

function nodegui() {
    const tableView = new QTableView();
    const model = new MyModel();
    tableView.setModel(model);
    tableView.show();

    (global as any).win = tableView;
}

(function start(global: any) {
    // Spider.CrawlPage("https://www.baidu.com");
    nodegui();
})(globalThis);
