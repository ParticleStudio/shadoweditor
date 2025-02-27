import 'package:get/get.dart';
import 'package:mediabox/models/extension_setting.dart';
import 'package:mediabox/data/services/database_service.dart';
import 'package:mediabox/utils/extension.dart';
import 'package:mediabox/data/services/extension_service.dart';

class ExtensionSettingsPageController extends GetxController {
  ExtensionSettingsPageController(this.package);

  final String package;

  final Rx<ExtensionService?> runtime = Rx(null);

  final List<ExtensionSetting> settings = <ExtensionSetting>[].obs;

  @override
  void onInit() {
    onRefresh();
    super.onInit();
  }

  onRefresh() async {
    runtime.value = ExtensionUtils.runtimes[package];
    settings.clear();
    settings.addAll(await DatabaseService.getExtensionSettings(package));
  }
}
