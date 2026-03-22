const express = require("express");
const multer = require("multer");
const path = require("path");
const fs = require("fs");
const pdfParse = require("pdf-parse");
const mammoth = require("mammoth");
const WordExtractor = require("word-extractor");

const APP_ROOT = __dirname;
const UPLOAD_DIR = path.join(APP_ROOT, "uploads");
fs.mkdirSync(UPLOAD_DIR, { recursive: true });

const app = express();
app.use(express.json({ limit: "1mb" }));
app.use(express.static(APP_ROOT));
app.use("/uploads", express.static(UPLOAD_DIR));

const ALLOWED_EXTENSIONS = new Set([".txt", ".csv", ".pdf", ".doc", ".docx"]);

const storage = multer.diskStorage({
    destination: (_req, _file, cb) => cb(null, UPLOAD_DIR),
    filename: (_req, file, cb) => {
        const ext = path.extname(file.originalname || "").toLowerCase();
        const safeBase = path.basename(file.originalname, ext).replace(/[^a-zA-Z0-9_-]/g, "_");
        cb(null, `${Date.now()}-${safeBase}${ext}`);
    }
});

const upload = multer({
    storage,
    limits: { fileSize: 50 * 1024 * 1024 },
    fileFilter: (_req, file, cb) => {
        const ext = path.extname(file.originalname || "").toLowerCase();
        if (!ALLOWED_EXTENSIONS.has(ext)) {
            cb(new Error("Unsupported file type. Use txt, csv, pdf, doc or docx."));
            return;
        }
        cb(null, true);
    }
});

let currentDocument = null;
let trieRoot = { children: Object.create(null), end: false };
let wordFreq = Object.create(null);

function normalizeToken(s) {
    return (s || "").toLowerCase().replace(/[^a-z0-9]/g, "");
}

function resetTrie() {
    trieRoot = { children: Object.create(null), end: false };
    wordFreq = Object.create(null);
}

function insertWord(word) {
    if (!word) return;
    let cur = trieRoot;
    for (const ch of word) {
        if (!cur.children[ch]) {
            cur.children[ch] = { children: Object.create(null), end: false };
        }
        cur = cur.children[ch];
    }
    cur.end = true;
    wordFreq[word] = (wordFreq[word] || 0) + 1;
}

function buildAutocompleteIndex(text) {
    resetTrie();
    const words = text.toLowerCase().split(/[^a-z0-9]+/g);
    for (const raw of words) {
        const w = normalizeToken(raw);
        if (w.length >= 2) insertWord(w);
    }
}

function collectFromNode(node, prefix, out, limit) {
    if (out.length >= limit) return;
    if (node.end) out.push(prefix);
    if (out.length >= limit) return;

    const keys = Object.keys(node.children).sort();
    for (const ch of keys) {
        collectFromNode(node.children[ch], prefix + ch, out, limit);
        if (out.length >= limit) return;
    }
}

function getSuggestions(prefix, limit = 8) {
    const norm = normalizeToken(prefix);
    if (!norm) return [];

    let cur = trieRoot;
    for (const ch of norm) {
        if (!cur.children[ch]) return [];
        cur = cur.children[ch];
    }

    const words = [];
    collectFromNode(cur, norm, words, Math.max(limit, 20));

    words.sort((a, b) => {
        const fa = wordFreq[a] || 0;
        const fb = wordFreq[b] || 0;
        if (fb !== fa) return fb - fa;
        return a.localeCompare(b);
    });

    return words.slice(0, limit);
}

function buildLineStarts(text) {
    const starts = [0];
    for (let i = 0; i < text.length; i++) {
        if (text[i] === "\n") starts.push(i + 1);
    }
    return starts;
}

