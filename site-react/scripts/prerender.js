/**
 * Pre-renders the SPA into static HTML, one file per route.
 *
 * The site is a Create React App bundle: without this step every URL serves the
 * same empty index.html, and a crawler only sees the page after it executes the
 * JavaScript. Pre-rendering puts the real title, meta tags and body text into
 * the HTML that the server returns.
 *
 * The route list comes from public/sitemap.xml so the two cannot drift apart.
 *
 * Each route is written as build/<route>.html (not <route>/index.html), because a
 * directory would make Apache redirect /xmq_about to /xmq_about/ and that fights
 * with the canonical URLs. See public/.htaccess for the matching rewrite rule.
 *
 * Usage: node scripts/prerender.js
 */

const fs = require("node:fs");
const path = require("node:path");
const http = require("node:http");
const puppeteer = require("puppeteer");

const ROOT = path.resolve(__dirname, "..");
const BUILD_DIR = path.join(ROOT, "build");
const SITEMAP = path.join(ROOT, "public", "sitemap.xml");

// The PHP endpoints are deliberately NOT answered during pre-rendering.
//
// Pages fetch them after mount (visitor counter, news list, download files), so the
// data only exists in the browser. If the static HTML carried it, the markup would
// not match the empty state React starts from on the client, hydration would fail
// and the whole page would be re-rendered from scratch. Failing the request keeps
// the pre-rendered markup identical to the client's first render; the real data
// arrives right after hydration, exactly as it does today.

const MIME_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".gif": "image/gif",
    ".ico": "image/x-icon",
    ".txt": "text/plain; charset=utf-8",
    ".xml": "application/xml",
    ".map": "application/json"
};

// puppeteer downloads its own Chromium when installed, so the build does not depend
// on a browser being present on the machine. CHROME_PATH overrides that for anyone
// who would rather reuse a system install than keep a second copy.
function browserOptions()
{
    const options = {
        headless: true,
        args: ["--no-sandbox", "--disable-setuid-sandbox", "--disable-dev-shm-usage"]
    };
    if (process.env.CHROME_PATH) {
        options.executablePath = process.env.CHROME_PATH;
    }
    return options;
}

function readRoutes()
{
    const sitemap = fs.readFileSync(SITEMAP, "utf8");
    const routes = [];
    for (const match of sitemap.matchAll(/<loc>\s*([^<\s]+)\s*<\/loc>/g)) {
        routes.push(new URL(match[1]).pathname);
    }
    if (routes.length === 0) {
        throw new Error("No <loc> entries found in " + SITEMAP);
    }
    return routes;
}

// Serves the build directory with an SPA fallback. The fallback always returns the
// original index.html captured at start-up, so pages pre-rendered during this run
// are never served as the shell of a later one.
function startServer(shellHtml)
{
    const server = http.createServer(async (req, res) => {
        const url = new URL(req.url, "http://localhost");

        if (url.pathname.endsWith(".php")) {
            res.writeHead(503, {"content-type": "application/json"});
            res.end(JSON.stringify({error: "not available during pre-render"}));
            return;
        }

        const filePath = path.join(BUILD_DIR, url.pathname);
        if (filePath.startsWith(BUILD_DIR) && fs.existsSync(filePath) &&
            fs.statSync(filePath).isFile()) {
            const type = MIME_TYPES[path.extname(filePath)] || "application/octet-stream";
            res.writeHead(200, {"content-type": type});
            res.end(fs.readFileSync(filePath));
            return;
        }

        res.writeHead(200, {"content-type": MIME_TYPES[".html"]});
        res.end(shellHtml);
    });

    return new Promise(resolve => {
        server.listen(0, "127.0.0.1", () => resolve(server));
    });
}

function outputFile(route)
{
    return route === "/" ? path.join(BUILD_DIR, "index.html")
                         : path.join(BUILD_DIR, route.replace(/^\//, "") + ".html");
}

async function main()
{
    if (!fs.existsSync(path.join(BUILD_DIR, "index.html"))) {
        throw new Error("build/index.html not found — run the build first");
    }

    const shellHtml = fs.readFileSync(path.join(BUILD_DIR, "index.html"), "utf8");
    const routes = readRoutes();
    const server = await startServer(shellHtml);
    const base = "http://127.0.0.1:" + server.address().port;

    const browser = await puppeteer.launch(browserOptions());

    // Collected first and written afterwards, so a half-finished run cannot leave
    // the build directory with a mix of old and new pages.
    const rendered = new Map();
    let failed = 0;

    try {
        for (const route of routes) {
            const page = await browser.newPage();
            try {
                await page.setViewport({width: 1280, height: 900});
                await page.goto(base + route, {waitUntil: "networkidle0", timeout: 45000});
                const html = await page.content();
                const title = await page.title();
                rendered.set(route, html);
                console.log("  " + route.padEnd(28) + (html.length / 1024).toFixed(1).padStart(7) +
                            " kB   " + title);
            } catch (error) {
                failed++;
                console.error("  FAILED " + route + ": " + error.message);
            } finally {
                await page.close();
            }
        }
    } finally {
        await browser.close();
        server.close();
    }

    if (failed > 0) {
        throw new Error(failed + " route(s) failed to pre-render; build output left unchanged");
    }

    for (const [route, html] of rendered) {
        const file = outputFile(route);
        fs.mkdirSync(path.dirname(file), {recursive: true});
        fs.writeFileSync(file, html);
    }

    console.log("Pre-rendered " + rendered.size + " routes into build/");
}

main().catch(error => {
    console.error("Pre-render failed: " + error.message);
    process.exit(1);
});
