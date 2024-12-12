# Peripheral Driver Module (PDM)

## Introduction

The Peripheral Driver Module (PDM) is a driver module designed to provide a generic framework for peripheral access. It simplifies the process of accessing and managing peripherals by defining a unified set of interfaces and models, making it more standardized.

PDM mainly consists of the following components:
- **PDM Core**: The core entry point of the driver, responsible for registering the PDM bus, initializing the PDM master device, initializing sub-modules, and registering peripheral drivers.
- **PDM Adapter**: A generic peripheral adapter type that provides a unified peripheral access model.
- **PDM Device**: A generic peripheral abstraction model that offers a unified peripheral device model.

## Compilation

### Prerequisites

Ensure you have the following tools and libraries installed:
- `make`
- `gcc`
- `kernel headers` and `kernel build system`

### Compilation Steps

1. **Clone the Repository**

   ```sh
   git clone https://github.com/yourusername/pdm.git
   cd pdm
   ```

2. **Local Compilation**

   - Compile:

     ```sh
     make
     ```

   - Clean:

     ```sh
     make clean
     ```

   This will compile the PDM module and generate the `pdm.ko` file.

3. **Cross-Compilation** (for embedded systems)

   - Set up cross-compilation environment variables:

     ```sh
     export ARCH=arm
     export CROSS_COMPILE=arm-linux-gnueabi-
     export KERNELDIR="/path/to/your/kernel/build/directory"
     ```

   - Execute the cross-compilation command:

     ```sh
     make
     ```

4. **Compilation Options**

   - **Logging Management**
     The driver supports configuration of log printing options

     - **Global Log Switch**：
       To disable all log printing, use the following command during compilation:

       ```sh
       make osa_log_enable=0
       ```

     - **Enable File Name, Line Number, and Function Name Logging**：
       To enable logging with file name, line number, and function name information, use the following command during compilation:

       ```sh
       make osa_log_enable=1 osa_log_with_function=1 osa_log_with_file_line=1
       ```

## Installation

### Installation Steps

1. **Install the Module**：

   By default, the module will be installed in the `_install` directory under the current directory. You can specify another installation directory by setting the `DESTDIR` environment variable.

   ```sh
   sudo make install DESTDIR="/path/to/install/directory"
   ```

   If `DESTDIR` is not specified, it defaults to the `_install` directory under the current directory.

2. **Load the Module**：

   ```sh
   sudo insmod pdm.ko
   ```

3. **Verify Successful Module Loading**：

   ```sh
   lsmod | grep pdm
   ```

   If the `pdm` module appears in the list, the module has been loaded successfully.

## Uninstallation

### Uninstallation Steps

1. **Unload the Module**：

   ```sh
   sudo rmmod pdm
   ```

2. **Clean Up the Installation Directory**：

   ```sh
   sudo make uninstall DESTDIR="/path/to/install/directory"
   ```

   If `DESTDIR` is not specified, it defaults to cleaning up the `_install` directory under the current directory.

## Debugging

### Debug File System

	The PDM module supports debugging via `debugfs` and `procfs`. After loading the module, you can find related debug files in the `/sys/kernel/debug/pdm` and `/proc/pdm` directory.

### Example

	To be completed.


## License

	The PDM module is licensed under the GPL license. For more information, see the LICENSE file.

## Contributions

	Contributions and suggestions are welcome!

## Contact

	If you have any questions or suggestions, please contact:
	- Author: wanguo
	- Email: guohaoprc@163.com

	Thank you for using and supporting this project!

## Project Address

- GitHub: [https://github.com/wanguo99/PDM](https://github.com/wanguo99/PDM)