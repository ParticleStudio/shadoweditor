"use strict";

import * as os from "os";
import * as fs from 'fs';
import {TimeUtil, Util} from "./lib/util.js";

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

    const endTime: number = TimeUtil.GetTime();
    console.log("end time:" + endTime.toString() + "   diffTime:" + (endTime - beginTime).toString());
    console.log("------------end-------------");
})(globalThis);
