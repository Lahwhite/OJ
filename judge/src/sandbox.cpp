#include "sandbox.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>

#else
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <cerrno>
#include <chrono>
#endif

Sandbox::Sandbox(const SandboxConfig& config) : config_(config) {
}

// 预留接口：Windows 下资源限制尚未实现
bool Sandbox::setupResourceLimits() {
    return true;
}

// 预留接口：进程隔离环境尚未实现
bool Sandbox::createIsolatedEnvironment() {
    return true;
}

#ifdef _WIN32

// Windows 沙箱：通过管道重定向 stdin/stdout/stderr，使用作业对象确保超时后彻底终止进程树
SandboxResult Sandbox::execute(const std::string& command, const std::string& input) {
    SandboxResult result;
    result.exit_code = -1;
    result.runtime_ms = 0;
    result.memory_kb = 0;
    result.timeout = false;
    result.memory_exceeded = false;

    // 1. 创建作业对象，用于超时后终止 cmd + 所有子进程
    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    if (!hJob) {
        result.error = "Failed to create job object";
        return result;
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        result.error = "Failed to configure job object";
        CloseHandle(hJob);
        return result;
    }

    // 2. 创建三组管道：标准输出、标准错误、标准输入
    HANDLE hOutputRead, hOutputWrite;
    HANDLE hErrorRead, hErrorWrite;
    HANDLE hInputRead, hInputWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&hOutputRead, &hOutputWrite, &saAttr, 0)) {
        result.error = "Failed to create output pipe";
        CloseHandle(hJob);
        return result;
    }
    if (!CreatePipe(&hErrorRead, &hErrorWrite, &saAttr, 0)) {
        result.error = "Failed to create error pipe";
        CloseHandle(hOutputRead); CloseHandle(hOutputWrite);
        CloseHandle(hJob);
        return result;
    }
    if (!CreatePipe(&hInputRead, &hInputWrite, &saAttr, 0)) {
        result.error = "Failed to create input pipe";
        CloseHandle(hOutputRead); CloseHandle(hOutputWrite);
        CloseHandle(hErrorRead);  CloseHandle(hErrorWrite);
        CloseHandle(hJob);
        return result;
    }

    // 3. 绑定子进程标准流到管道
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hOutputWrite;
    si.hStdError  = hErrorWrite;
    si.hStdInput  = hInputRead;
    si.dwFlags   |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    // 经 cmd.exe 执行，兼容复杂命令行
    std::string full_command = "cmd.exe /c " + command;
    std::vector<char> cmd_buf(full_command.begin(), full_command.end());
    cmd_buf.push_back('\0');

    // 4. 以挂起模式创建进程，确保在它产生任何子进程前放入作业
    BOOL bSuccess = CreateProcessA(
        nullptr, cmd_buf.data(), nullptr, nullptr, TRUE,
        CREATE_SUSPENDED,
        nullptr, nullptr, &si, &pi);

    if (!bSuccess) {
        DWORD error_code = GetLastError();
        char error_msg[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            error_msg, sizeof(error_msg), nullptr);
        result.error = "Failed to create process: " + std::string(error_msg);
        CloseHandle(hOutputRead); CloseHandle(hOutputWrite);
        CloseHandle(hErrorRead);  CloseHandle(hErrorWrite);
        CloseHandle(hInputRead);  CloseHandle(hInputWrite);
        CloseHandle(hJob);
        return result;
    }

    // 5. 将 cmd.exe 进程加入作业对象
    if (!AssignProcessToJobObject(hJob, pi.hProcess)) {
        result.error = "Failed to assign process to job object";
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        CloseHandle(hOutputRead); CloseHandle(hOutputWrite);
        CloseHandle(hErrorRead);  CloseHandle(hErrorWrite);
        CloseHandle(hInputRead);  CloseHandle(hInputWrite);
        CloseHandle(hJob);
        return result;
    }

    // 恢复进程运行
    ResumeThread(pi.hThread);

    // 6. 关闭父进程侧不需要的管道端，防止死锁
    CloseHandle(hOutputWrite);
    CloseHandle(hErrorWrite);
    CloseHandle(hInputRead);

    // 7. 向子进程 stdin 写入测试输入
    if (!input.empty()) {
        size_t total = 0;
        while (total < input.size()) {
            DWORD dwWritten = 0;
            const DWORD to_write = static_cast<DWORD>(input.size() - total);
            if (!WriteFile(hInputWrite, input.data() + total, to_write, &dwWritten, nullptr) || dwWritten == 0) {
                break;
            }
            total += dwWritten;
        }
    }
    CloseHandle(hInputWrite);

    // 8. 等待进程结束或超时
    DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, static_cast<DWORD>(config_.time_limit_ms));
    if (dwWaitResult == WAIT_TIMEOUT) {
        // 超时：终止整个作业（cmd.exe + java.exe 等子进程）
        TerminateJobObject(hJob, 1);
        result.timeout = true;
    }

    // 获取退出码
    DWORD dwExitCode;
    if (GetExitCodeProcess(pi.hProcess, &dwExitCode))
        result.exit_code = static_cast<int>(dwExitCode);

    // 9. 读取 stdout / stderr —— 作业终止后管道写端已关闭，ReadFile 会正常读到 EOF
    char buffer[4096];
    DWORD dwRead;
    while (ReadFile(hOutputRead, buffer, sizeof(buffer), &dwRead, nullptr) && dwRead > 0) {
        result.output.append(buffer, dwRead);
    }
    while (ReadFile(hErrorRead, buffer, sizeof(buffer), &dwRead, nullptr) && dwRead > 0) {
        result.error.append(buffer, dwRead);
    }

    // 10. 清理所有句柄
    CloseHandle(hOutputRead);
    CloseHandle(hErrorRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hJob);   // 关闭作业句柄（KILL_ON_CLOSE 提供双保险）

    return result;
}

