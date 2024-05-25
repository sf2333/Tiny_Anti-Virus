# Tiny_Anti-Virus 微杀毒软件

软件可分为四个模块：病毒扫描模块，通过特征码查杀方法对磁盘目录中的文件进行静态查杀；主动防御模块，通过内核层的驱动程序监控文件读写、注册表项修改、API调用和进程启动，并进行内核和用户层之间的通信，以告警用户是否存在异常行为；病毒清除模块，通过驱动层对被占用的恶意文件进行解锁和强制删除；自我保护模块，允许用户选择隐藏系统进程或阻止其他程序终止系统自身进程的运行。

![系统架构图](https://my-tc-1308282641.cos.ap-guangzhou.myqcloud.com/markdown/%E7%B3%BB%E7%BB%9F%E6%9E%B6%E6%9E%84%E5%9B%BE.png)

## 软件使用

### 使用环境：Win10虚拟机

因为软件涉及内核驱动的安装，若在物理机中运行可能导致蓝屏等一系列BUG。

### 配置步骤

0、关闭虚拟机中所有的杀毒软件（可能会禁止本软件某些功能的运行）

1、将Realse目录完整的复制到虚拟机中

2、将INF目录下的所有.inf文件依次进行安装

![image-20240525223255474](https://my-tc-1308282641.cos.ap-guangzhou.myqcloud.com/markdown/image-20240525223255474.png)

3、将hook_dll目录下的两个文件夹（\x86、\x64）分别加入系统环境变量中

![image-20240525223325491](https://my-tc-1308282641.cos.ap-guangzhou.myqcloud.com/markdown/image-20240525223325491.png)

4、运行Tiny_anti-vruis.exe

## 代码文件Code

软件主要采用C++语言进行内核驱动程序(.sys)以及动态链接库(.dll)的编写。模块功能的实现上，通过调用YARA扫描API实现病毒扫描模块的功能。通过在内核驱动里注册内核回调函数、获取进程删除权限、读取并修改内核进程信息等操作实现主动防御、病毒清除、自我保护三个模块的功能。

### 各文件夹对应功能

- UI——软件前端

- YaraApiTest——YARA扫描引擎

- FileScanner——磁盘文件监控

- KernelCallBacks——内核回调注册（API调用监控、注册表监控）

- ProcessBlock——进程启动监控

- MyHookDll——HOOK R3级API的DLL

- ForceDelete——病毒清除

- SelfProctect——自我保护

- sysinstaller——驱动安装

- AdUserDLL——各模块用户层与内核层通信

- Virus_API——用于测试API调用监控功能