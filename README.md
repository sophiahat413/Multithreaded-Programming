### 我三個小題,分成main1.c main2.c main3.c

## a. Discription
### 改寫檔案：
### Single thread on different CPU 
* main1.c
1. 由於create thread function(pthread_create)其第四個參數需為function的變數,且需為指標形式,因此我將raytracing的8個input argument包成一個structure,並轉換為指標,再傳入raytracing function中
2. 先利用sysconf(_SC_NPROCESSORS_CONF)查看我的實體機有幾個cpu,得出共有4個

3. 利用pthread_create function create出一個thread,並跑ray tracing function
4. 建一迴圈,利用pthread_setaffinity_np讓thread再不同cpu上執行,另外跑原始檔案約為9.5秒,因此我讓其再每個cpu上執行約2秒sleep(2),再轉換至另一個cpu上執行
* CPU 執行圖
![](https://i.imgur.com/h6D984r.jpg)


### Two threads on different CPU 
* main2.c
1. 在raytracing的input structure中多加兩個參數,thread之id及共有多少個thread
2. 並建一迴圈,create出兩個thread,並使其run在兩個不同的cpu上,利用thread id % 4決定run在那顆cpu上
* raytracing.c
1. 由於圖片之長寬皆為512,為2的指數倍數,因此我假設我所欲產生之thread數目皆會為2的倍數,因此於raytracing function中,我利用將高分割成不同等份的方式,配給不同的thread做運算
2. 再raytracing function中,我將height除上總共的thread數,得知每個thread須畫多少面積的圖,並依據不同的thread id決定所要畫之圖片位置
* CPU執行圖
![](https://i.imgur.com/uhaFlNH.jpg)


### Two threads on one CPU 
* main3.c
所做更動與multithread_programming2大致一樣,唯每個thread所指定之執行cpu是一樣的
* CPU 執行圖
![](https://i.imgur.com/OPDL4uP.jpg)

## b. Analysis
### 在同一個cpu上執行不同數量的thread,如下圖

![](https://i.imgur.com/Z36VmzD.png)
### 分析: 
* 從一個thread變為兩個thread時,執行時間有減少,然而變為4個thread時,其執行時間卻上升：由於程式在單一CPU上跑,一個thread切換到另一個執thread會耗掉某些效率, 於是當執行緒太多的時候,反而會使效率變差

### 執行一個以上的thread,且每個thrad在不同cpu上執行,如下圖
![](https://i.imgur.com/ZCIkqeD.png)
### 分析:
* thread越多,執行時間越短：開一個以上的thread,且thread在不同的cpu上執行,時間確實減少許多,因為除了thread可分工計算量,且在不同的cpu上同時執行,效能提升許多 

### 一個thread,並在不同數量的cpu中切換,如下圖
![](https://i.imgur.com/DdbfszH.png)
### 分析:
* 單一thread,CPU數量增加,執行時間沒有一定的變快或變慢：單一thread在不同數量的cpu間轉換執行,似乎不會隨著轉換的cpu數量增加,執行時間會隨著減少,分析可能是因為轉換cpu會耗效率,且每顆cpu執行的效率不同,轉換至不同的cpu會影響執行效率

## C. Others
### Problems：
1. 在把raytracing 的參數arguments包成struct時,值傳入function會錯： 發現慧根struct宣告參數的前後順序有關,但幕前還不知為會這樣
2. 其他程式的執行會干擾CPU跑的結果：由於原本的實體機有再跑server,以至於CPU跑的結果不如預期,關掉server後,變正常許多
3. Makefile格式問題：某些指令後接的變數須換行,否則無法執行
### What I learned:
1. 如何create thread, set thread affinity
2. 遇到多參數function時,要先把參數包成一個structure,然後轉成指標傳入
3. Makefile之編寫方式
4. 不同cpu數量,不同thread數量與執行情形的關係
5. 複習C (XD)
