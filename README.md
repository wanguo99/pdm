
# Peripheral Driver Module

## 简介

PDM (Peripheral Driver Module) 是一个用于提供通用外设访问框架的驱动模块。
它通过定义一套统一的接口和模型，使得外设的访问和管理变得更加简单和标准化。

PDM 主要由以下几个部分组成：
- **PDM Core**: 驱动的核心入口，负责 PDM 总线的注册、PDM 主设备的初始化、子模块的初始化以及外设驱动的注册。
- **PDM Master**: 一个通用的外设控制器类型，提供了一个统一的外设访问模型。
- **PDM Template Master**: 一个具体的外设控制器实例，提供了一种名为 template 的外设访问接口。
- **PDM Device**: 一个通用的外设抽象模型，提供了统一的外设设备模型。
- **PDM Template I2C Driver**: 一个使用 I2C 访问的 template 外设的实例。

## 目录结构

```
.
├── include
│   ├── pdm.h
│   ├── pdm_osa.h
│   ├── pdm_submodule.h
│   └── pdm_template.h
├── Kbuild
├── LICENSE
├── Makefile
├── README.md
├── src
│   ├── core
│   │   ├── pdm_core.c
│   │   ├── pdm_device.c
│   │   ├── pdm_master.c
│   │   └── pdm_submodule.c
│   └── template
│       ├── drivers
│       │   └── pdm_template_i2c_driver.c
│       └── pdm_template_master.c
└── test
    └── test.c
```

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

2. **编译模块**：

   ```sh
   make
   ```

   这将编译 PDM 模块，并生成 `pdm.ko` 文件。

## 安装

### 安装步骤

1. **安装模块**：

   ```sh
   sudo make install
   ```

   这将把编译好的模块和相关文件安装到指定的目录中。

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
   sudo make uninstall
   ```

   这将删除安装目录中的所有文件。

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

- GitHub: [https://github.com/wanguo99/PDM](https://github.com/wanguo99/pdm)

## 版本历史

- v1.0.0: 初始版本


