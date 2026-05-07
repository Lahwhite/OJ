package org.example.users.constants;

import java.nio.file.Path;

public class SandboxConstant {
    public static final int MAX_CONCURRENT_RUNS = 5;

    public static final int TIMEOUT_DEFAULT = 4;
    public static final int TIMEOUT_JAVA = 10;

    public static final int PIDS_LIMIT_DEFAULT = 10;
    public static final int PIDS_LIMIT_JAVA = 25;

    public static final int MAX_CODE_LENGTH = 16 * 1024;
    public static final int MAX_OUTPUT_BYTES = 64 * 1024;

    public static final String MEMORY_LIMIT = "128m";
    public static final String CPU_CORES_LIMIT = "0.5";
    public static final String TEMP_DIR = Path.of(System.getProperty("java.io.tmpdir"), "sandbox").toString();
    public static final String DISK_LIMIT = "64m";
    public static final String NETWORK_MODE = "none";
}
