import {
  AbsoluteFill,
  interpolate,
  spring,
  useCurrentFrame,
  useVideoConfig,
  Series,
  Sequence,
  Audio,
  Img,
  OffthreadVideo,
  staticFile,
} from "remotion";
import { theme } from "./theme";
import { SHOTS } from "./shots.generated"; // public/shots/ の画像一覧（scripts/gen_shots.mjs で自動生成）

export const PR_FPS = 30;

// ゲーム映像(動画)を差し込む場合は public/footage/gameplay.mp4 を置いて true にする。
export const HAS_FOOTAGE = false;

// スクショギャラリーの尺: ショット枚数に応じて伸ばす（1枚 2.6s）。無ければプレースホルダ 4s。
const FOOTAGE_FRAMES = SHOTS.length > 0 ? SHOTS.length * 78 : 120;

// 各シーンの尺（フレーム @30fps）。ナレーション長（VOICEVOX 合成）＋余白に合わせて決定。
const D = {
  title: 160,
  systems: 310,
  nav: 190,
  verbs: 150,
  footage: FOOTAGE_FRAMES,
  commits: 210,
  outro: 130,
};

// 字幕（ナレーションと同内容の読みやすい要約。完全一致ではない）
const SUBTITLES: Record<string, string> = {
  title: "見つからずに進むゲーム — 今回できたこと",
  systems:
    "敵は目と耳で気づき、だんだん警戒して追いかけてくる。花火でおとり、煙で隠れ、後ろから気絶させて隠せる",
  nav: "建物は階段と橋でつながり、屋上まで登れる。行き方はいろいろある",
  verbs: "空を飛べる。光る箱を30個集めてクリア",
  footage: "実際の画面です",
  commits: "作ったもの：屋上へ登る・橋や手すり・いろいろな道・箱集め",
  outro: "今回はここまで。続きもお楽しみに",
};

const Subtitle: React.FC<{ text: string }> = ({ text }) => {
  const frame = useCurrentFrame();
  const { durationInFrames } = useVideoConfig();
  const op = Math.min(
    interpolate(frame, [4, 16], [0, 1], { extrapolateRight: "clamp" }),
    interpolate(frame, [durationInFrames - 12, durationInFrames], [1, 0], {
      extrapolateLeft: "clamp",
    })
  );
  return (
    <AbsoluteFill style={{ justifyContent: "flex-end", alignItems: "center", padding: 56, opacity: op }}>
      <div
        style={{
          maxWidth: 1500,
          textAlign: "center",
          background: "rgba(8,12,16,0.74)",
          border: `1px solid ${theme.line}`,
          borderRadius: 0,
          padding: "16px 30px",
          color: theme.ink,
          fontSize: 33,
          fontWeight: 600,
          lineHeight: 1.5,
          fontFamily: theme.sans,
        }}
      >
        {text}
      </div>
    </AbsoluteFill>
  );
};

// ナレーション付きシーン: シーン本体＋ずんだもんの音声（少し遅らせて開始）＋字幕
const NarratedScene: React.FC<{ name: string; children: React.ReactNode }> = ({
  name,
  children,
}) => (
  <>
    {children}
    <Subtitle text={SUBTITLES[name] ?? ""} />
    <Sequence from={8} name={`vo-${name}`}>
      <Audio src={staticFile(`audio/narration/${name}.wav`)} />
    </Sequence>
  </>
);
export const PR_DURATION =
  D.title + D.systems + D.nav + D.verbs + D.footage + D.commits + D.outro;

// ---- 共通パーツ ----

const GridBg: React.FC = () => {
  const frame = useCurrentFrame();
  const pan = (frame * 0.25) % 48;
  return (
    <AbsoluteFill
      style={{
        backgroundColor: theme.bg,
        backgroundImage: `linear-gradient(${theme.grid} 1px, transparent 1px), linear-gradient(90deg, ${theme.grid} 1px, transparent 1px)`,
        backgroundSize: "48px 48px",
        backgroundPosition: `${pan}px ${pan}px`,
      }}
    />
  );
};

