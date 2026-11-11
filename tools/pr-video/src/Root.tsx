import { Composition } from "remotion";
import { ProgressPR, PR_FPS, PR_DURATION } from "./ProgressPR";

export const RemotionRoot: React.FC = () => {
  return (
    <Composition
      id="ProgressPR"
      component={ProgressPR}
      durationInFrames={PR_DURATION}
      fps={PR_FPS}
      width={1920}
      height={1080}
    />
  );
};
