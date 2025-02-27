import 'dart:io';

import 'package:flutter/material.dart';
import 'package:mediabox/router/router.dart';
import 'package:mediabox/views/pages/search/search_page.dart';
import 'package:mediabox/utils/i18n.dart';
import 'package:mediabox/data/providers/anilist_provider.dart';
import 'package:mediabox/views/widgets/horizontal_list.dart';
import 'package:mediabox/views/widgets/grid_item_tile.dart';
import 'package:get/get.dart';
import 'package:mediabox/controllers/search_controller.dart';
import 'package:mediabox/views/pages/tracking/anilist_more_page.dart';

class AnilistHorizontalList extends StatefulWidget {
  const AnilistHorizontalList({
    super.key,
    required this.anilistType,
    required this.data,
  });

  final AnilistType anilistType;
  final Map<dynamic, dynamic> data;

  @override
  State<AnilistHorizontalList> createState() => _AnilistHorizontalListState();
}

class _AnilistHorizontalListState extends State<AnilistHorizontalList> {
  @override
  Widget build(BuildContext context) {
    final data = widget.data;
    final type = widget.anilistType;
    final count = data["CURRENT"]?.length ?? 0;

    return HorizontalList(
      title: (type == AnilistType.anime)
          ? "common.anime".i18n
          : "common.manga".i18n,
      itemBuilder: (context, index) {
        final itemData = data["CURRENT"][index];

        final title = itemData["media"]["title"]["userPreferred"];
        final cover = itemData["media"]["coverImage"]["large"];

        return GridItemTile(
          onTap: () {
            if (Platform.isAndroid) {
              Get.to(() => const SearchPage());
            } else {
              router.push("/search");
            }
            final c = Get.put(SearchPageController());
            c.search.value = title;
          },
          title: title,
          cover: cover,
        );
      },
      itemCount: count,
      onClickMore: () {
        if (Platform.isAndroid) {
          Get.to(
            () => AnilistMorePage(
              anilistType: type,
            ),
          );
        } else {
          router.push(Uri(
            path: '/settings/anilist_more',
            queryParameters: {
              'type': type.index.toString(),
            },
          ).toString());
        }
      },
    );
  }
}