// スクショを表示するシンプルな枠（角丸＋細い枠のみ）。index でショットを選ぶ。
// cycle=true のときはシーン尺内で全ショットを順に切り替える（ギャラリー用）。
const ShotFrame: React.FC<{ index?: number; cycle?: boolean }> = ({
  index = 0,
  cycle = false,
}) => {
  const frame = useCurrentFrame();
  const { durationInFrames } = useVideoConfig();
  if (SHOTS.length === 0) {
    return (
      <div style={{ width: "100%", height: "100%", borderRadius: 0, border: `2px dashed ${theme.line}`, background: theme.panel, display: "flex", justifyContent: "center", alignItems: "center", color: theme.muted, fontFamily: theme.mono, fontSize: 20 }}>
        public/shots/ に画像を置く
      </div>
    );
  }
  let idx = ((index % SHOTS.length) + SHOTS.length) % SHOTS.length;
  let span = durationInFrames;
  let local = frame;
  if (cycle) {
    span = durationInFrames / SHOTS.length;
    idx = Math.min(SHOTS.length - 1, Math.floor(frame / span));
    local = frame - idx * span;
  }
  const zoom = interpolate(local, [0, span], [1.0, 1.05], { extrapolateRight: "clamp" });
  return (
    <div style={{ width: "100%", height: "100%", borderRadius: 0, overflow: "hidden", border: `1px solid ${theme.line}`, background: "#000" }}>
      <Img src={staticFile(`shots/${SHOTS[idx]}`)} style={{ width: "100%", height: "100%", objectFit: "cover", transform: `scale(${zoom})` }} />
    </div>
  );
};

const SceneWrap: React.FC<{ children: React.ReactNode; pad?: number }> = ({
  children,
  pad = 120,
}) => {
  const frame = useCurrentFrame();
  const { durationInFrames } = useVideoConfig();
  const fadeIn = interpolate(frame, [0, 14], [0, 1], { extrapolateRight: "clamp" });
  const fadeOut = interpolate(
    frame,
    [durationInFrames - 14, durationInFrames],
    [1, 0],
    { extrapolateLeft: "clamp" }
  );
  return (
    <AbsoluteFill>
      <GridBg />
      <AbsoluteFill style={{ opacity: Math.min(fadeIn, fadeOut), padding: pad }}>
        {children}
      </AbsoluteFill>
    </AbsoluteFill>
  );
};

const Eyebrow: React.FC<{ children: React.ReactNode }> = ({ children }) => (
  <div
    style={{
      color: theme.accent,
      fontFamily: theme.mono,
      letterSpacing: 2,
      fontSize: 26,
    }}
  >
    {children}
  </div>
);

const Heading: React.FC<{ children: React.ReactNode; delay?: number }> = ({
  children,
  delay = 0,
}) => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const s = spring({ frame: frame - delay, fps, config: { damping: 200 } });
  return (
    <div
      style={{
        color: theme.ink,
        fontSize: 76,
        fontWeight: 700,
        transform: `translateX(${interpolate(s, [0, 1], [-24, 0])}px)`,
        opacity: s,
      }}
    >
      {children}
    </div>
  );
};

// スタッガーで出るチップ（機能ラベル）
const Chip: React.FC<{ label: string; note?: string; index: number; accent?: string }> = ({
  label,
  note,
  index,
  accent = theme.amber,
}) => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const s = spring({
    frame: frame - 24 - index * 7,
    fps,
    config: { damping: 200, mass: 0.6 },
  });
  return (
    <div
      style={{
        transform: `translateY(${interpolate(s, [0, 1], [26, 0])}px)`,
        opacity: s,
        background: theme.panel,
        border: `1px solid ${theme.line}`,
        borderLeft: `4px solid ${accent}`,
        borderRadius: 0,
        padding: "18px 22px",
        display: "flex",
        flexDirection: "column",
        gap: 6,
      }}
    >
      <div style={{ color: theme.ink, fontSize: 30, fontWeight: 600 }}>{label}</div>
      {note ? (
        <div style={{ color: theme.muted, fontSize: 19, fontFamily: theme.mono }}>{note}</div>
      ) : null}
    </div>
  );
};

