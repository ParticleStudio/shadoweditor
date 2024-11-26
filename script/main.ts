"use strict";

// import * as os from "os";
// import * as std from "std";
import * as axios from "axios";
import {Util, TimeUtil} from "./lib/util";
import * as Spider from "./spider/image/x-crawl";

function log(target: ClassA, context: ClassMethodDecoratorContext): ClassA {
    console.log("this is " + target);
    return target;
}

function test(): void {
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
}

@log
class ClassA {
    public name: string;
    public age: number;

    constructor(name: string, age: number) {
        this.name = name;
        this.age = age;
    }

    public GetName(): string {
        return this.name;
    }

    public GetAge(): number {
        return this.age;
    }
}

function test2(callback: Function): void {
    callback(10);
}

function Delay2(): Promise<number> {
    return new Promise((resolve, reject) => {
        test2((value: number) => {
            resolve(value);
        });
    });
}

function Delay(ms: number): Promise<number> {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            resolve(ms);
        }, ms);
    });
}

async function GetDataAsync(): Promise<string> {
    console.log("开始获取数据");

    const response: axios.AxiosResponse = await axios.default.get("https://www.bing.com");
    console.log("异步获取的数据: " + response.data);

    const delayTime: number = await Delay(2000);
    console.log("delayTime:" + delayTime);

    const delayTime2: number = await Delay2();
    console.log("delayTime2:" + delayTime2);

    return "模拟的数据";
}

(function start(global: any) {
    const classA: ClassA = new ClassA("zhangsan", 18);
    console.log(classA.GetName());
    console.log(classA.GetAge());

    GetDataAsync().then((data: string): void => {
        console.log("then " + data);
    });
})(globalThis);
