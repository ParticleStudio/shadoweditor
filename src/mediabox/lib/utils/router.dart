import 'dart:io';

import 'package:get/get.dart';
import 'package:mediabox/router/router.dart';

class RouterUtils {
  static pop() {
    if (Platform.isAndroid) {
      return Get.back();
    }
    return router.pop();
  }
}
