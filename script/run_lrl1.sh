#!/bin/bash
server=1
worker=4
tool=./local.sh

configPath=../config/rcv1/filter/
logPath=../log/

name=l1lr_baseline
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log

name=l1lr_kkt
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log

name=l1lr_round_13bit
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log

name=l1lr_round_5bit
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log

name=l1lr_round_1bit
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log

name=l1lr_round_adapt
rm -f $logPath/$name.log
echo "$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log"
$tool $server $worker $configPath/$name.conf 2> $logPath/$name.log
