import os
import subprocess
import pytest


@pytest.fixture
def run_lox():
    def run(code: str) -> str:
        lox_path = os.environ.get("LOX_BINARY", "./src/cpplox")
        result = subprocess.run(
            [lox_path, "-c", code, "--stress-gc", "--gc-initial-collection-threshold", "1024"],
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    return run
