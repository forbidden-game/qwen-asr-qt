# Third-Party Backend Source

The Qt app talks to `qwen_asr_server` over HTTP. The backend source is expected
to come from the pure-C qwen-asr project.

Supported development layouts:

1. Keep qwen-asr outside this repository and configure CMake with:

   ```sh
   cmake -S . -B build -DQWEN_ASR_BACKEND_SOURCE_DIR=/path/to/qwen-asr
   ```

2. Add qwen-asr here as a submodule or subtree at:

   ```text
   third_party/qwen-asr/
   ```

When CMake sees a backend source tree, it runs `make server` there and copies
`qwen_asr_server` next to the Qt executable.
