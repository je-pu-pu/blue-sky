// public/shots/ の画像を走査して src/shots.generated.ts を書き出す。
// render 前に実行され、置いた画像が自動でギャラリーに反映される。
import { readdirSync, writeFileSync, existsSync, mkdirSync } from "fs";
import { join, dirname } from "path";
import { fileURLToPath } from "url";

const root = dirname(dirname(fileURLToPath(import.meta.url))); // tools/pr-video
const shotsDir = join(root, "public", "shots");
if (!existsSync(shotsDir)) mkdirSync(shotsDir, { recursive: true });

const files = readdirSync(shotsDir)
  .filter((f) => /\.(png|jpe?g|webp)$/i.test(f))
  .sort();

const out =
  "// 自動生成: scripts/gen_shots.mjs（public/shots/ の画像一覧）。手で編集しない。\n" +
  `export const SHOTS: string[] = ${JSON.stringify(files, null, 2)};\n`;
writeFileSync(join(root, "src", "shots.generated.ts"), out);
console.log(`スクショ: ${files.length} 枚`, files);
