# Changelog

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
