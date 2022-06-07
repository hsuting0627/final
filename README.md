# final
My code download:  
https://drive.google.com/file/d/1U1aQzE8mom6O4qhhBQPVqsas9JMpjEMr/view  
壹、	簡介
本次期末專題是設計一台能夠跟隨提前繪製好之黑線地圖之自走車，加上一整學期以來學習的各項電子零件，組合馬達零件Servo、測距離零件 Ping、無線控制零件XBee、輪速偵測光圈零件Optical Encoder、偵測黑線零件QTI Sensor。  
貳、	零件  
甲、	伺服馬達  
本次期末專題我使用的是Parallax High Speed Continuous Rotation Servo，可以經過調變脈衝寬度來調整旋轉角度，擁有比一般伺服馬達三倍的轉速。  
-	Calibration 校正：1500之脈衝寬度，整體馬達應處以停止狀態，若有轉動的情況發生，需轉動馬達後方的校準螺絲，如發生抖動的情況，有可能導致內部齒輪磨損。  
乙、	超音波距離感測器  
本次期末專題我使用的是 Parallax PING Ultrasonic Sensor，藉由發出超聲波，到目標的距離通過計算波遇到目標反射後再次接收的時間。  
丙、	無線控制零件  
本次期末專題我使用的是 Digi International XBee-S2C Module，藉由建立UART溝通管道，使用兩個XBee，一個連接電腦，一個裝置在自走車上，以此即時回傳自走車速度及行走距離。  
丁、	輪速偵測光圈零件  
本次期末專題我使用的是 Parallax Boe Bot Digital Encoder Kit，裝置在輪圈後方，偵測每次亮暗輪圈上的洞間距，來測量總共旋轉幾圈。  
戊、	黑線偵測裝置  
本次期末專題我使用的是 Parallax QTI Sensor，裝置上有紅外線發射器及接收器，藉由電路設計搭配電容來區別黑線與白地，黑線會吸掉發出的能量，白地則會完全返回，就能夠區別電容的放電時間。  


參、	零件安裝配置  
Servo Motor Left	D10  
Servo Motor Right	D11  
Optical Encoder	D3  
PING Ultrasonic module	D9  
XBee	D1, D0, 9600  
QTI Sensor	D4, D5, D6, D7  

步驟：  
python : led_test_client.py  
py led_test_client.py COM3  
使用eRPC開始自走車，開始後即會回傳資料(速度、已行走距離)  