// ---- シーン ----

const TitleScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const s = spring({ frame, fps, config: { damping: 200 } });
  const lineW = interpolate(s, [0, 1], [0, 420]);
  return (
    <SceneWrap>
      <AbsoluteFill style={{ justifyContent: "center", alignItems: "center", gap: 10 }}>
        <Eyebrow>blue-sky</Eyebrow>
        <div
          style={{
            color: theme.ink,
            fontSize: 96,
            fontWeight: 800,
            transform: `translateY(${interpolate(s, [0, 1], [24, 0])}px)`,
          }}
        >
          見つからずに進むゲーム
        </div>
        <div style={{ height: 3, width: lineW, background: theme.accent, marginTop: 8 }} />
        <div style={{ color: theme.muted, fontSize: 28, marginTop: 14 }}>
          今回できたこと
        </div>
      </AbsoluteFill>
    </SceneWrap>
  );
};

const SystemsScene: React.FC = () => {
  const items = [
    { label: "目と耳で気づく", note: "見える・聞こえる" },
    { label: "だんだん警戒する", note: "気づく→探す→追う" },
    { label: "見回りと追いかけ", note: "道を通って動く" },
    { label: "道具を使う", note: "花火でおとり・煙で隠れる" },
    { label: "気絶させて隠す", note: "後ろから近づく" },
    { label: "仲間に知らせる", note: "警報が広まる" },
  ];
  return (
    <SceneWrap>
      <Eyebrow>敵</Eyebrow>
      <Heading>敵の動き</Heading>
      <div style={{ marginTop: 40, display: "flex", gap: 44, height: 560 }}>
        <div style={{ flex: 1.25, display: "grid", gridTemplateColumns: "1fr 1fr", gap: 18, alignContent: "start" }}>
          {items.map((it, i) => (
            <Chip key={it.label} label={it.label} note={it.note} index={i} />
          ))}
        </div>
        <div style={{ flex: 1 }}>
          <ShotFrame index={1} />
        </div>
      </div>
    </SceneWrap>
  );
};

// 簡易ステージ上面図（ビル＋オフメッシュ接続が描かれていく）
const MiniMap: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const build = (i: number) =>
    spring({ frame: frame - 30 - i * 6, fps, config: { damping: 200 } });
  const link = interpolate(frame, [70, 120], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  // ワールド x[-14,14] z[-2,20] を 620x420 にマップ
  const W = 620, H = 420;
  const px = (x: number) => ((x + 14) / 28) * W;
  const py = (z: number) => ((20 - z) / 22) * H;
  const rects = [
    { n: "A-2F", x0: -13, x1: -9, z0: 10, z1: 14, hi: false },
    { n: "A-3F", x0: -7, x1: -3, z0: 10, z1: 14, hi: true },
    { n: "B-3F", x0: 3, x1: 7, z0: 10, z1: 14, hi: true },
    { n: "B-2F", x0: 9, x1: 13, z0: 10, z1: 14, hi: false },
    { n: "h6", x0: -10, x1: -6, z0: 2, z1: 6, hi: false },
  ];
  const links = [
    [[-11, 18], [-11, 13]],
    [[-10, 12], [-6, 12]],
    [[-4, 12], [4, 12]],
    [[10, 12], [6, 12]],
    [[11, 18], [11, 13]],
    [[-8, -1], [-8, 3]],
  ];
  return (
    <svg width={W} height={H} style={{ background: theme.panel, borderRadius: 0, border: `1px solid ${theme.line}` }}>
      {rects.map((r, i) => {
        const s = build(i);
        return (
          <g key={r.n} opacity={s}>
            <rect
              x={px(r.x0)}
              y={py(r.z1)}
              width={px(r.x1) - px(r.x0)}
              height={py(r.z0) - py(r.z1)}
              fill={r.hi ? "#5b7f9c" : "#3c5163"}
              stroke="rgba(255,255,255,.18)"
            />
            <text
              x={(px(r.x0) + px(r.x1)) / 2}
              y={(py(r.z0) + py(r.z1)) / 2 + 5}
              fill={theme.ink}
              fontSize={15}
              fontFamily={theme.sans}
              textAnchor="middle"
            >
              {r.n}
            </text>
          </g>
        );
      })}
      {links.map((l, i) => {
        const [a, b] = l;
        const x1 = px(a[0]), y1 = py(a[1]), x2 = px(b[0]), y2 = py(b[1]);
        return (
          <line
            key={i}
            x1={x1}
            y1={y1}
            x2={x1 + (x2 - x1) * link}
            y2={y1 + (y2 - y1) * link}
            stroke={theme.accent}
            strokeWidth={3}
            strokeDasharray="4 4"
          />
        );
      })}
    </svg>
  );
};