function offsetToLineCol(offset, lineStarts) {
    let left = 0;
    let right = lineStarts.length - 1;

    while (left <= right) {
        const mid = Math.floor((left + right) / 2);
        if (lineStarts[mid] <= offset) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    const lineIndex = Math.max(0, right);
    return {
        line: lineIndex + 1,
        column: offset - lineStarts[lineIndex] + 1
    };
}

function escapeRegex(s) {
    return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function escapeHtml(s) {
    return s
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/\"/g, "&quot;")
        .replace(/'/g, "&#39;");
}

function renderDocumentTextHtml(title, bodyHtml, subtitle) {
        return `<!doctype html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>${escapeHtml(title)}</title>
    <style>
        body { font-family: Segoe UI, sans-serif; margin: 16px; color: #1c2430; }
        .muted { color: #5a6878; margin: 0 0 10px; }
        .doc { border: 1px solid #d6dfeb; border-radius: 8px; background: #fbfdff; padding: 12px; white-space: pre-wrap; line-height: 1.5; }
        mark { background: #ffe58f; padding: 0 2px; }
    </style>
</head>
<body>
    <h3>${escapeHtml(title)}</h3>
    <p class="muted">${escapeHtml(subtitle)}</p>
    <div class="doc">${bodyHtml}</div>
</body>
</html>`;
}

async function extractTextFromFile(filePath, ext) {
    if (ext === ".txt" || ext === ".csv") {
        return fs.readFileSync(filePath, "utf8");
    }

    if (ext === ".pdf") {
        const buffer = fs.readFileSync(filePath);
        const parsed = await pdfParse(buffer);
        return parsed.text || "";
    }

    if (ext === ".docx") {
        const parsed = await mammoth.extractRawText({ path: filePath });
        return parsed.value || "";
    }

    if (ext === ".doc") {
        const extractor = new WordExtractor();
        const parsed = await extractor.extract(filePath);
        return parsed.getBody() || "";
    }

    return "";
}

function searchInDocument(query, maxReturn = 300) {
    if (!currentDocument) {
        return { totalMatches: 0, matches: [] };
    }

    const text = currentDocument.text;
    const lineStarts = currentDocument.lineStarts;
    const re = new RegExp(escapeRegex(query), "ig");

    let m;
    let total = 0;
    const matches = [];

    while ((m = re.exec(text)) !== null) {
        const index = m.index;
        total++;

        if (matches.length < maxReturn) {
            const loc = offsetToLineCol(index, lineStarts);
            const snippetStart = Math.max(0, index - 60);
            const snippetEnd = Math.min(text.length, index + query.length + 60);
            const snippet = text.slice(snippetStart, snippetEnd).replace(/\s+/g, " ").trim();

            matches.push({
                offset: index,
                line: loc.line,
                column: loc.column,
                snippet
            });
        }

        if (m.index === re.lastIndex) re.lastIndex++;
    }

    return { totalMatches: total, matches };
}

app.post("/upload", upload.single("file"), async (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).json({ message: "No file uploaded" });
        }

        const ext = path.extname(req.file.originalname || "").toLowerCase();
        const text = (await extractTextFromFile(req.file.path, ext)).replace(/\r\n/g, "\n").replace(/\r/g, "\n");

        if (!text.trim()) {
            if (fs.existsSync(req.file.path)) fs.unlinkSync(req.file.path);
            return res.status(400).json({ message: "Could not extract readable text from this file." });
        }

        if (currentDocument && currentDocument.path && fs.existsSync(currentDocument.path)) {
            fs.unlinkSync(currentDocument.path);
        }

        currentDocument = {
            path: req.file.path,
            webPath: `/uploads/${path.basename(req.file.path)}`,
            originalName: req.file.originalname,
            ext,
            mimeType: req.file.mimetype,
            size: req.file.size,
            text,
            lineStarts: buildLineStarts(text)
        };

        buildAutocompleteIndex(text);

        res.json({
            message: "File uploaded and indexed successfully",
            file: {
                name: currentDocument.originalName,
                type: currentDocument.ext,
                size: currentDocument.size,
                openUrl: currentDocument.webPath
            }
        });
    } catch (err) {
        res.status(500).json({ message: err.message || "Upload failed" });
    }
});

app.post("/search", async (req, res) => {
    const query = (req.body.query || "").trim();

    if (!currentDocument) {
        return res.status(400).json({ message: "Upload a document first" });
    }
    if (!query) {
        return res.status(400).json({ message: "Query is required" });
    }
    if (query.length > 200) {
        return res.status(400).json({ message: "Query is too long" });
    }

    const result = searchInDocument(query, 300);

    res.json({
        query,
        totalMatches: result.totalMatches,
        shownMatches: result.matches.length,
        file: {
            name: currentDocument.originalName,
            openUrl: currentDocument.webPath
        }
    });
});

app.get("/suggest", (req, res) => {
    if (!currentDocument) {
        return res.json({ suggestions: [] });
    }

    const q = (req.query.q || "").toString();
    const suggestions = getSuggestions(q, 8);
    res.json({ suggestions });
});

app.get("/document-view", (req, res) => {
    const query = (req.query.q || "").toString().trim();

    if (!currentDocument) {
        res.status(400).send("No document uploaded.");
        return;
    }

    const maxChars = 300000;
    let text = currentDocument.text;
    let truncated = false;
    if (text.length > maxChars) {
        text = text.slice(0, maxChars);
        truncated = true;
    }

    let htmlText = escapeHtml(text);
    if (query) {
        const re = new RegExp(escapeRegex(query), "ig");
        htmlText = htmlText.replace(re, (m) => `<mark>${m}</mark>`);
    }

    const subtitle = truncated
        ? `${currentDocument.originalName} (large file preview truncated to first ${maxChars} characters)`
        : currentDocument.originalName;

    res.send(renderDocumentTextHtml("Document Viewer", htmlText, subtitle));
});

app.get("/health", (_req, res) => {
    res.json({ ok: true, hasDocument: Boolean(currentDocument) });
});

app.use((err, _req, res, _next) => {
    if (err && err.message) {
        res.status(400).json({ message: err.message });
        return;
    }
    res.status(500).json({ message: "Unexpected error" });
});

app.listen(3000, () => console.log("Server running on http://localhost:3000"));