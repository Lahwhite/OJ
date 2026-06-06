package org.example.users.utils;

import java.net.*;
import java.util.Enumeration;

public class IpAddressUtil {

    public static String getLocalIpv4() {
        try {
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
            String fallbackIp = null; // 兜底IP

            while (interfaces.hasMoreElements()) {
                NetworkInterface ni = interfaces.nextElement();
                String name = ni.getName();          // 如 eth0, wlan0
                String displayName = ni.getDisplayName(); // 如 "以太网", "VMware Network Adapter VMnet1"

                // 跳过回环、未启用
                if (!ni.isUp() || ni.isLoopback()) continue;

                // 【关键】排除已知虚拟网卡，用名称/显示名双重过滤
                if (isVirtualAdapter(name, displayName)) continue;

                Enumeration<InetAddress> addrs = ni.getInetAddresses();
                while (addrs.hasMoreElements()) {
                    InetAddress addr = addrs.nextElement();
                    if (addr instanceof Inet4Address && !addr.isLoopbackAddress()) {
                        String ip = addr.getHostAddress();
                        // 优先级：物理网卡（以太网/Wi-Fi）立即返回
                        if (isPhysicalAdapter(name, displayName)) {
                            return ip;
                        }
                        // 否则暂存为兜底
                        if (fallbackIp == null) {
                            fallbackIp = ip;
                        }
                    }
                }
            }
            // 实在没找到物理网卡，返回第一个非虚拟网卡IP
            if (fallbackIp != null) return fallbackIp;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "127.0.0.1";
    }

    // 判断是否为物理网卡（名称特征）
    private static boolean isPhysicalAdapter(String name, String displayName) {
        String lowerName = name.toLowerCase();
        String lowerDisplay = displayName.toLowerCase();
        return lowerDisplay.contains("以太网") ||
                lowerDisplay.contains("wlan") ||
                lowerDisplay.contains("wi-fi") ||
                lowerName.startsWith("en") ||
                lowerName.startsWith("eth") ||
                lowerName.startsWith("wlan");
    }

    // 判断是否为虚拟网卡（需排除）
    private static boolean isVirtualAdapter(String name, String displayName) {
        String lowerName = name.toLowerCase();
        String lowerDisplay = displayName.toLowerCase();
        return lowerDisplay.contains("vmware") ||
                lowerDisplay.contains("virtualbox") ||
                lowerDisplay.contains("vbox") ||
                lowerDisplay.contains("docker") ||
                lowerDisplay.contains("vethernet") ||
                lowerName.startsWith("vmnet") ||
                lowerName.startsWith("vboxnet") ||
                lowerName.startsWith("docker");
    }
}