const NavScene: React.FC = () => {
  const chips = [
    { label: "箱を組んだ建物", note: "" },
    { label: "階段と橋でつながる", note: "上へ行ける" },
    { label: "屋上まで登れる", note: "" },
    { label: "行き方はいろいろ", note: "" },
  ];
  return (
    <SceneWrap>
      <Eyebrow>ステージ</Eyebrow>
      <Heading>登って進む</Heading>
      <div style={{ marginTop: 40, display: "flex", gap: 40, alignItems: "center" }}>
        <MiniMap />
        <div style={{ display: "flex", flexDirection: "column", gap: 18, flex: 1 }}>
          {chips.map((c, i) => (
            <Chip key={c.label} label={c.label} note={c.note} index={i} accent={theme.accent} />
          ))}
        </div>
      </div>
    </SceneWrap>
  );
};

const VerbsScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const big = (i: number) => spring({ frame: frame - 24 - i * 12, fps, config: { damping: 200 } });
  const verbs = [
    { k: "空を飛ぶ", v: "ジャンプ長押しで浮かぶ", accent: theme.amber },
    { k: "箱を集める", v: "光る箱を30個集めてクリア", accent: theme.good },
  ];
  return (
    <SceneWrap>
      <Eyebrow>操作</Eyebrow>
      <Heading>できること</Heading>
      <div style={{ marginTop: 44, display: "flex", gap: 44, height: 480, alignItems: "center" }}>
        <div style={{ flex: 1.1, display: "flex", flexDirection: "column", gap: 40 }}>
          {verbs.map((it, i) => {
            const s = big(i);
            return (
              <div
                key={it.k}
                style={{
                  opacity: s,
                  transform: `translateX(${interpolate(s, [0, 1], [-30, 0])}px)`,
                  display: "flex",
                  flexDirection: "column",
                  gap: 6,
                }}
              >
                <div style={{ color: it.accent, fontSize: 52, fontWeight: 800 }}>{it.k}</div>
                <div style={{ color: theme.muted, fontSize: 26 }}>{it.v}</div>
              </div>
            );
          })}
        </div>
        <div style={{ flex: 1, height: "100%" }}>
          <ShotFrame index={5} />
        </div>
      </div>
    </SceneWrap>
  );
};

const FootageScene: React.FC = () => {
  return (
    <SceneWrap pad={90}>
      {HAS_FOOTAGE ? (
        <div style={{ width: "100%", height: "100%" }}>
          <div style={{ width: "100%", height: "100%", borderRadius: 0, overflow: "hidden", border: `1px solid ${theme.line}`, boxShadow: "0 24px 70px rgba(0,0,0,0.5)" }}>
            <OffthreadVideo src={staticFile("footage/gameplay.mp4")} style={{ width: "100%", height: "100%", objectFit: "cover" }} />
          </div>
        </div>
      ) : (
        <>
          <Eyebrow>画面</Eyebrow>
          <Heading>実際の画面</Heading>
          <div style={{ marginTop: 28, height: 720 }}>
            <ShotFrame cycle />
          </div>
        </>
      )}
    </SceneWrap>
  );
};

