# blue-sky 進捗 PR 動画（Remotion）

React で動画をプログラム生成する [Remotion](https://www.remotion.dev/) のプロジェクト。
1920x1080 / 30fps / 約44秒の進捗紹介動画（`ProgressPR` コンポジション）。

## セットアップ

```bash
cd tools/pr-video
npm install
```

（初回レンダー時に Remotion がヘッドレス Chromium を自動取得する）

## 使い方

```bash
npm run dev      # Remotion Studio（ブラウザで編集・プレビュー）
npm run render   # out/progress-pr.mp4 に書き出し
npm run still    # 静止画1枚（out/still.png, 既定 frame=60）
```

任意フレームの静止画:
```bash
npx remotion still ProgressPR out/check.png --frame=460
```

## 構成

`src/ProgressPR.tsx` にシーンをまとめている（`<Series>` で連結）:

1. タイトル
2. 敵AI（知覚・警戒・パトロール・道具・無力化/死体隠し・警報伝播）
3. 多層ナビ（簡易上面図＋合成ビル/階段/橋/オフメッシュ接続/複数ルート）
4. プレイヤーの動詞（ジェット滞空・目標収集）
5. **ゲーム映像スロット**（下記）
6. コミットタイムライン
7. アウトロ（統計）

配色・書体は `src/theme.ts`（ステージ上面図のブループリント調）。

## ゲーム映像の差し込み（ハイブリッド）

初期状態はプレースホルダ。実際のゲーム画面を入れるには:

1. 画面録画を `public/footage/gameplay.mp4` として置く
2. `src/ProgressPR.tsx` の `HAS_FOOTAGE` を `true` にする
3. `npm run render`

スクショを別シーンに使う場合は `public/footage/xxx.png` に置き、`staticFile("footage/xxx.png")` で参照。
（footage 配下の mp4/png/jpg は .gitignore 済み＝各自用意）

## メモ

- Node は v24 で動作確認（公式サポートは 18/20/22。警告が出ても実用可）。
- `node_modules/` と `out/` は .gitignore 済み。
- コミット一覧・統計はセッション時点の値をハードコード。更新は `src/ProgressPR.tsx` の `COMMITS` 配列と `OutroScene` の `stats`。
