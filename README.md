# Peripheral Driver Module (PDM)

## 简介

PDM (Peripheral Driver Module) 是一个用于提供通用外设访问框架的驱动模块。它通过定义一套统一的接口和模型，简化了外设的访问和管理流程，使其更加标准化。

PDM 主要由以下几个部分组成：
- **PDM Core**: 驱动的核心入口，负责 PDM 总线的注册、PDM 主设备的初始化、子模块的初始化以及外设驱动的注册。
- **PDM Master**: 一个通用的外设控制器类型，提供了一个统一的外设访问模型。
- **PDM Template Master**: 一个具体的外设控制器实例，提供了一种名为 template 的外设访问接口。
- **PDM Device**: 一个通用的外设抽象模型，提供了统一的外设设备模型。
- **PDM Template I2C Driver**: 一个使用 I2C 访问的 template 外设的实例。


## 编译

### 前提条件

确保你已经安装了以下工具和库：
- `make`
- `gcc`
- `kernel headers` 和 `kernel build system`

### 编译步骤

1. **克隆仓库**：

   ```sh
   git clone https://github.com/yourusername/pdm.git
   cd pdm
   ```

2. **本地编译**：

   - 编译:

     ```sh
     make
     ```

   - 清理:

     ```sh
     make clean
     ```

   这将编译 PDM 模块，并生成 `pdm.ko` 文件。

3. **交叉编译**（针对嵌入式系统）：

   - 设置交叉编译环境变量：

     ```sh
     export ARCH=arm
     export CROSS_COMPILE=arm-linux-gnueabi-
     export KERNELDIR="/path/to/your/kernel/build/directory"
     ```

   - 执行交叉编译命令：

     ```sh
     make
     ```

4. **编译选项**：

   - **日志管理**：
     该驱动支持日志打印信息的配置：

     - **日志总开关**：
       要关闭所有日志打印功能，可以在编译时使用以下命令：

       ```sh
       make osa_log_enable=0
       ```

     - **开启文件名、行号和函数名信息打印**：
       要开启日志打印并包含文件名、行号和函数名信息，可以在编译时使用以下命令：

       ```sh
       make osa_log_enable=1 osa_log_with_function=1 osa_log_with_file_line=1
       ```

## 安装

### 安装步骤

1. **安装模块**：

   默认情况下，模块将安装到当前目录下的 `_install` 文件夹中。你可以通过设置 `DESTDIR` 环境变量来指定其他安装目录。

   ```sh
   sudo make install DESTDIR="/path/to/install/directory"
   ```

   如果不指定 `DESTDIR`，则默认安装到当前目录下的 `_install` 文件夹中。

2. **加载模块**：

   ```sh
   sudo insmod pdm.ko
   ```

3. **验证模块是否加载成功**：

   ```sh
   lsmod | grep pdm
   ```

   如果看到 `pdm` 模块在列表中，说明模块加载成功。

## 卸载

### 卸载步骤

1. **卸载模块**：

   ```sh
   sudo rmmod pdm
   ```

2. **清理安装目录**：

   ```sh
   sudo make uninstall DESTDIR="/path/to/install/directory"
   ```

   如果不指定 `DESTDIR`，则默认清理当前目录下的 `_install` 文件夹。

## 调试

### 调试文件系统

PDM 模块支持通过 `debugfs` 进行调试。在模块加载后，可以在 `/sys/kernel/debug/pdm` 目录下找到相关的调试文件。

### 示例

待完善

## 测试

### 测试代码

在 `test` 目录下有一个测试文件 `test.c`，可以用来验证 PDM 模块的功能。

### 编译测试代码

1. **进入测试目录**：

   ```sh
   cd test
   ```

2. **编译测试代码**：

   ```sh
   gcc -o test test.c
   ```

### 运行测试

1. **运行测试程序**：

   ```sh
   ./test
   ```

2. **查看测试结果**：

   测试程序会输出一些调试信息，帮助你验证 PDM 模块的功能。

## 许可证

PDM 模块遵循 GPL 许可证。更多信息请参见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎贡献代码和提出改进建议！

## 联系方式

如果有任何问题或建议，请联系：

- 作者：wanguo
- 邮箱：guohaoprc@163.com

感谢你的使用和支持！

## 项目地址

- GitHub: [https://github.com/wanguo99/PDM](https://github.com/wanguo99/PDM)

## 版本历史

- v1.0.0: 初始版本
