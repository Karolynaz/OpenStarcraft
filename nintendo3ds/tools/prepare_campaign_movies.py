#!/usr/bin/env python3
"""Convert the original English StarCraft CD cinematics for private 3DS builds."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

# The CD has no internal movie listfile. Verify each block before assigning it.
MOVIES = (
    (12, "fall-of-fenix", "1ee13b708e9aa8122b166dbff02069606e3dc4d3f9c97db2a905837d90df409f"),
    (13, "ambush", "87d7023cd41fe175f74875c1795ed38819710c36f07d4e7f9563d0ff01f184aa"),
    (14, "return-to-aiur", "88cf41a4e4fc878f925c808d0a92a13521ae3ecc69ab18f92c02204095f7252d"),
    (15, "death-of-the-overmind", "b050023a8571ce3288f7cab6b0cecb5a5ddef76bd6637a78420e059a93c5dae3"),
    (17, "wasteland-patrol", "7d8055db4bc0af8c4127d670c708432706ff4fa9997413fa7760e82c3ef9339e"),
    (18, "open-rebellion", "7682adc9ab89f9159211c2f140b2b4b0b3b99daaf520dcad0b387c50e97a8c22"),
    (19, "norad-ii", "3137c1c86b675a57a43c8e0807e832ad83e98324d3ab9ac8b712762300dbd470"),
    (20, "inauguration", "0e224bd26b0d4ebf72f4457799e8251be8238ceba557f7b7dbb0f3c0c9c6ab83"),
    (22, "dream", "44c0a3250927b57f0f4e23243a1ac82d12f817c23b6c049b3e34f96b785cdafb"),
    (23, "amerigo", "8e9fbf92e49bdeebbd9a61d127c7d124a280133c2b072d466e20748131a1df73"),
    (24, "warp", "5cc6925eecda9b45d20e4ef87f606111e260d2cde120700ee6cc1469cc0c603c"),
    (25, "invasion-of-aiur", "02b010f2d7a2a50eb504526c71103932d6e2b82486ef781820509200307e8bc2"),
)

def prepare(extractor, archive, stage):
    output = stage / "campaign" / "movies"
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=output) as directory:
        work = Path(directory)
        for block, name, digest in MOVIES:
            source = work / "movie.smk"
            subprocess.run([extractor.resolve(), archive.resolve(), str(block), source], check=True)
            with source.open("rb") as stream:
                if hashlib.file_digest(stream, "sha256").hexdigest() != digest:
                    raise ValueError(f"Unexpected CD edition or movie at block {block}")
            video, audio = work / f"{name}.bin", work / f"{name}1.pcm"
            subprocess.run([
                "ffmpeg", "-v", "error", "-xerror", "-y", "-i", source,
                "-map", "0:v:0", "-vf", "fps=12,scale=256:128:flags=lanczos",
                "-pix_fmt", "rgb565le", "-f", "rawvideo", video,
                "-map", "0:a:0", "-af", "apad=pad_dur=1", "-ar", "22050",
                "-ac", "2", "-f", "s16le", audio,
            ], check=True)
            frames, remainder = divmod(video.stat().st_size, 256 * 128 * 2)
            if not 0 < frames <= 12 * 600 or remainder:
                raise ValueError(f"Invalid movie frame count: {name}")
            video.replace(output / video.name)
            audio.replace(output / audio.name)
            print(f"{name}: {frames} frames", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("extractor", "archive", "stage"):
        parser.add_argument(name, type=Path)
    prepare(**vars(parser.parse_args()))
