## Need to be fixed
1. 在wifi连接到网络时，检测会非常慢，可能是在命令中用调用iwconfig时使用多管道，要改。
2. 对于多无线网卡情况还没加处理。
3. 可能存在多无线网卡时，一张用于接入interface，一张用于AP共享，这种情况要放在后续版本中补充。
4. 连入设备的检测还未编写
5. 需要添加gksu或使用Policykit来让某些操作获得root权限
