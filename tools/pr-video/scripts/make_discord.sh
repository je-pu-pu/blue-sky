#!/usr/bin/env bash
# 動画を Discord 投稿用に 10MB 以内へ 2パス変換する。
# 使い方: bash scripts/make_discord.sh [入力.mp4] [出力.mp4]
set -e

IN="${1:-out/progress-pr.mp4}"
OUT="${2:-out/progress-pr-discord.mp4}"
TARGET_MB=9.3          # 10MB 未満に収める安全マージン
AUDIO_K=96             # 音声ビットレート(kbps)
FF="npx remotion ffmpeg"

# 長さ(秒)を ffprobe から取得。HH:MM:SS.ss を文字列で int()/float() パース
# （$H を直接 python コードに埋めると 00/01 が「先頭ゼロの整数リテラル」で構文エラーになる）
dline=$(npx remotion ffprobe "$IN" 2>&1 | grep -oE "Duration: [0-9]+:[0-9]+:[0-9.]+" | head -1)
t="${dline#Duration: }"
dur=$(python -c "h,m,s='$t'.split(':'); print(int(h)*3600+int(m)*60+float(s))")

# 動画ビットレート(kbps) = 目標(kbit)/尺 - 音声
vk=$(python -c "print(max(300, int($TARGET_MB*8192/$dur - $AUDIO_K)))")
echo "長さ=${dur}秒  目標=${TARGET_MB}MB  映像=${vk}k  音声=${AUDIO_K}k"

# 2パス（1パス目は破棄、2パス目で出力）。faststart で先頭に moov を置く。
$FF -y -i "$IN" -c:v libx264 -preset slow -b:v ${vk}k -pass 1 -an -f null - 2>/dev/null
$FF -y -i "$IN" -c:v libx264 -preset slow -b:v ${vk}k -pass 2 \
    -c:a aac -b:a ${AUDIO_K}k -movflags +faststart "$OUT" 2>/dev/null

rm -f ffmpeg2pass-0.log ffmpeg2pass-0.log.mbtree
mb=$(python -c "import os;print(round(os.path.getsize('$OUT')/1048576,2))")
echo "出力=$OUT  大きさ=${mb} MB"
