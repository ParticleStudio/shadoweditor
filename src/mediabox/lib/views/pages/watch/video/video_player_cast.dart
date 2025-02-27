import 'package:flutter/material.dart';
import 'package:dlna_dart/dlna.dart';
import 'package:mediabox/utils/i18n.dart';
import 'package:mediabox/utils/log.dart';
import 'package:mediabox/views/widgets/progress.dart';

class VideoPlayerCast extends StatefulWidget {
  const VideoPlayerCast({
    super.key,
    this.onDeviceSelected,
  });

  final Function(DLNADevice device)? onDeviceSelected;

  @override
  State<VideoPlayerCast> createState() => _VideoPlayerCastState();
}

class _VideoPlayerCastState extends State<VideoPlayerCast> {
  late DLNAManager searcher;
  Map<String, DLNADevice> deviceList = {};

  @override
  void initState() {
    super.initState();
    _init();
  }

  _init() async {
    searcher = DLNAManager();
    logger.info('DLNA searching devices...');
    final m = await searcher.start();
    m.devices.stream.listen((deviceList) {
      logger.info('DLNA devices: $deviceList');
      setState(() {
        this.deviceList = deviceList;
      });
    });
  }

  @override
  void dispose() {
    logger.info('DLNA stop searching devices...');
    searcher.stop();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        Padding(
          padding: const EdgeInsets.only(left: 16),
          child: Text(
            'video.cast'.i18n,
            style: const TextStyle(
              fontSize: 18,
            ),
          ),
        ),
        const SizedBox(height: 10),
        for (final device in deviceList.entries)
          ListTile(
            title: Text(device.value.info.friendlyName),
            subtitle: Text(device.key),
            onTap: () async {
              widget.onDeviceSelected?.call(device.value);
            },
          ),
        const Center(
          child: ProgressRing(),
        )
      ],
    );
  }
}