const COMMITS = [
  "敵が屋上まで登れるようにした",
  "建物を階段と橋でつないだ",
  "行き方をいくつも用意した",
  "橋に手すり、屋上に隠れ場所を追加",
  "光る箱を集めるゲームにした",
  "空を飛べるようにした",
  "敵の見回りと追いかけを作った",
  "花火でおとり、煙で隠れる道具",
  "後ろから気絶させて隠せる",
  "見つかると仲間に警報が広まる",
];

const CommitsScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { durationInFrames } = useVideoConfig();
  const rowH = 62;
  const listH = COMMITS.length * rowH;
  const viewH = 620;
  const scroll = interpolate(
    frame,
    [30, durationInFrames - 20],
    [0, Math.max(0, listH - viewH)],
    { extrapolateLeft: "clamp", extrapolateRight: "clamp" }
  );
  return (
    <SceneWrap>
      <Eyebrow>記録</Eyebrow>
      <Heading>作ったもの</Heading>
      <div style={{ marginTop: 30, display: "flex", gap: 44, height: viewH }}>
        <div style={{ flex: 1.35, overflow: "hidden" }}>
          <div style={{ transform: `translateY(${-scroll}px)` }}>
            {COMMITS.map((c, i) => {
              const type = theme.good;
              return (
                <div
                  key={i}
                  style={{
                    height: rowH,
                    display: "flex",
                    alignItems: "center",
                    gap: 18,
                    borderBottom: `1px solid ${theme.grid}`,
                  }}
                >
                  <div style={{ width: 10, height: 10, borderRadius: 0, background: type, flex: "none" }} />
                  <div style={{ color: theme.ink, fontSize: 26 }}>{c}</div>
                </div>
              );
            })}
          </div>
        </div>
        <div style={{ flex: 1 }}>
          <ShotFrame index={3} />
        </div>
      </div>
    </SceneWrap>
  );
};

const OutroScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const s = spring({ frame, fps, config: { damping: 200 } });
  const sub = spring({ frame: frame - 12, fps, config: { damping: 200 } });
  return (
    <SceneWrap>
      <AbsoluteFill style={{ justifyContent: "center", alignItems: "center", gap: 24 }}>
        <div style={{ color: theme.ink, fontSize: 84, fontWeight: 800, opacity: s }}>今回はここまで</div>
        <div style={{ color: theme.muted, fontSize: 30, opacity: sub, transform: `translateY(${interpolate(sub, [0, 1], [16, 0])}px)` }}>
          続きもお楽しみに
        </div>
        <div style={{ color: theme.accent, fontSize: 24, fontFamily: theme.mono, marginTop: 12, letterSpacing: 2, opacity: sub }}>blue-sky</div>
      </AbsoluteFill>
    </SceneWrap>
  );
};

// ---- 本編 ----

export const ProgressPR: React.FC = () => {
  return (
    <AbsoluteFill style={{ backgroundColor: theme.bg, fontFamily: theme.sans }}>
      {/* BGM（全編・低音量でナレーションを邪魔しない） */}
      <Audio src={staticFile("audio/bgm.mp3")} volume={0.22} />

      <Series>
        <Series.Sequence durationInFrames={D.title}>
          <NarratedScene name="title">
            <TitleScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.systems}>
          <NarratedScene name="systems">
            <SystemsScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.nav}>
          <NarratedScene name="nav">
            <NavScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.verbs}>
          <NarratedScene name="verbs">
            <VerbsScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.footage}>
          <NarratedScene name="footage">
            <FootageScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.commits}>
          <NarratedScene name="commits">
            <CommitsScene />
          </NarratedScene>
        </Series.Sequence>
        <Series.Sequence durationInFrames={D.outro}>
          <NarratedScene name="outro">
            <OutroScene />
          </NarratedScene>
        </Series.Sequence>
      </Series>
    </AbsoluteFill>
  );
};