#else

// Linux 沙箱：fork + 管道 + setrlimit 限制内存，轮询检测超时
SandboxResult Sandbox::execute(const std::string& command, const std::string& input) {
    SandboxResult result;
    result.exit_code = -1;
    result.runtime_ms = 0;
    result.memory_kb = 0;
    result.timeout = false;
    result.memory_exceeded = false;

    int stdout_pipe[2], stderr_pipe[2], stdin_pipe[2];
    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0 || pipe(stdin_pipe) != 0) {
        result.error = "Failed to create pipes: " + std::string(strerror(errno));
        return result;
    }

    const auto start_time = std::chrono::steady_clock::now();

    // fork 子进程执行用户程序
    pid_t pid = fork();
    if (pid < 0) {
        result.error = "fork() failed: " + std::string(strerror(errno));
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        close(stdin_pipe[0]);  close(stdin_pipe[1]);
        return result;
    }

    if (pid == 0) {
        // 子进程：重定向标准流到管道
        dup2(stdin_pipe[0],  STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);  close(stdin_pipe[1]);
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);

        // 设置虚拟内存上限（RLIMIT_AS）
        if (config_.memory_limit_mb > 0) {
            struct rlimit rl;
            rl.rlim_cur = static_cast<rlim_t>(config_.memory_limit_mb) * 1024 * 1024;
            rl.rlim_max = rl.rlim_cur;
            setrlimit(RLIMIT_AS, &rl);
        }

        // 通过 shell 执行命令字符串
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127);
    }

    // 父进程：关闭子进程侧管道端
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    if (!input.empty()) {
        size_t total = 0;
        while (total < input.size()) {
            ssize_t n = write(stdin_pipe[1], input.data() + total, input.size() - total);
            if (n <= 0) break;
            total += static_cast<size_t>(n);
        }
    }
    close(stdin_pipe[1]);

    // 非阻塞读管道，便于与超时检测配合轮询
    auto setNonBlocking = [](int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags != -1) fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    };
    setNonBlocking(stdout_pipe[0]);
    setNonBlocking(stderr_pipe[0]);

    bool timed_out = false;
    const int time_limit_ms = config_.time_limit_ms;

    // 轮询读取输出并检查子进程是否退出
    while (true) {
        char buf[4096];
        ssize_t n;
        while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0) {
            result.output.append(buf, static_cast<size_t>(n));
        }
        while ((n = read(stderr_pipe[0], buf, sizeof(buf))) > 0) {
            result.error.append(buf, static_cast<size_t>(n));
        }

        int status = 0;
        pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) {
            if (WIFEXITED(status)) {
                result.exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.exit_code = -WTERMSIG(status);
            }
            break;
        }

        const auto elapsed = std::chrono::steady_clock::now() - start_time;
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        // 超出时限：发送 SIGKILL 并标记 timeout
        if (elapsed_ms >= time_limit_ms) {
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            result.timeout = true;
            result.exit_code = -1;
            break;
        }

        usleep(5000);
    }

    // 进程退出后再排空管道中剩余数据
    {
        char buf[4096];
        ssize_t n;
        while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0) {
            result.output.append(buf, static_cast<size_t>(n));
        }
        while ((n = read(stderr_pipe[0], buf, sizeof(buf))) > 0) {
            result.error.append(buf, static_cast<size_t>(n));
        }
    }

    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    // 记录实际运行耗时（Linux 路径由父进程计时）
    const auto elapsed = std::chrono::steady_clock::now() - start_time;
    result.runtime_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());

    result.memory_kb = 0;  // 当前实现未采集内存用量

    if (result.exit_code == 127) {
        result.error = "Command not found or failed to execute: " + command + "\n" + result.error;
    }

    return result;
}

#endif // _WIN32
