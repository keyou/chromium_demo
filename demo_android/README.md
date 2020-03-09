# demo_apk

`demo_apk` 可以构建出一名为`DemoApk.apk`的Android应用，它和普通的Android应用最大的不同就是构建方式。一般的Android开发者使用AndroidStudio等IDE来构建Android应用，在chromium中不使用这些IDE而使用gn。

chromium提供了一套gn配置用来打包各种Android组件，包括 apk,aar,jar 等。主要的脚本位于`//build/config/android/`目录下。可以重点查看以下两个文件：

```
//build/config/android/config.gni
//build/config/android/rules.gni
```

## demo_apk 构成

demo_apk 由以下几部分组成：

1. `AndroidManifest.xml`：Android应用的描述文件，包括权限定义，icon等；
2. `res`: Android应用的资源文件，包括UI布局，图片，字符串等；
3. `src`: 源代码文件；

## JNI

gn提供了从java代码生成JNI/C++头文件的脚本，具体信息参考`base/android/jni_generator/README.md`。