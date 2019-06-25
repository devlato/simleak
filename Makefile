BUILD_DIR=target/build
EXEC_PATH="${BUILD_DIR}/simleak"

rm_build_dir:
	rm -rf "${BUILD_DIR}"

create_build_dir:
	mkdir -p "${BUILD_DIR}"

reset_build: rm_build_dir create_build_dir

build: reset_build
	g++ -v -std=c++17 -g main.cpp -o "${EXEC_PATH}"

run: build
	bash -c "${EXEC_PATH} ${RUN_ARGS}"

run_verbose: build
	bash -c "${EXEC_PATH} ${RUN_ARGS} -v"
