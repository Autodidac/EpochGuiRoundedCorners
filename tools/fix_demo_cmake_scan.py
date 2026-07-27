from pathlib import Path

cmake_path = Path(__file__).resolve().parents[1] / "CMakeLists.txt"
cmake = cmake_path.read_text(encoding="utf-8")
old = "set(CMAKE_CXX_SCAN_FOR_MODULES ON)"
if old not in cmake:
    raise RuntimeError("Demo module-scan setting was not found")
cmake_path.write_text(cmake.replace(old, "set(CMAKE_CXX_SCAN_FOR_MODULES OFF)", 1), encoding="utf-8")
