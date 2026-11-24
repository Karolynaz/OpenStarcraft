"""Exercise real engine load failures before expensive asset initialization."""
from pathlib import Path
import struct
import subprocess
import tempfile

exe = Path("build/host-sim").resolve()
with tempfile.TemporaryDirectory(prefix="openbw-preflight-") as directory:
    root = Path(directory)
    def check(name, contents, expected):
        path = root / (name + ".chk")
        path.write_bytes(contents)
        result = subprocess.run([str(exe), str(root), str(path)], capture_output=True, text=True)
        assert result.returncode == 1, result
        assert expected in result.stderr, result.stderr
    dim = lambda w, h: b"DIM " + struct.pack("<IHH", 4, w, h)
    bounds = "between 32 and 256 tiles"
    check("too-wide", dim(257, 256), bounds)
    check("too-tall", dim(256, 257), bounds)
    check("too-small", dim(31, 32), bounds)
    check("duplicate", dim(32, 32) * 2, "Invalid DIM")
    check("truncated", b"DIM " + struct.pack("<I", 4) + b"\x20", "past end")
    check("missing", b"VER " + struct.pack("<IH", 2, 205), bounds)
    check("max-dimensions", dim(256, 256), "Patch_rt.mpq")
    check("no-assets", dim(32, 32), "Patch_rt.mpq")
print("Oversized/duplicate/truncated map rejection and missing-asset recovery PASS")
