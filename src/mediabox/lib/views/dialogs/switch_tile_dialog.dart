import 'package:flutter/material.dart';
import 'package:mediabox/utils/i18n.dart';

class SwitchTileDialog<T> extends StatefulWidget {
  const SwitchTileDialog({
    super.key,
    required this.title,
    required this.value,
    required this.buildOptions,
    required this.onSelected,
    required this.onClear,
  });

  final String title;
  final T? value;
  final Map<String, T> buildOptions;
  final Function(T) onSelected;
  final Function onClear;

  @override
  State<SwitchTileDialog<T>> createState() => _SwitchTileDialogState<T>();
}

class _SwitchTileDialogState<T> extends State<SwitchTileDialog<T>> {
  _selectOption(String key) {
    widget.onSelected(widget.buildOptions[key] as T);
    Navigator.pop(context);
  }

  _buildContent() {
    if (widget.value == null) {
      return Text(widget.title);
    }
    return Text(
      widget.buildOptions.keys.firstWhere(
        (key) => widget.buildOptions[key] == widget.value,
        orElse: () => widget.title,
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () {
        showModalBottomSheet(
          context: context,
          showDragHandle: true,
          builder: (context) => ListView(
            shrinkWrap: true,
            children: [
              Padding(
                padding: const EdgeInsets.only(left: 20),
                child: Text(
                  widget.title,
                  style: const TextStyle(
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
              const SizedBox(height: 10),
              for (final entrie in widget.buildOptions.entries)
                RadioListTile(
                  title: Text(entrie.key),
                  value: entrie.value,
                  groupValue: widget.value,
                  onChanged: (value) => _selectOption(entrie.key),
                ),
              Padding(
                padding: const EdgeInsets.all(20),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.end,
                  children: [
                    TextButton(
                      onPressed: () {
                        widget.onClear();
                        Navigator.pop(context);
                      },
                      child: Text("common.clear".i18n),
                    ),
                  ],
                ),
              ),
            ],
          ),
        );
      },
      child: Container(
        color: Theme.of(context).cardColor,
        child: Center(
          child: _buildContent(),
        ),
      ),
    );
  }
}
