import {useEffect} from "react";

const SITE_URL = "https://www.sptk.net";

// Creates the tag if it isn't in index.html yet, then sets its content
function setMeta(selector, attribute, name, content)
{
    let tag = document.head.querySelector(selector);
    if (!tag) {
        tag = document.createElement("meta");
        tag.setAttribute(attribute, name);
        document.head.appendChild(tag);
    }
    tag.setAttribute("content", content);
}

function setCanonical(url)
{
    let link = document.head.querySelector("link[rel='canonical']");
    if (!link) {
        link = document.createElement("link");
        link.setAttribute("rel", "canonical");
        document.head.appendChild(link);
    }
    link.setAttribute("href", url);
}

/**
 * @brief Per-page SEO tags
 * Props:
 * - title: page title, also used as og:title
 * - description: meta description, also used as og:description
 * - keywords: optional comma-separated keyword list
 * - path: page path, e.g. "/xmq_about", used to build the canonical URL
 *
 * The site is a single-page app, so every route is served the same index.html.
 * Without this component all pages would share one title and one description in
 * search results. Rendering it inside a page overwrites those tags on navigation.
 */
export default function Seo({title, description, keywords, path})
{
    useEffect(() => {
        const url = SITE_URL + (path || window.location.pathname);

        document.title = title;
        setMeta("meta[name='description']", "name", "description", description);
        if (keywords) {
            setMeta("meta[name='keywords']", "name", "keywords", keywords);
        }
        setCanonical(url);
        setMeta("meta[property='og:title']", "property", "og:title", title);
        setMeta("meta[property='og:description']", "property", "og:description", description);
        setMeta("meta[property='og:url']", "property", "og:url", url);
        setMeta("meta[name='twitter:title']", "name", "twitter:title", title);
        setMeta("meta[name='twitter:description']", "name", "twitter:description", description);
    }, [title, description, keywords, path]);

    return null;
}
