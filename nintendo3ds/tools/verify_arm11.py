"""Reject a final executable whose linked code advertises a newer ISA."""
import subprocess
import sys

attributes = subprocess.run([sys.argv[1], "-A", sys.argv[2]], check=True, capture_output=True, text=True).stdout
if "Tag_CPU_arch: v6K\n" not in attributes or "Tag_FP_arch: VFPv2\n" not in attributes:
    raise SystemExit("Expected ARMv6K/VFPv2 in final ELF attributes:\n" + attributes)
if "Thumb-2" in attributes or "Tag_Advanced_SIMD_arch" in attributes:
    raise SystemExit("Unsupported Thumb-2/NEON in final ELF attributes:\n" + attributes)
print("Final ELF architecture: ARMv6K / VFPv2 / no Thumb-2 or NEON PASS")
