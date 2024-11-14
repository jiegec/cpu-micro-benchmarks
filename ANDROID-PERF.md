# Use perf counters on Android

There are different ways to access perf counters on Android:

1. On root-ed device, you can access PMU via root user
2. Execute microbenchmarks using `adb shell`

You need to copy executables to Android using `adb push`. But beware that some partitions are mounted as noexec e.g. `/storage/emulated/0/`.

If you find it hard to find a target directory for `adb push`, you can:

1. Run `sshd` in Termux to launch a SSH server
2. Use `scp` to copy program to home directory under termux
3. Run `run-as com.termux` to enter Termux data directory under `adb shell`
4. Run `cd files/home` and run programs there using perf counters

You can use `simpleperf` from NDK via `adb shell` in the same way. You can also run `usr/bin/sshd` from Termux in `adb shell` instead of running in Termux app.
