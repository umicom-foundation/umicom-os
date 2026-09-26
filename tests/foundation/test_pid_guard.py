"""PID-1 entry must refuse ordinary host execution without side effects."""
import subprocess
import sys
result = subprocess.run([sys.argv[1]], capture_output=True, text=True, timeout=5)
assert result.returncode == 78, (result.returncode, result.stderr)
assert "only runs as PID 1" in result.stderr
print("Host refusal verified; no guest boot was performed.")
