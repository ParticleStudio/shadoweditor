import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:mediabox/models/extension.dart';
import 'package:mediabox/controllers/home_controller.dart';
import 'package:mediabox/views/widgets/home/home_favorites.dart';
import 'package:mediabox/views/widgets/home/home_recent.dart';
import 'package:mediabox/utils/i18n.dart';
import 'package:mediabox/views/widgets/platform_widget.dart';

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  late HomePageController c;

  @override
  void initState() {
    c = Get.put(HomePageController());
    super.initState();
  }

  Widget _buildContent() {
    return Obx(
      () {
        if (c.resents.isEmpty &&
            c.favorites.values.every((element) => element.isEmpty)) {
          return Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Text(
                  "（＞人＜；）",
                  style: TextStyle(
                    fontSize: 50,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(height: 20),
                Text(
                  "home.no-record".i18n,
                ),
              ],
            ),
          );
        }

        return SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (c.resents.isNotEmpty) ...[
                  HomeRecent(
                    data: c.resents,
                  ),
                  const SizedBox(height: 16),
                ],
                if (c.favorites.isNotEmpty) ...[
                  HomeFavorites(
                    type: ExtensionType.bangumi,
                    data: c.favorites[ExtensionType.bangumi]!,
                  ),
                  HomeFavorites(
                    type: ExtensionType.manga,
                    data: c.favorites[ExtensionType.manga]!,
                  ),
                  HomeFavorites(
                    type: ExtensionType.fikushon,
                    data: c.favorites[ExtensionType.fikushon]!,
                  ),
                ]
              ],
            ),
          ),
        );
      },
    );
  }

  Widget _buildAndroidHome(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("common.home".i18n),
      ),
      body: _buildContent(),
    );
  }

  Widget _buildDesktopHome(BuildContext context) {
    return _buildContent();
  }

  @override
  Widget build(BuildContext context) {
    return PlatformBuildWidget(
      androidBuilder: _buildAndroidHome,
      desktopBuilder: _buildDesktopHome,
    );
  }
}
