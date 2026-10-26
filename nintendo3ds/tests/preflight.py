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
    check("large", dim(256, 256), "128x128 tiles")
    check("duplicate", dim(32, 32) * 2, "Invalid DIM")
    check("truncated", b"DIM " + struct.pack("<I", 4) + b"\x20", "past end")
    check("missing", b"VER " + struct.pack("<IH", 2, 205), "128x128 tiles")
    check("no-assets", dim(32, 32), "Patch_rt.mpq")
print("Oversized/duplicate/truncated map rejection and missing-asset recovery PASS")
