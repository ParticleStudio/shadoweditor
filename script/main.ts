"use strict";

import * as os from "os";
import * as std from "std";
import {Util, TimeUtil} from "./lib/util.js";

(function start(global: any) {
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
        do {
            console.log(file.getline());
        } while (!file.eof());
    } catch (error) {
        console.log("read file error:" + error);
    } finally {
        file.close();
    }

    // try {
    //     file = std.open("openid.csv", "w");
    //     file.puts("key\n");
    //     let openIds:string = "";
    //     for (let i = 100000000; i < 201000000; i++) {
    //         openIds += i.toString() + "\n";
    //     }
    //     file.puts(openIds);
    // } catch (error) {
    //     console.log("write file error:" + error);
    // } finally {
    //     file.close();
    // }

    const endTime: number = TimeUtil.GetTime();
    console.log("end time:" + endTime.toString() + "   diffTime:" + (endTime - beginTime).toString());
    console.log("------------end-------------");
})(globalThis);
