from locust import HttpUser, task, between


class ProblemSubmitUser(HttpUser):
    wait_time = between(1, 3)

    # 从浏览器复制的完整 cookie 字符串
    cookie_str = "SESSION=MTU2NGQ1NmEtN2Q2OC00NDE0LWI4N2YtY2NkM2UwMTI2ZmQ3"

    # 通用的请求头（除了 Content-Type 由 json 参数自动添加，其他可自定义）
    common_headers = {
        "Accept": "*/*",
        "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
        "Origin": "http://10.22.22.69:8080",
        "Referer": "http://10.22.22.69:8080/problems/problem-detail.html?id=1&return_url=http%3A%2F%2F10.22.22.69%3A8081%2Fhome&username=hash",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
    }

    @task
    def submit_problem(self):
        # 请求体（payload）
        payload = {
            "username": "hash",
            "language": "cpp",
            "code": '#include <bits/stdc++.h>\nusing namespace std;\n\nint main() {\n    ios_base::sync_with_stdio(false);\n    cin.tie(NULL);\n    \n    string str;\n    cin >> str;\n    cout << str;\n    \n    return 0;\n}'
        }

        # 发送 POST 请求
        with self.client.post(
                "/problems/v1/problems/1/submit",  # 注意前导斜杠
                json=payload,  # 自动设置 Content-Type: application/json
                headers=self.common_headers,  # 添加其他请求头
                cookies={"SESSION": self.cookie_str.split("=")[1]}  # 直接传入 cookie 字典
        ) as response:
            # 可选：断言状态码是否为 200
            if response.status_code != 200:
                print(f"Request failed: {response.status_code}, {response.text}")