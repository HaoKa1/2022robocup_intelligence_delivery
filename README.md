# 前言
22年赛道第一次设立，通宵达旦干了两个月，最后获得了冠军，三年过去了，忽然找到留存的资料，想起有好几个赛事群内的群友问我要过资料。虽赛事要求已发生了变化，但基本功能和逻辑一致，于是想直接开源出来，希望能帮助到参赛的同学，也希望赛事能得到更好的发展！  
时间久远，有些文件可能会有错误，欢迎指正，如果内容对你有用的话，请给个小星星⭐️！
# 介绍
PCB硬件设计部分由[@HaoKa1](https://github.com/HaoKa1)完成  
STM32控制代码部分由[@HaoKa1](https://github.com/HaoKa1)和Kaicheng Jin共同完成  
建模由Donghao Wang/Dewang Chen共同完成  
<img src="./images/assembly.jpg" alt="Assembly Image" width="400">
## 建模
结构使用亚克力板、铝合金板材和铝型材搭建，参考models目录下的文件
## 硬件
主控芯片选型为STM32F103RCT6，PCB元器件选型参考hardware目录下的工程文件，外设元器件参考如下，其中因主控芯片定时器数量限制，超声波测距实际上并未被使用                 
### 📡 传感模块
| 传感器类型 | 型号 | 功能 |
|:----------|:-----|:-----|
| 激光测距 | ATK-MS53L0M | 距离判断 |
| 超声波测距 | HC-SR04 | 测量距离 |
| 灰度传感器 | 得科技术 | 循迹检测 |
| 颜色识别 | TCS3472XFN | 高尔夫球颜色识别 |

### 🚀 动力模块
| 部件类型 | 型号 | 功能 |
|:--------|:-----|:-----|
| 电机驱动 | 得科技术 | 控制电机 |
| 电机 | 任意 | 驱动麦轮 |
| 麦轮 | 任意 | / |
| 舵机 | DS3220/DS3230 | 拨盘、转盘、开门 |

### 🎛️ 其他模块
| 模块类型 | 型号 | 功能 |
|:--------|:-----|:-----|
| 扫码模块 | MG65 | 识别二维码 |
| 语音播报模块 | SYN 6288 | 播报信息 |

## 软件
单片机移植FreeRTOS做状态调度，简化调度逻辑，具体流程如下
```mermaid
graph TD
    %% 初始化部分
    Servo_Rest["Servo Rest()<br/>优先级: 2"] -->|挂起相关任务| Start["Start()<br/>优先级: 2"]
    Start -->|判断出发区| GoLine["GoLine()<br/>优先级: 1"]
    
    %% 主巡线循环
    GoLine -->|巡线中| TOF["TOF()<br/>优先级: 2"]
    GoLine -->|转弯信号| Corner["Corner()<br/>优先级: 2"]
    GoLine -->|接收测距通知| Stop["Stop()<br/>优先级: 1"]
    GoLine -->|超声波信号| Wave["Wave()<br/>优先级: 1"]
    
    %% 分选系统（优先级1）
    subgraph "分选系统 - 优先级1"
        Drop["Drop()<br/>1. 接受Stop信号<br/>2. 接受Judge/Select信号<br/>3. 转动<br/>4. 信号给Recognition"]
        Recognition["Recognition()<br/>1. 接受Drop信号<br/>2. 颜色存入队列HC<br/>3. 信号给Judge"]
        Judge["Judge()<br/>1. 接受Recognition信号<br/>2. 颜色判断<br/>3. 无球→JudgetoDrop<br/>4. 有球→颜色存入Index<br/>5. 信号给Select"]
        Select["Select()<br/>1. 接受Judge信号<br/>2. 从Index取颜色<br/>3. 转到对应位置<br/>4. 信号给Drop"]
        
        Drop -->|信号量D| Recognition
        Recognition -->|信号量Recognition| Judge
        Judge -->|无球: JudgetoDrop| Drop
        Judge -->|有球: 存入队列Index| Select
        Select -->|SelecttoDrop| Drop
    end
    
    %% 避障系统
    Wave -->|距离<250| Avoid["Avoid()<br/>优先级: 2"]
    Wave -->|>=250| GoLine
    Avoid -->|结束条件| GoLine
    
    %% 停车处理流程
    TOF -->|测距数据| Stop
    Stop -->|第一次停车| GoLineLow["挂起Goline<br/>拉起GolineLow"]
    Stop -->|通知Drop| Drop_Start["分选开始"]
    Stop -->|第2-7次停车| Movein["Movein()<br/>优先级: 1"]
    Stop -->|第8次停车| SlowCross["减速过跷跷板"]
    
    %% 货站操作流程
    Movein -->|右侧平移| QR["QR()<br/>优先级: 2"]
    QR -->|数据=0| Speak_0["Speak()<br/>播报0<br/>优先级: 3"]
    QR -->|数据≠0| Put["Put()<br/>优先级: 1"]
    
    QR -->|QRtoMoveout| Moveout["Moveout()<br/>优先级: 1"]
    Speak_0 -->|播报完成| Moveout
    Put -->|PuttoSpeak| Speak_1["Speak()<br/>播报颜色<br/>优先级: 3"]
    Put -->|PuttoMoveout| Moveout
    Speak_1 -->|播报完成| Moveout
    
    %% 继续前进
    Moveout -->|MoveouttoForward| Forward["Forward()<br/>优先级: 1"]
    Stop -->|20秒后| Forward_Continue["拉起Goline/相关任务"]
    Forward -->|拉起TOF/Goline/Stop/QR| GoLine
    
    %% 特殊流程
    Stop -->|第7次后| EnableFrontTOF["开启前方测距"]
    Corner -->|第4次转弯| ReturnStart["返回出发区"]
    
    %% 结束条件
    Judge -->|10个球分选完| DeleteModules["删除Drop/Recognition/Judge/Select"]
    ReturnStart --> End["流程结束"]

    %% 样式定义
    classDef priority1 fill:#e1f5fe,stroke:#0288d1,stroke-width:2px
    classDef priority2 fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef priority3 fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px
    classDef special fill:#fff3e0,stroke:#ef6c00,stroke-width:2px
    
    class Drop,Recognition,Judge,Select,GoLine,Stop,Movein,Put,Moveout,Forward,Wave priority1
    class Servo_Rest,Start,Corner,TOF,QR,Avoid priority2
    class Speak_0,Speak_1 priority3
    class DeleteModules,ReturnStart,EnableFrontTOF,SlowCross special
```