import * as os from "os";
import * as https from "https"
import fs from "fs";
import * as cheerio from "cheerio";
import {string} from "@tensorflow/tfjs-node";
import * as axios from "axios";

function DownloadImages(url: string) {
    https.get(url, function (res) {
        res.setEncoding('binary');

        let data = "";
        res.on("data", function (chunk) {
            data += chunk;
        });
        res.on("end", function () {
            if (!fs.existsSync("./images")) {
                fs.mkdirSync("./images");
            }

            const $ = cheerio.load(data);
            const resultList = [];
            $()

            fs.writeFile('images/' + Math.random() + '.png', data, 'binary', function (err) {  //以二进制格式保存
                if (err) throw err;
                console.log('保存成功');
            });
        });
    }).on("error", function (err) {
        console.log("Error: ", err.message);
    })
}

class StealData {
    private base_url: string;
    private current_page: number;
    private result_list: any[];

    // private axiosObj: any;

    constructor(url: string) {
        this.base_url = url; //要爬取的网站
        this.current_page = 1;
        this.result_list = [];
        // this.axiosObj = axios.create({
        //     httpsAgent: new https.Agent({
        //         rejectUnauthorized: false
        //     })
        // });
        // axios.defaults.headers['Connection'] = 'keep-alive';
    }

    async init() {
        try {
            await this.getPageData();
            await this.downLoadPictures();
        } catch (e) {
            console.log(e);
        }
    }

    sleep(time: number) {
        return new Promise((resolve) => {
            console.log(`自动睡眠中，${time / 1000}秒后重新发送请求......`);
            setTimeout(() => {
                resolve();
            }, time);
        });
    }

    async getPageData() {
        const target_url = this.base_url;
        try {
            const res = await axios.get(target_url);
            const html = res.data;
            const $ = cheerio.load(html);
            const result_list = [];
            $(".mmComponent_images_1").each((index, element) => {
                console.log(index);
                result_list.push({
                    title: $(element).find('.newsintroduction').text(),
                    down_loda_url: $(element).find('img').attr('src').split('!')[0],
                });
            });
            this.result_list.push(...result_list);
            console.log(result_list);
            return Promise.resolve(result_list);
        } catch (e) {
            console.log('获取数据失败');
            return Promise.reject(e);
        }
    }

    async downLoadPictures() {
        const result_list = this.result_list;
        try {
            for (let i = 0, len = result_list.length; i < len; i++) {
                console.log(`开始下载第${i + 1}张图片!`);
                await this.downLoadPicture(result_list[i].down_loda_url);
                await this.sleep(3000 * Math.random());
                console.log(`第${i + 1}张图片下载成功!`);
            }
            return Promise.resolve();
        } catch (e) {
            console.log('写入数据失败');
            return Promise.reject(e)
        }
    }

    async downLoadPicture(href: string) {
        try {
            const target_path = path.resolve(__dirname, `./image/${href.split('/').pop()}`);
            const response = await axios.get(href, {responseType: 'stream'});
            await response.data.pipe(fs.createWriteStream(target_path));
            console.log('写入成功');
            return Promise.resolve();
        } catch (e) {
            console.log('写入数据失败');
            return Promise.reject(e)
        }
    }

}


function main() {
    // DownloadImages("https://cn.bing.com/images/search?q=%E5%B8%A6%E7%83%9F%E5%9B%BE%E7%89%87&go=%E6%90%9C%E7%B4%A2&qs=ds&form=QBIR&first=1");

    const thief = new StealData("https://cn.bing.com/images/search?q=%E5%B8%A6%E7%83%9F%E5%9B%BE%E7%89%87&go=%E6%90%9C%E7%B4%A2&qs=ds&form=QBIR&first=1");
    thief.init();
}

main();
