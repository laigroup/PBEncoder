# PB Encoder

用于把 Pseudo-Boolean Constraint 转化为 CNF 范式

## 编译执行

使用 `./INSTALL.sh` 脚本调用 cmake 进行编译，生成 `Encoder` 可执行文件

使用 `./Encoder input_file output_file` 来执行编码


## 格式

输入格式为 PB约束，根据 PB16 的输入规则进行，给出示例如下：

```
* #variable= 5 #constraint= 3
****************************************
*
+1 x1 +1 x2 +1 x3 = 2 ;
-1 x3 -1 x4 -1 x5 >= -2 ;
-1 x3 -2 x5 -1 x2 >= -2 ;
```

其中包括了以 * 开头的注释行，和以PB约束

输出格式为 CNF 范式 
