"use strict";

import * as os from "os";
import {TimeUtil, Util} from "./lib/util.js";

(function (global: any) {
    console.log("-----------begin------------");

    console.log("os: " + JSON.stringify(os));

    console.log(Util.Add(1, 2).toString());
    console.log(TimeUtil.GetTime().toString());
    console.log("platform: " + os.platform);

    console.log("------------end-------------");
})(globalThis);
