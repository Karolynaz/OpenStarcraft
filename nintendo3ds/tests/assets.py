"""Optional tests using the user's private MPQs and a small two-player map."""
import argparse
import csv
import io
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--data", type=Path, required=True)
parser.add_argument("--map", type=Path, required=True)
parser.add_argument("--ticks", type=int, default=2400)
parser.add_argument("--output", type=Path)
args = parser.parse_args()
exe = Path(__file__).resolve().parents[1] / "build/host-sim"
rows = []
for scenario in ("empty", "two", "move", "melee"):
    result = subprocess.run([str(exe), str(args.data), str(args.map), scenario, str(args.ticks)],
                            check=True, capture_output=True, text=True)
    print(result.stdout, end="")
    lines = result.stdout.splitlines()
    header = next(i for i, line in enumerate(lines) if line.startswith("platform,map_width"))
    row = next(csv.DictReader(io.StringIO("\n".join(lines[header:header + 2]))))
    assert int(row["frame"]) == args.ticks, row
    expected = {"empty": 0, "two": 2, "move": 2}
    if scenario in expected:
        assert int(row["units"]) == expected[scenario], row
    else:
        assert int(row["units"]) >= 10, row
    rows.append({"scenario": scenario, **row})
result = subprocess.run([str(exe), str(args.data), str(args.map), "commands"],
                        check=True, capture_output=True, text=True)
print(result.stdout, end="")
if args.output:
    with args.output.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
print(f"Private-asset initialization / {args.ticks}-tick scenarios / movement / economy / production / combat PASS")
