# Changelog

## 0.4.2

- CEF を 129.0.12+gf09539f+chromium-129.0.6668.101 に更新

## 0.4.1

- CEF を 99.2.15+g71e9523+chromium-99.0.4844.84 に更新

## 0.4.0

- CEF を 99.2.14+g3f796b8+chromium-99.0.4844.84 に更新
- `.wasm` を `application/wasm` で返すように MIME の定義を追加
- `glTF表示デモ.obj` と `SVGアニメ.obj` と `ファイル参照デモ.obj` を削除
  - 元々機能のデモンストレーション目的で含めていましたが、一般的な実用性はないため削除しました
- 配布用パッケージを自動生成するようにした

## 0.3

- `tabid` パラメーターを追加した
  - 同じコンテンツをブラウザー上の別のタブに読み込むための識別用パラメーター
  - 可能な限りコンテンツ側で対応すべきなので、積極的に使う必要はありません
- コンテンツを取得するときに一部のヘッダーを追加
  - すべてのファイルに対して `Cross-Origin-Embedder-Policy: require-corp` と `Cross-Origin-Opener-Policy: same-origin` を追加
  - `/userfile` に対して `Content-Security-Policy: default-src 'self'` と `Cache-Control: no-store` を追加
- CEF を 90.6.7+g19ba721+chromium-90.0.4430.212 に更新

## 0.2

- 一部のファイルが配布ファイルから漏れていたのを修正

## 0.1

- 初版
