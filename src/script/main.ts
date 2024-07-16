"use strict";

import * as os from "os";
import * as std from "std";
import {Util, TimeUtil} from "./lib/util.js";

(function (global: any) {
    console.log("-----------begin------------");

    const beginTime: number = TimeUtil.GetTime();
    console.log("begin time:" + beginTime.toString());

    console.log("os: " + JSON.stringify(os));

    console.log(Util.Add(1, 2).toString());

    console.log("platform: " + os.platform);

    for (let i = 0; i < 1000000; i++) {
        Util.Add(1, 2);
    }

    let file;
    try {
        file = std.open("behaviortree.config.h", "r");
        const fileBuffer: ArrayBuffer = new ArrayBuffer(1024);
        file.read(fileBuffer, 0, 1024);
        console.log("file: " + Util.ArrayBufferToString(fileBuffer));
    } catch (error) {
        console.log("read file error:" + error);
    } finally {
        file.close();
    }


    const endTime: number = TimeUtil.GetTime();
    console.log("end time:" + endTime.toString() + "   diffTime:" + (endTime - beginTime).toString());
    console.log("------------end-------------");
})(globalThis);
