## fuse-bot 使用方法

本fuse仅包含两层结构

## 运行与挂载

进入src文件夹，输入如下命令

```
make
/bot /mnt
cd /mnt
```

## 命令

```
mkdir $username 创建用户，仅在挂载点下生效

ls $path 查看目录文件，若路径为挂载点，则显示所有用户；若路径为用户，则显示其收到的信息

cd $path 打开目录文件，效果同ls

cat $receive/$send 打开$receive用户收到的来自$send用户的上一条信息

echo "your msg" > $send/$receive $receive用户向$send用户发送信息
```

