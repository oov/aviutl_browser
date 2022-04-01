# aviutl_browser

AviUtl の拡張編集プラグイン上でブラウザーの表示内容を持ち込めるようにするためのプログラムです。  
使用には bridge.dll ( https://twitter.com/oovch/status/1161359705723195392 ) の導入が必要です。

## 注意事項

aviutl_browser は無保証で提供されます。  
aviutl_browser を使用したこと及び使用しなかったことによるいかなる損害について、開発者は一切の責任を負いません。

これに同意できない場合、あなたは aviutl_browser を使用することができません。  

## インストール方法

exedit.auf と同じ場所にある script フォルダー内に、browser フォルダーを入れればインストール完了です。  
bridge.dll も必要になるので忘れずにインストールしておいてください。

拡張編集上で追加されたカスタムオブジェクトを選ぶと `browser\exe\aviutl_browser.exe` が起動されます。  
このプログラムがウィルス対策ソフトなどでブロックされないようにしてください。

## 表示用コンテンツの作り方

やることの大筋は以下の2ステップです。

作例として `絵文字` / `Markdown` も付属しているので参考にしてください。

### 1. カスタムオブジェクト用のスクリプトを作成する

カスタムオブジェクト用のスクリプトである `*.obj` を `browser` フォルダー内に作成します。  
このスクリプトからブラウザーを呼び出すことになります。  
（最終的にスクリプトが実行できればいいので、呼び出し元はアニメーション効果などでも構いません）

```lua
-- ブラウザーの表示内容を受け取るために透明の画像を用意する
obj.setoption("drawtarget", "tempbuffer", 1920, 1080)
obj.load("tempbuffer")

-- フォルダー指定の場合
local ok, ret = require("browser").execute({
  dir = "mycontent", -- contents フォルダー内のフォルダー名を指定する
  tabid = "", -- 同じコンテンツをブラウザーの別タブで開く場合は識別用の文字列を指定する、必要なければ省略可
  param = "文字列をJavaScriptに渡せます", -- 必要なければ省略可
  userfile = file, -- ファイル参照ダイアログを使う場合
  dev = false, -- 開発モードを有効にする場合は true
});

-- abc ファイル指定の場合
local ok, ret = require("browser").execute({
  abc = "mycontent.abc", -- contents フォルダー内のファイル名を指定する
  tabid = "", -- 同じコンテンツをブラウザーの別タブで開く場合は識別用の文字列を指定する、必要なければ省略可
  param = "文字列をJavaScriptに渡せます", -- 必要なければ省略可
  userfile = file, -- ファイル参照ダイアログを使う場合
  dev = false, -- 開発モードを有効にする場合は true
});

-- 戻り値:
-- ok - 成功した場合は true, 失敗した場合は false
-- ret - 成功した場合は HTML から resolve で渡した文字列
```

開発モードを有効にすると DevTools ウィンドウが表示されます。  
このウィンドウで F5 を押すと HTML のリロードができるため、製作中に AviUtl を再起動しなくても HTML などの変更を反映させられます。

`tabid` は同じコンテンツをブラウザー上で別のタブとして複数保持したい場合に使用できます。  
例えば同じカスタムオブジェクトをレイヤー１とレイヤー２に置いて同時に表示する場合、`tabid` が同じだと毎フレーム `レイヤー１を描画` → `レイヤー２を描画` と交互に呼び出され続けることになるため、描画するたびに表示内容が変わっていることになり効率が悪くなる可能性があります。  
それぞれに別々の `tabid` を割り当てておけば片方のタブはレイヤー１の描画だけ、もう片方のタブはレイヤー２の描画だけを処理すれば良くなります。  
ただし当然別タブに読み込む分のオーバーヘッドがあるので、可能な限り HTML 側で交互に呼び出されてもパフォーマンスが落ちないように準備しておくのが望ましいです。

### 2. ブラウザーで表示するコンテンツを `browser\contents` フォルダー内に作成する

コンテンツのデータ準備方法には、フォルダーを指定する方法と、abc ファイルを指定する方法の２つがあります。  
付属コンテンツでは abc ファイルを指定する方法が使われています。

abc ファイルの実態は「表示用データを詰め込んだ zip ファイルを作成し、拡張子を変更しただけ」のファイルです。  
コンテンツの利用者がファイルの中身を気にしなくてもいい場合は abc ファイルを使うとファイルが散らばりません。  
逆に、適宜 HTML などに手を入れながら使うような使われ方を想定する場合はフォルダー指定のほうが適しています。

ブラウザーが起動されると、フォルダーや abc ファイルのルートにある `index.html` が読み込まれます。  
JavaScript で `AviUtlBrowser.registerRenderer(render)` などとして関数を登録しておくと、`render` 関数が毎回の描画時に呼び出されるようになります。  
これを利用して、トラックバーのデータなどを毎フレーム HTML 内に反映させることでアニメーションなどを行えます。

```html
<!DOCTYPE html>
<meta charset="utf-8">
<p>Hello world <span></span>!</p>
<script>
  // 描画されるたびに呼び出される非同期関数を登録する
  // params.param には前のステップの Lua スクリプト内で渡した「文字列をJavaScriptに渡せます」が入っている
  AviUtlBrowser.registerRenderer(async params => {
    // param の内容を画面に反映
    document.querySelector("span").textContent = params.param;
    return ''; // もし Lua 側に返したいデータがあるなら文字列で渡す
  });
</script>
```

## コンテンツ制作時の注意事項

- 画面左上の1ピクセルは使用できません
  - 内部で画面更新を検知するために、左上の1ピクセルを使用しています  
    画像などを左上ピッタリから表示してしまうと左上の1ピクセルが欠けてしまうので、必要に応じて回避してください
- html タグや body タグへの CSS 適用に注意
  - 更新検知用のドットが描画される位置がずれてしまうと、正しく更新を検知できずに AviUtl がフリーズする可能性があります  
  - `margin` や `padding` などを確保したい場合は `div` などを作ってそこに当ててください
- オンライン上のデータへアクセスするスクリプトは十分に注意して作成すること
  - 毎フレームアクセスするようなことをすると相手に多大な迷惑がかかるので絶対にやらないでください
  - 例えば「岡崎市立中央図書館事件」では1秒に1回のアクセスで逮捕されています
- abc ファイルを作るときはフォルダーごと圧縮しないこと
  - zip ファイルのルートにはフォルダーではなく index.html が来るようにしてください
- ファイル参照ボタンも使えます
  - Lua 側から `userfile = file` で渡すと、HTML 側で `/userfile` で受け取ることができます
  - 使用例: https://gist.github.com/oov/40ac2545d70527f5f2af69b7fa02a1fe
  - 今のところ複数のファイルを渡す仕組みは想定されていません

## FAQ

### Q. AviUtl がフリーズする、動かない

サンプルとして付属しているコンテンツが動かない場合はブラウザーがブロックされずに正しく動作しているか確認してください。  
自作したコンテンツが動かない場合は「コンテンツ制作時の注意事項」の内容を確認してみてください。

例えば JavaScript で `resolve()` を呼び忘れると、ブラウザ側に完了したことが伝えられないのでいつまでも待ち状態になります。  
途中で例外が投げられる場合などにも注意してください。

### Q. DevTools が閉じられない

開発モードが有効なときに開く DevTools は、設計上ウィンドウを直接閉じることはできません。  
（閉じられるようにしてもいいけど1フレーム動かしたらまた出てくるだけなので……）

### Q. 自作の abc ファイルだと index.html が読み込めない

ファイルの直下にはフォルダーではなく index.html が来るようにしてください。

## コンパイル環境の準備方法

Visual Studio 2019 で開発しています。

1. http://opensource.spotify.com/cefbuilds/index.html にアクセスして、Windows 64-bit Builds の Minimal Distribution をダウンロード、展開
2. ルートディレクトリーの `CMakeLists.txt` の最後に `add_subdirectory(aviutl_browser)` を追加
3. ルートディレクトリーで `git clone https://github.com/oov/aviutl_browser && cd aviutl_browser && build.bat`

## Credits

aviutl_browser is made possible by the following open source softwares.  
See also lua/contents/CREDITS.md for content files open source credit list.

### Chromium Embedded Framework

https://bitbucket.org/chromiumembedded/cef

Copyright (c) 2008-2020 Marshall A. Greenblatt. Portions Copyright (c)
2006-2009 Google Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
   * Neither the name of Google Inc. nor the name Chromium Embedded
Framework nor the names of its contributors may be used to endorse
or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

See also LICENSES.chromium.html for Chromium open source credit list.

### PicoJSON

https://github.com/kazuho/picojson

Copyright 2009-2010 Cybozu Labs, Inc.
Copyright 2011-2014 Kazuho Oku
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
