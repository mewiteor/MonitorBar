Monitor Bar
==============

This application is a desk band which monitors the cpu usage, the memory usage
and the cpu temperature in the windows.

The MonitorBar project is the main project.
The ManageDriverAndDeskBand project is the manager of the MonitorBar.

The Monitor Bar has three status bars.The first one is cpu usage bar.The second one is
memory usage bar.And the last one is the cpu temperature bar.the cpu temperature
support intel cpu and amd cpu.

When move the mouse to the monitor bar,it will show a tooltip which show the
detailed information.The detailed information includes:
* Max cpu usage
* Min cpu usage
* Top 3 running processes sorted by cpu usage
* Max memory usage
* Min memory usage
* Top 3 running processes sorted by memory usage
* Max cpu temperature
* Min cpu temperature
* Each cpu core temperature

Right click monitor bar,you can reset the monitor bar or close the monitor bar.

The ManageDriverAndDeskBand is used to install/uninstall the driver and register/
unregister the desk band.

Monitor Bar
==============

这是Windows平台下的一个桌面状态栏工具，用于显示CPU使用率、内存使用率和CPU温度。

此解决方案包含MonitorBar和ManageDriverAndDeskBand两个项目。

Monitor Bar有三个状态条：CPU使用率、内存使用率和CPU温度。CPU温度支持Intel和AMD的CPU。

将鼠标移到Monitor Bar会显示详细信息，包含：
* CPU最大使用率
* CPU最小使用率
* CPU使用率最大的三个进程
* 内存最大使用率
* 内存最小使用率
* 内存使用率最大的三个进程
* CPU最高温度
* CPU最低温度
* CPU每个核心的温度

右键点击Monitor Bar可以重置和关闭Monitor Bar。

ManageDriverAndDeskBand用于安装/卸载驱动和注册/反注册desk band。
