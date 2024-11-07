import {createCrawl, createCrawlOpenAI} from "x-crawl";

// 创建爬虫应用
const crawlApp = createCrawl({
    mode: 'async',
});

// 创建 AI 应用
const crawlOpenAIApp = createCrawlOpenAI({
    clientOptions: {apiKey: process.env['OPENAI_API_KEY']},
    defaultModel: {chatModel: 'gpt-4-turbo-preview'}
});

export function CrawlPage(url: string): void {
    crawlApp.crawlPage(url).then(async (res) => {
        const {page, browser} = res.data;

        // 等待元素出现在页面中, 并获取 HTML
        await page.waitForSelector('#wrapper #content .article');
        const targetHTML = await page.$eval(
            '#wrapper #content .article',
            (e) => e.outerHTML
        );

        browser.close();

        // 让 AI 获取电影信息 (描述越详细越好)
        const filmResult = await crawlOpenAIApp.parseElements(
            targetHTML,
            `这是电影列表, 需要获取电影名(name), 封面链接(picture), 简介(info), 评分(score), 
    评论人数(commentsNumber)。使用括号的单词作为属性名`
        );

        console.log(filmResult)
    });
}
