#!/bin/bash
mkdir -p bin dist/script/browser/contents dist/script/browser/exe

# copy documents
sed 's/\r$//' README.md | sed 's/lua\/contents\/CREDITS.md/script\/browser\/contents\/CREDITS.txt/' | sed 's/$/\r/' > dist/aviutl_browser.txt
sed 's/\r$//' LICENSES.chromium.html | sed 's/$/\r/' > dist/LICENSES.chromium.html
sed 's/\r$//' lua/contents/CREDITS.md | sed 's/$/\r/' > dist/script/browser/contents/CREDITS.txt

# copy scripts
sed 's/\r$//' lua/browser.lua | sed 's/$/\r/' > dist/script/browser/browser.lua
sed 's/\r$//' lua/glTF表示デモ.obj | sed 's/$/\r/' > dist/script/browser/glTF表示デモ.obj
sed 's/\r$//' lua/Markdown.anm | sed 's/$/\r/' > dist/script/browser/Markdown.anm
sed 's/\r$//' lua/Markdown.exa | sed 's/$/\r/' > dist/script/browser/Markdown.exa
sed 's/\r$//' lua/SVGアニメ.obj | sed 's/$/\r/' > dist/script/browser/SVGアニメ.obj
sed 's/\r$//' lua/絵文字.obj | sed 's/$/\r/' > dist/script/browser/絵文字.obj

# copy contents
cp lua/contents/glTF表示デモ.abc dist/script/browser/contents/
cp lua/contents/Markdown.abc dist/script/browser/contents/
cp lua/contents/SVGアニメ.abc dist/script/browser/contents/
cp lua/contents/絵文字.abc dist/script/browser/contents/
cp -r lua/contents/絵文字セレクター dist/script/browser/contents/

# copy program
cp -r ../build/aviutl_browser/Release/* dist/script/browser/exe/

# pack
cd dist
zip -r ../bin/aviutl_browser_wip.zip *
