import React from 'react';
import ReactDOM from 'react-dom/client';
import './index.css';
import App from './App';
import reportWebVitals from './reportWebVitals';

// scripts/prerender.js fills #root with static markup so crawlers get real content
// without running the app. That markup is deliberately NOT hydrated: it is a snapshot
// of the live DOM, and the browser rewrites inline styles when it serialises them
// (#ddd becomes rgb(221, 221, 221), calc(100vh - 100px) becomes calc(-100px + 100vh)).
// React compares those against the strings the components produce, finds them
// different and discards the markup anyway — with a row of errors in the console.
// Rendering over it keeps the same end result, quietly.
const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
    <App className="body"/>
);

// If you want to start measuring performance in your app, pass a function
// to log results (for example: reportWebVitals(console.log))
// or send to an analytics endpoint. Learn more: https://bit.ly/CRA-vitals
reportWebVitals();
