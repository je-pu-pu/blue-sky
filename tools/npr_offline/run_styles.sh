#!/usr/bin/env bash
# 3スタイルを順に neural 評価する薄いランナー。
# 使い方: ./run_styles.sh [frames_dir] [steps] [max_size] [warm]
#   warm に "warm" を渡すと前フレーム出力で初期化する案A（時間連続化）で評価し、out_<style>_warm へ出力。
set -e
cd "$(dirname "$0")"
PY=.venv/Scripts/python.exe
FRAMES="${1:-../../source/dump}"
STEPS="${2:-150}"
MAXSIZE="${3:-384}"
WARM="${4:-}"

WARM_FLAG=""
OUT_SUFFIX=""
if [ "$WARM" = "warm" ]; then
  WARM_FLAG="--warm-start"
  OUT_SUFFIX="_warm"
fi

for s in brush_starry_night flat_great_wave watercolor; do
  echo "=================== style: $s ${WARM_FLAG} ==================="
  "$PY" run_eval.py --frames "$FRAMES" --stylizer neural \
    --style "styles/$s.jpg" --steps "$STEPS" --max-size "$MAXSIZE" $WARM_FLAG \
    --out "out_$s$OUT_SUFFIX"
done

echo "=== done. metrics summary ==="
"$PY" - <<'PY'
import json, glob, os
for d in sorted(glob.glob("out_*")):
    p = os.path.join(d, "metrics.json")
    if not os.path.exists(p):
        continue
    m = json.load(open(p, encoding="utf-8"))
    print(f"{d:24s} naive={m['temporal_error_naive_mean']:.5f} "
          f"stable={m['temporal_error_stable_mean']:.5f} "
          f"reduction={m['reduction_percent']:.1f}%")
PY
