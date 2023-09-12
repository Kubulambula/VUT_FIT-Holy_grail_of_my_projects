#!/usr/bin/python3
import asyncio
import aiohttp


async def get_url(url):
    try:
        async with aiohttp.ClientSession() as session:
            async with session.get(url) as response:
                return (response.status, url)
    except aiohttp.ClientError:
        return ("aiohttp.ClientError", url)


async def get_urls(urls):
    tasks = [asyncio.create_task(get_url(url)) for url in urls]
    return await asyncio.gather(*tasks)


if __name__ == '__main__':
    urls = ['https://www.fit.vutbr.cz', 'https://www.szn.cz', 'https://www.alza.cz', 'https://office.com', 'https://aukro.cz']

    # for MS Windows
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    res = asyncio.run(get_urls(urls))
    print(res)
