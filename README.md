# Custom Repath

> 把《Sky: 光·遇》的**截图**和**录像**目录，从游戏安装目录重定向到系统「图片」文件夹下的 `Sky` 子目录。

`custom-repath` 是一个独立 mod，依赖 [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) 模组加载器运行。

---

## 依赖

| 依赖 | 说明 |
|------|------|
| [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) | 模组加载器 / SDK。提供 `htmodloader` 头文件与 `libhtmodloader` 链接库，构建与运行都需要它。本 mod 的 `Makefile` 从 `../libraries/htmodloader` 引用其头文件与库。 |

---

## 功能

游戏默认把截图和录像写在**游戏安装目录**里：

```
<游戏目录>\data\ThatGameCompany\<应用ID>\images\
<游戏目录>\data\ThatGameCompany\<应用ID>\Record\
```

本 mod 会把这两类目录改写到 Windows 的「图片」文件夹下：

```
%Pictures%\Sky\<应用ID>\images\
%Pictures%\Sky\<应用ID>\Record\
```

`<应用ID>` 由游戏自己决定（例如国服为 `com.netease.sky`），mod 会原样保留，因此不同版本 / 不同渠道的游戏资源仍然分开存放。


---

## 目录结构

```
customcampath/
├── manifest.json
├── include/custompath/
└── src/
    ├── campath.cpp             # 钩子实现 + 路径解析
    ├── mod.cpp                 # mod 入口（HTModOnInit）
    ├── dllmain.c               # DLL 入口
    └── exports.txt
```

对外 API 只有三个函数：

```cpp
namespace campath {
  bool init() noexcept;                                  // 安装钩子
  void getCustomPath(char* out, std::size_t size) noexcept; // 取解析后的路径
  void shutdown() noexcept;                              // 卸载钩子
}
```

---

## 构建

前置条件：

- MinGW-w64（`gcc` / `g++`）与 `mingw32-make`；
- [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) 已就位于 `../libraries/htmodloader`（提供头文件与 `libhtmodloader`）。

```bash
cd customcampath
mingw32-make
```

产物：

```
dist/custom-repath.dll
```

`make clean` 可清除中间产物。

---

## 安装

1. 构建得到 `dist/custom-repath.dll`；
2. 把 `custom-repath.dll` 连同 `manifest.json` 放进游戏的 mod 目录；
3. 启动游戏即可。

---

