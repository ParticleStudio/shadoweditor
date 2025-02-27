import 'package:get/get.dart';
import 'package:mediabox/utils/extension.dart';
import 'package:mediabox/data/services/extension_service.dart';

class ExtensionPageController extends GetxController {
  RxMap<String, ExtensionService> runtimes = <String, ExtensionService>{}.obs;
  RxMap<String, String> errors = <String, String>{}.obs;
  RxBool isInstallloading = false.obs;
  bool needRefresh = true;
  bool isPageOpen = false;

  @override
  void onInit() {
    onRefresh();
    super.onInit();
  }

  onRefresh() async {
    runtimes.clear();
    errors.clear();
    runtimes.addAll(ExtensionUtils.runtimes);
    errors.addAll(ExtensionUtils.extensionErrorMap);
  }

  callRefresh() {
    if (isPageOpen) {
      onRefresh();
    } else {
      needRefresh = true;
    }
  }
}
